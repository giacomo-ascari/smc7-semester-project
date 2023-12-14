#include <string>
#include "daisy_seed.h"
#include "daisysp.h"
#include "Dubby.h"

/* Included headers for compressor
compressor.h*/
#pragma once
#ifndef DSY_COMPRESSOR_H
#define DSY_COMPRESSOR_H

#include "Utility/dsp.h"

/*_________________________________*/



using namespace daisy;
using namespace daisysp;

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

class NoiseGen : public DspBlock {
public:
    NoiseGen(int bufferLenth) : DspBlock(1,1, bufferLength){};
    ~NoiseGen() = default;
    void initialize(float samplerate) override;
    void handle() override;
    

};

/* --------Compressor---------------*/

class Compressor : public DspBlock
{
  public:
    Compressor() {}
    ~Compressor() {}
    /** Initializes compressor
        \param sample_rate rate at which samples will be produced by the audio engine.
    */
    void initialize(float samplerate) override;

    /** Compress the audio input signal, saves the calculated gain
        \param in audio input signal
    */
    float Process(float in);

    /** Compresses the audio input signal, keyed by a secondary input.
        \param in audio input signal (to be compressed)
        \param key audio input that will be used to side-chain the compressor
    */
    float Process(float in, float key)
    {
        Process(key);
        return Apply(in);
    }

    /** Apply compression to the audio signal, based on the previously calculated gain
        \param in audio input signal
    */
    float Apply(float in) { return gain_ * in; }

    /** Compresses a block of audio
        \param in audio input signal
        \param out audio output signal
        \param size the size of the block
    */
    void ProcessBlock(float *in, float *out, size_t size)
    {
        ProcessBlock(in, out, in, size);
    }

    /** Compresses a block of audio, keyed by a secondary input
        \param in audio input signal (to be compressed)
        \param out audio output signal
        \param key audio input that will be used to side-chain the compressor
        \param size the size of the block
    */
    void ProcessBlock(float *in, float *out, float *key, size_t size);

    /** Compresses a block of multiple channels of audio, keyed by a secondary input
        \param in audio input signals (to be compressed)
        \param out audio output signals
        \param key audio input that will be used to side-chain the compressor
        \param channels the number of audio channels
        \param size the size of the block
    */
    void ProcessBlock(float **in,
                      float **out,
                      float * key,
                      size_t  channels,
                      size_t  size);

    /** Gets the amount of gain reduction */
    float GetRatio() { return ratio_; }

    /** Sets the amount of gain reduction applied to compressed signals
     \param ratio Expects 1.0 -> 40. (untested with values < 1.0)
    */
    void SetRatio(float ratio)
    {
        ratio_ = ratio;
        RecalculateRatio();
    }

    /** Gets the threshold in dB */
    float GetThreshold() { return thresh_; }

    /** Sets the threshold in dB at which compression will be applied
     \param threshold Expects 0.0 -> -80.
    */
    void SetThreshold(float threshold)
    {
        thresh_ = threshold;
        RecalculateMakeup();
    }

    /** Gets the envelope time for onset of compression */
    float GetAttack() { return atk_; }

    /** Sets the envelope time for onset of compression for signals above the threshold.
        \param attack Expects 0.001 -> 10
    */
    void SetAttack(float attack)
    {
        atk_ = attack;
        RecalculateAttack();
    }

    /** Gets the envelope time for release of compression */
    float GetRelease() { return rel_; }

    /** Sets the envelope time for release of compression as input signal falls below threshold.
        \param release Expects 0.001 -> 10
    */
    void SetRelease(float release)
    {
        rel_ = release;
        RecalculateRelease();
    }

    /** Gets the additional gain to make up for the compression */
    float GetMakeup() { return makeup_gain_; }

    /** Manually sets the additional gain to make up for the compression
        \param gain Expects 0.0 -> 80
    */
    void SetMakeup(float gain) { makeup_gain_ = gain; }

    /** Enables or disables the automatic makeup gain. Disabling sets the makeup gain to 0.0
        \param enable true to enable, false to disable
    */
    void AutoMakeup(bool enable)
    {
        makeup_auto_ = enable;
        makeup_gain_ = 0.0f;
        RecalculateMakeup();
    }

    /** Gets the gain reduction in dB
    */
    float GetGain() { return fastlog10f(gain_) * 20.0f; }

  private:
    float ratio_, thresh_, atk_, rel_;
    float makeup_gain_;
    float gain_;

    // Recorded slope and gain, used in next sample
    float slope_rec_, gain_rec_;

    // Internals from faust
    float atk_slo2_, ratio_mul_, atk_slo_, rel_slo_;

    int   sample_rate_;
    float sample_rate_inv2_, sample_rate_inv_;

    // Auto makeup gain enable
    bool makeup_auto_;

    // Methods for recalculating internals
    void RecalculateRatio()
    {
        ratio_mul_ = ((1.0f - atk_slo2_) * ((1.0f / ratio_) - 1.0f));
    }

    void RecalculateAttack()
    {
        atk_slo_  = expf(-(sample_rate_inv_ / atk_));
        atk_slo2_ = expf(-(sample_rate_inv2_ / atk_));

        RecalculateRatio();
    }

    void RecalculateRelease() { rel_slo_ = expf((-(sample_rate_inv_ / rel_))); }

    void RecalculateMakeup()
    {
        if(makeup_auto_)
            makeup_gain_ = fabsf(thresh_ - thresh_ / ratio_) * 0.5f;
    }
};
