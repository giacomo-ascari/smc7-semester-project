#include "DspBlock.h"


void Clock::initialize(float samplerate)
{
        this->samplerate = samplerate;
}

void Clock::handle()
{
        float freqHz = getInputReference(0)[0];
        if (freqHz == 0 )
        {
                // Avoid 0-division later
                freqHz = 0.0000001;
        }
        // Note: In case of decimal periods, this will always round down
        int periodSamples = (1 / freqHz) * samplerate;

        for (int i = 0; i < bufferLength; i++)
        {
                if (samplesSinceTick == periodSamples - 1)
                {
                        out->writeSample(1.f, i, 0);
                        samplesSinceTick = 0;
                } else
                {
                        out->writeSample(0, i, 0);
                        samplesSinceTick++;
                }
        }

        samplesSinceTick = samplesSinceTick % periodSamples;
}

Osc::Osc(int bufferLenth) : Osc::DspBlock(1, 1, bufferLenth) {};

// Initializes the DaisySp::Oscillator with some default values
void Osc::initialize(float samplerate) 
{
        osc.Init(samplerate);
        osc.SetWaveform(osc.WAVE_SIN);
        osc.SetAmp(1.f);
        osc.SetFreq(800);
}

void Osc::handle() 
{
        // Read the input frequency buffer
        float * freqIn = getInputReference(0);
        // Set the frequency once per buffer (with small buffer lengths it should be sufficient for now)
        // Important note: Here we assume, that the value of freqIn[0] is provided in Hz. But that will not work generically. For example imagine we pluck the output of an osc into the freq input of this osc
        // in that case freqIn[0] would be some value between -1 and 1, which does not make sense in terms of Hz. We need to figure out how to make this better.
        osc.SetFreq(freqIn[0]);
        for(auto i = 0; i < bufferLength; i++)
        {
                float osc_out = osc.Process();
                // Apply the amp input
                //Set the left and right outputs
                out->writeSample(osc_out, i, 0);
        }
    
}

void ADSREnv::initialize(float samplerate)
{
        env.Init(samplerate);
}

void ADSREnv::handle()
{       
        float * trigger = getInputReference(0);
        float attack = abs(getInputReference(1)[0]);
        float decay = abs(getInputReference(2)[0]);
        float sustain = abs(getInputReference(3)[0]);
        float release = abs(getInputReference(4)[0]);

        env.SetAttackTime(attack);
        env.SetDecayTime(decay);
        env.SetSustainLevel(sustain);
        env.SetReleaseTime(release);

        for(int i = 0; i < bufferLength; i++)
        {
                if( trigger[i] == 1)
                {
                        env.Retrigger(false);
                }
                out->writeSample(env.Process(false), i, 0);
        }
}

void FeedbackDelay::initialize(float samplerate)
{
        for (int i = 0; i < delayLengthSamples; i++)
        {
                circBuf[i] = 0;
        }
}

void FeedbackDelay::handle()
{
        float * audioIn = getInputReference(0);
        float * ampDelay = getInputReference(1);

        for(int i = 0; i < bufferLength; i++)
        {       
                int readFrom = (circBufPos + i) % delayLengthSamples;
                float output = (1 - ampDelay[i]) * audioIn[i] + ampDelay[i] * circBuf[readFrom];
                out->writeSample(output, i, 0);
                circBuf[readFrom] = output;
        }
        circBufPos = (circBufPos + bufferLength) % delayLengthSamples;

}

void KnobMap::handle()
{
        float val = dubby.GetKnobValue(knob);
        for(int i = 0; i < bufferLength; i++) 
        {
                out->writeSample(val, i, 0);
        }
}

// Fills the ConstValue's buffer with a value
void ConstValue::initialize(float samplerate) 
{
        for(int i = 0; i < bufferLength; i++) 
        {
                out->writeSample(val, i, 0);
        }
}

void NMultiplier::handle()
{
        for (int i = 0; i < bufferLength; i++) 
        {
                float mul = 1;
                for (int k = 0; k < numInputs; k++)
                {
                        mul *= getInputReference(k)[i];
                }
                out->writeSample(mul, i, 0);
        }
}

void Unipolariser::handle()
{
        float * in = getInputReference(0); 

        for (int i = 0; i < bufferLength; i++) 
        {
                out->writeSample(abs(in[i]), i, 0);
        }
}

                //--------Volume Controller----------//

void VolumeControl::handle()
{
        float * amp = getInputReference(0);
        float in = 0;

        for (int sample = 0; sample < bufferLength; sample++)
        {
             in = getInputReference(1)[sample];
             in *= amp[sample];
             
             out->writeSample(in,sample,0);   
        }

}

                //--------Mixer----------//

void Mix::handle()
{
        float in = 0;
        for (int sample = 0; sample < bufferLength; sample++)
        {       

                for (int chIn = 0; chIn < numInputs; chIn++)
                {
                in += getInputReference(chIn)[sample];
                }

                in /= numInputs;
                
                for(int ch = 0; ch < numOutputs; ch++)
                {       
                        out->writeSample(in,sample,ch);
                }
        
        }
}

//------------ White noise generator-----------
#include <cstdlib>
#include <ctime>

NoiseGen::NoiseGen(int bufferLenth) : NoiseGen::DspBlock(1, 1, bufferLenth) {};

void NoiseGen::handle() 
{       
        // Seed the random number generator with the current time
    std::srand(static_cast<unsigned int>(std::time(0)));

        float * amp = getInputReference(0);
        float noiseOut=0;
        
        for(auto sample = 0; sample < bufferLength; sample++)
        {
                noiseOut = (2.0 * std::rand() / RAND_MAX) - 1.0;
                noiseOut *= amp[sample];

        
                out->writeSample(noiseOut, sample, 0);
        }
    
}

/* --------Compressor---------------*/
#include <cmath>
#include <stdlib.h>
#include <stdint.h>
#include "compressor.h"

using namespace daisysp;

#ifndef max
#define max(a, b) ((a < b) ? b : a)
#endif

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

void Compressor::initialize(float samplerate)
{
    samplerate_      = min(192000, max(1, samplerate));
    samplerate_inv_  = 1.0f / (float)samplerate_;
    samplerate_inv2_ = 2.0f / (float)samplerate_;

    // Initializing the params in this order to avoid dividing by zero

    Compressor.SetRatio(2.0f);
    Compressor.SetAttack(0.1f);
    Compressor.SetRelease(0.1f);
    Compressor.SetThreshold(-12.0f);
    Compressor.AutoMakeup(true);

    gain_rec_  = 0.1f;
    slope_rec_ = 0.1f;
}

float Compressor::Process(float in)
{
    float inAbs   = fabsf(in);
    float cur_slo = ((slope_rec_ > inAbs) ? rel_slo_ : atk_slo_);
    slope_rec_    = ((slope_rec_ * cur_slo) + ((1.0f - cur_slo) * inAbs));
    gain_rec_     = ((atk_slo2_ * gain_rec_)
                 + (ratio_mul_
                    * fmax(((20.f * fastlog10f(slope_rec_)) - thresh_), 0.f)));
    gain_         = pow10f(0.05f * (gain_rec_ + makeup_gain_));

    return gain_ * in;
}

void Compressor::ProcessBlock(float *in, float *out, float *key, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        Process(key[i]);
        out[i] = Apply(in[i]);
    }
}

// Multi-channel block processing
void Compressor::ProcessBlock(float **in,
                              float **out,
                              float * key,
                              size_t  channels,
                              size_t  size)
{
    for(size_t i = 0; i < size; i++)
    {
        Process(key[i]);
        for(size_t c = 0; c < channels; c++)
        {
            out[c][i] = Apply(in[c][i]);
        }
    }
}