#include <string>
#include "daisy_seed.h"
#include "daisysp.h"
#include "Dubby.h"

using namespace daisy;
using namespace daisysp;

/**
 * Stores a variable amount of channels sequentially in a single buffer in the format of
 * { A_1, A_2, B_1, B_2, ..., N_1, N_2} and provides access to individual channels.
*/
class MultiChannelBuffer {
public:
    MultiChannelBuffer(int numChannels, int bufferSizePerChannel) 
    {
        this->numChannels = numChannels;
        this->samplesPerChannel = bufferSizePerChannel;
        // Initialize the buffer with appropriate size: channels * samplesPerChannel
        buffer = new float[numChannels * bufferSizePerChannel];
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
    void writeChannel(float * data, int channelNumber)
    {
        // Check if the provided channelNumber is with in the range of existing channels
        if (channelNumber < 0 || channelNumber >= numChannels) {
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

class Osc : public DspBlock {
public:
    Osc(int bufferLenth);
    ~Osc() = default;
    void initialize(float samplerate) override;
    void handle() override;
    
private:
    Oscillator osc;
};

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

class Multiplier : public DspBlock {
    public:
    Multiplier(int bufferLength) : DspBlock(2, 1, bufferLength) {};
    ~Multiplier() = default;
    void initialize(float samplerate) override {};
    void handle() override;
};

class Unipolariser : public DspBlock {
    public:
    Unipolariser(int bufferLength) : DspBlock(1, 1, bufferLength) {};
    ~Unipolariser() = default;
    void initialize(float samplerate) override {};
    void handle() override;
};