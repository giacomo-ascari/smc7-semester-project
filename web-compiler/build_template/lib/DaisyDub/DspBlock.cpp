#include "DspBlock.h"

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