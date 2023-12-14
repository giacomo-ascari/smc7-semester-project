#include <string>
#include "daisy_seed.h"
#include "daisysp.h"
#include "Dubby.h"

/*_________________________________*/



using namespace daisy;
using namespace daisysp;

namespace dspblock {
/**
 * Stores a variable amount of channels sequentially in a single buffer in the format of
 * { A_1, A_2, B_1, B_2, ..., N_1, N_2} and provides access to individual channels.
*/
class MultiChannelBuffer 
{
public:
    MultiChannelBuffer(int numChannels, int bufferSizePerChannel) 
    {
        this->numChannels = numChannels;
        this->samplesPerChannel = bufferSizePerChannel;
        // Initialize the buffer with appropriate size: channels * samplesPerChannel
        buffer = new float[numChannels * bufferSizePerChannel];
        for (int i = 0; i < numChannels * bufferSizePerChannel; i++) 
        {
            buffer[i] = 0;
        }
    };

    // Function to get a pointer to the first sample of a specified channel
    float * getChannel(int channelNumber) 
    {
        // Check if the channel number is valid
        if (channelNumber < 0 || channelNumber >= numChannels) {
            return nullptr;
        }
        // Calculate the starting index of the specified channel
        int startIndex = channelNumber * samplesPerChannel;
        // Return the pointer to the first sample of the specified channel
        //  Note: For this to work, it is assumed that the consumer of this buffer knows how many samples are in a single channel buffer. Otherwise weird stuff could happen
        return &buffer[startIndex];
    };

    // Write data to a channel by just specifying its channel number
    void writeChannel(const float * data, int channelNumber)
    {
        // Check if the provided channelNumber is with in the range of existing channels
        if (channelNumber < 0 || channelNumber >= numChannels || data == nullptr) {
            return;
        }

        // Calculate starting position for writing
        int startIndex = channelNumber * samplesPerChannel;
        // Copy the provided buffer into the multichannel buffer
        std::memcpy(&buffer[startIndex], data, samplesPerChannel * sizeof(float));
    };

    // Write a single sample to a specified channel at a specified index
    void writeSample(float sample, int index, int channelNumber)
    {
        // Check if the provided channelNumber is with in the range of existing channels
        if (channelNumber < 0 || channelNumber >= numChannels) {
            return;
        }
        // Check if provided index is within the buffer size
        if(index < 0 || index >= samplesPerChannel) {
            return;
        }
        // Get the correct write position
        int writePos = channelNumber * samplesPerChannel + index;
        buffer[writePos] = sample;
    }

private:
    float * buffer;
    int numChannels; // Number of channels
    int samplesPerChannel; // Number of samples per channel
};

// Generic superclass to unify different "DspBlocks" such as oscilator, delay, compressor, ...
class DspBlock {
public:
    // Constructor to initialize "every" DspBlock
    DspBlock(int numberIns, int numberOuts, int bufferLength)
    {
        this->bufferLength = bufferLength;
        // Initialize the output Multichannel buffer
        //  Note: How many DspBlocks will there be that have more than one output? Probably not many and the ones that are, we can probably neglect
        out = new MultiChannelBuffer(numberOuts, bufferLength);
        this->inputChannels = new float*[numberIns];
    };
    ~DspBlock() = default;
    // Override this function, to handle everything that needs to be only handled once at the beginning
    virtual void initialize(float samplerate) = 0;
    virtual void handle() = 0;
    float * getOutputChannel(int channelNumber) {
        return out->getChannel(channelNumber);
    }
    void setInputReference(float * inputRef, int channelNumber)
    {
        this->inputChannels[channelNumber] = inputRef;
    }
    float * getInputReference(int channelNumber)
    {
        return this->inputChannels[channelNumber];
    }

protected:
    MultiChannelBuffer* out;
    int bufferLength;
    float** inputChannels;
};

/**
 * Block to integrate physical knobs.
 * Assign the knob (0 - 3) using the constructor. Also please assign each knob only once.
 * 0 Inputs.
 * 1 Output:
 * - channel 0: knob value (0 - 1), its read only once per block, so all samples in this channel should be equal
*/
class KnobMap : public DspBlock {
public:
    KnobMap(Dubby& dubby, int knobNumber, int bufferLength) : DspBlock(0, 1, bufferLength), dubby (dubby)
    {
        this->knob = static_cast<Dubby::Ctrl>(knobNumber);
    };
    ~KnobMap() = default;
    void initialize(float samplerate) override {};
    void handle() override;
protected:
    Dubby::Ctrl knob;
    Dubby& dubby;
};

class DubbyKnobs : public DspBlock {
public:
    DubbyKnobs(Dubby& dubby, int bufferLength) : DspBlock(0, 4, bufferLength), dubby (dubby)
    {
        knobs = new Dubby::Ctrl[4];
        knobs[0] = static_cast<Dubby::Ctrl>(0);
        knobs[1] = static_cast<Dubby::Ctrl>(1);
        knobs[2] = static_cast<Dubby::Ctrl>(2);
        knobs[3] = static_cast<Dubby::Ctrl>(3);
    };
    void initialize(float samplerate) override {};
    void handle() override;
protected:
    Dubby::Ctrl * knobs;
    Dubby& dubby;
};

class DubbyAudioIns : public DspBlock {
public:
    DubbyAudioIns(int bufferLength) : DspBlock(0, 4, bufferLength) {};
    void initialize(float samplerate) override;
    void handle() override {};
    void writeChannel(const float * data, int channelNumber);

};

/**
 * Outputs an 1-sample impulse in the specified intervall.
 * Needs to be initialized
 * 1 Inputs:
 * - channel 0: tick frequency in Hz
 * 1 Output:
 * - channel 0: 0s, when no tick is produced, 1 for every tick
*/
class Clock : public DspBlock {
public:
    Clock(int bufferLength) : DspBlock(1, 1, bufferLength)
    {
        this->samplesSinceTick = 0;
    };
    ~Clock() = default;
    void initialize(float samplerate) override;
    void handle() override;
private:
    int samplesSinceTick;
    float samplerate;
};


/**
 * Simple oscillator using the underlying DaisySP::Oscillator
 * initalize() must be called beforehand!
 * 1 Input:
 * - channel 0: desired frequency, provide it in Hz
 * 1 Output:
 * - oscillator output (-1 - 1)
*/
class Osc : public DspBlock {
public:
    Osc(int bufferLenth);
    ~Osc() = default;
    void initialize(float samplerate) override;
    void handle() override;
    
private:
    Oscillator osc;
};

/**
 * ADSR Envelope using DaisySPs implementation.
 * Call initialize.
 * 4 Input:
 * - channel 0: trigger
 * - channel 1: attack in seconds
 * - channel 2: decay in seconds
 * - channel 3: sustain in seconds
 * - channel 4: release in seconds
 * 1 Output:
 * - envelope output (0 - 1)
*/
class ADSREnv : public DspBlock {
public:
    ADSREnv(int bufferLength) : DspBlock(4, 1, bufferLength) { };
    ~ADSREnv() = default;
    void initialize(float samplerate) override;
    void handle() override;
    
private:
    Adsr env;
};

/**
 * Simple feedback delay. 
 * Assign length in samples in the constructor.
 * 2 Inputs:
 * - channel 0: audio in
 * - channel 1: dry/wet mix with 0 being only dry and 1 being only wet signal
 * 1 Output:
 * - the (wet) signal
*/
class FeedbackDelay : public DspBlock {
public:
    FeedbackDelay(int lengthSamples, int bufferLength) : DspBlock(2, 1, bufferLength) {
        this->circBuf = new float[lengthSamples];
        this->circBufPos = 0;
        this->delayLengthSamples = lengthSamples;
    };
    ~FeedbackDelay() = default;
    void initialize(float samplerate) override;
    void handle() override;
private:
    float * circBuf;
    int circBufPos;
    int delayLengthSamples;
};

/**
 * Block to store a constant value.
 * Assing the constant value using the constructor.
 * Use it's initialize method!
 * The handle method, does not need to be called.
 * 0 Inputs
 * 1 Outpus:
 * - a buffer consisting of the same constant value, everytime.
*/
class ConstValue : public DspBlock {
public:
    // Constructor that automatically initializes DspBlock with a fixed inputVector and one channel
    ConstValue(float value, int bufferLength) : DspBlock(0, 1, bufferLength) {
        this->val = value;
    };
    ~ConstValue() = default;
    void initialize(float samplerate) override;
    void handle() override { };
private:
    float val;
};




// -----------------------------MATH OPERATIORS ------------------------------------//

//----Multiplier----//
//Multiplies tow diferent input values and outputs the result.

/**
 * A block to multiply the output of n channels sample-wise.
 * Assign the amount of input channels in the constructor
 * n inputs:
 * - [0..n] channels to multiply with each other
 * 1 output:
 * - the result of the multiplication
*/
class NMultiplier : public DspBlock {
public:
    NMultiplier(int numInputs, int bufferLength) : DspBlock(numInputs, 1, bufferLength)
    {
        this->numInputs = numInputs;
    };
    ~NMultiplier() = default;
    void initialize(float samplerate) override {};
    void handle() override;
private:
    int numInputs;
};


//-----Summation----//
//Add n diferent channel input values and outputs the result
class Sum : public DspBlock {
public:
    Sum(int numInputs, int bufferLength) : DspBlock(numInputs, 1, bufferLength)
    {
        this->numInputs = numInputs;
    };
    ~Sum() = default;
    void initialize(float samplerate) override {};
    void handle() override;
private:
    int numInputs;
};


//---Subtraction----/
//Subtract n diferent channel input values and outputs the result
class Sub : public DspBlock {
public:
    Sub(int numInputs, int bufferLength) : DspBlock(numInputs, 1, bufferLength)
    {
        this->numInputs = numInputs;
    };
    ~Sub() = default;
    void initialize(float samplerate) override {};
    void handle() override;
private:
    int numInputs;
};


//---Division----//
//Divides n diferent channel input values and outputs the result
class Div : public DspBlock {
public:
    Div(int numInputs, int bufferLength) : DspBlock(numInputs, 1, bufferLength)
    {
        this->numInputs = numInputs;
    };
    ~Div() = default;
    void initialize(float samplerate) override {};
    void handle() override;
private:
    int numInputs;
};


class Scaler : public DspBlock {
public:
Scaler(float inMin, float inMax, float outMin, float outMax, int bufferLength) : DspBlock(1, 1, bufferLength)
{
this->inMin = inMin;
this->inMax = inMax;
this->outMin = outMin;
this->outMax = outMax;
};
~Scaler() = default;
void initialize(float samplerate) override{};
void handle() override;

private:
float inMin, inMax, outMin, outMax;


};

//-----------------------------END OF MATH OPERATORS------------------------------//





/**
 * A block to make a signal unipolar by using the absolute value of the input.
 * 1 Input:
 * - the input signal
 * 1 output:
 * - the absolute / unipolar signal
*/
class Unipolariser : public DspBlock {
    public:
    Unipolariser(int bufferLength) : DspBlock(1, 1, bufferLength) {};
    ~Unipolariser() = default;
    void initialize(float samplerate) override {};
    void handle() override;
};

class VolumeControl : public DspBlock {
    public:
    VolumeControl(int bufferLength) : DspBlock(2 ,1 , bufferLength){};
   
    ~VolumeControl() = default;
    void initialize(float samplerate) override {};
    void handle() override;


};

class Mix : public DspBlock {
    public:
    Mix(int numInputs, int numOutputs, int bufferLength) : DspBlock(numInputs, numOutputs, bufferLength)
    {
        this->numInputs = numInputs;
        this->numOutputs = numOutputs;
    }
    ~Mix() = default;
    void initialize(float samplerate) override {};
    void handle() override;

    private:
    int numInputs;
    int numOutputs;

};

//  ------- white noise generator-------
// This 

class NoiseGen : public DspBlock {
public:
    NoiseGen(int bufferLenth);
    ~NoiseGen() = default;
    void initialize(float samplerate) override {};
    void handle() override;
    

};

//--------BPM related time signature to samples converter------//

// the user insert BPM , Note value , and dotted (if it is preferred)
// then the block converts the musical time into time in samples depending on BPM
// Half Note = 2, Quarter Note = 1, Eigth Note = 0.5, Sixteenth Note = 0.25;
// Dotted Off = 0; Dotted On = 1;
class MusicalTime : public DspBlock {
public:
    MusicalTime(int bufferlength) : DspBlock(3,1,bufferlength){};
    ~MusicalTime() = default;
    void initialize(float samplerate) override{};
    void handle()override;

}; 

//----- Time in samples to HZ converter--------//
// This block takes time in samples and outputs the frequency that is related to samples
//Example: if we want to modulate a block with a specific musical time (quarters) we have to convert
//the samples that MusicalTime block provides to HZ that oscilator can handle. So, that what this block does.

class StoF : public DspBlock {
public:
    StoF(int bufferlength) : DspBlock(1,1,bufferlength){};
    ~StoF() = default;
    void initialize(float samplerate) override{};
    void handle() override;

};

//------------Filters--------

//bandPass

class BPF : public DspBlock {
public:
    BPF(int bufferLength) : DspBlock(3, 1, bufferLength){};
    ~BPF() = default;
    void initialize(float samplerate) override{}; 
    void handle() override;    

private:
float cirBuffin[4];
float cirBuffout[4];
};

//------Low Pass Filter----

class LPF : public DspBlock {
public:
    LPF(int bufferLength) : DspBlock(3, 1, bufferLength){};
    ~LPF() = default;
    void initialize(float samplerate) override{}; 
    void handle() override;    

private:
float cirBuffin[4];
float cirBuffout[4];
};


//-------High Pass Filter HPF-------

class HPF : public DspBlock {
public:
    HPF(int bufferLength) : DspBlock(3, 1, bufferLength){};
    ~HPF() = default;
    void initialize(float samplerate) override{}; 
    void handle() override;    

private:
float cirBuffin[4];
float cirBuffout[4];
};

/* --------Compressor---------------*/

class Compressor : public DspBlock
{
  public:
    Compressor(int bufferLength) : DspBlock(2, 2, bufferLength)  {}
    ~Compressor() = default;
    
    void initialize(float samplerate) override;
    void handle() override;

    private:
    daisysp::Compressor compressor;

};