#include "DspBlock.h"

using namespace dspblock;

void Clock::initialize(float samplerate)
{
        this->samplerate = samplerate;
}

void Clock::handle()
{
        float freqHz = getInputReference(0)[0];
        if (freqHz == 0)
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
                }
                else
                {
                        out->writeSample(0, i, 0);
                        samplesSinceTick++;
                }
        }

        samplesSinceTick = samplesSinceTick % periodSamples;
}

Osc::Osc(int bufferLenth) : Osc::DspBlock(1, 1, bufferLenth){};

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
        float *freqIn = getInputReference(0);
        // Set the frequency once per buffer (with small buffer lengths it should be sufficient for now)
        // Important note: Here we assume, that the value of freqIn[0] is provided in Hz. But that will not work generically. For example imagine we pluck the output of an osc into the freq input of this osc
        // in that case freqIn[0] would be some value between -1 and 1, which does not make sense in terms of Hz. We need to figure out how to make this better.
        osc.SetFreq(freqIn[0]);
        for (auto i = 0; i < bufferLength; i++)
        {
                float osc_out = osc.Process();
                // Apply the amp input
                // Set the left and right outputs
                out->writeSample(osc_out, i, 0);
        }
}

void ADSREnv::initialize(float samplerate)
{
        env.Init(samplerate);
}

void ADSREnv::handle()
{
        float *trigger = getInputReference(0);
        float attack = abs(getInputReference(1)[0]);
        float decay = abs(getInputReference(2)[0]);
        float sustain = abs(getInputReference(3)[0]);
        float release = abs(getInputReference(4)[0]);

        env.SetAttackTime(attack);
        env.SetDecayTime(decay);
        env.SetSustainLevel(sustain);
        env.SetReleaseTime(release);

        for (int i = 0; i < bufferLength; i++)
        {
                if (trigger[i] == 1)
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
        float *audioIn = getInputReference(0);
        float *ampDelay = getInputReference(1);

        for (int i = 0; i < bufferLength; i++)
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
        for (int i = 0; i < bufferLength; i++)
        {
                out->writeSample(val, i, 0);
        }
}

void DubbyKnobs::handle()
{
        for (int k = 0; k < 4; k++)
        {
                float val = dubby.GetKnobValue(knobs[k]);
                for (int i = 0; i < bufferLength; i++)
                {
                        out->writeSample(val, i, k);
                }
        }
}

void DubbyAudioIns::initialize(float samplerate)
{
        for (int k = 0; k < 4; k++)
        {
                for (int i = 0; i < bufferLength; i++)
                {
                        out->writeSample(0, i, k);
                }
        }
}

void DubbyAudioIns::writeChannel(const float * data, int channelNumber)
{
        out->writeChannel(data, channelNumber);
}

// Fills the ConstValue's buffer with a value
void ConstValue::initialize(float samplerate)
{
        for (int i = 0; i < bufferLength; i++)
        {
                out->writeSample(val, i, 0);
        }
}

// -----------------------------MATH OPERATIORS ------------------------------------//

//----Multiplier----//
// Multiplies n diferent channel input values and outputs the result
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

//-----Summation----//
// Add n diferent channel input values and outputs the result
void Sum::handle()
{
        for (int i = 0; i < bufferLength; i++)
        {
                float sum = 0;
                for (int k = 0; k < numInputs; k++)
                {
                        sum += getInputReference(k)[i];
                }
                out->writeSample(sum, i, 0);
        }
}
//---Subtraction----/
// Subtract n diferent channel input values and outputs the result
void Sub::handle()
{
        for (int i = 0; i < bufferLength; i++)
        {
                float sub = 0;
                for (int k = 0; k < numInputs; k++)
                {
                        sub -= getInputReference(k)[i];
                }
                out->writeSample(sub, i, 0);
        }
}

//---Division----//
// Divides n diferent channel input values and outputs the result

void Div::handle()
{
        for (int i = 0; i < bufferLength; i++)
        {
                float div = 0;
                for (int k = 0; k < numInputs; k++)
                {
                        div /= getInputReference(k)[i];
                }
                out->writeSample(div, i, 0);
        }
}

void Scaler::handle()
{
        float oldRange = inMax - inMin;
        float newRange = outMax - outMin;
        float *newValue;

        for (int sample = 0; sample < bufferLength; sample++)
        {

                newValue[sample] = ((getInputReference(0)[sample] - inMin) * newRange / oldRange) + outMin;

                out->writeSample(newValue[sample], sample, 0);
        }
}

//---------------------------- END OF MATH OPERATORS --------------------------//

void Unipolariser::handle()
{
        float *in = getInputReference(0);

        for (int i = 0; i < bufferLength; i++)
        {
                out->writeSample(abs(in[i]), i, 0);
        }
}

//--------Volume Controller----------//

void VolumeControl::handle()
{
        float *amp = getInputReference(0);
        float in = 0;

        for (int sample = 0; sample < bufferLength; sample++)
        {
                in = getInputReference(1)[sample];
                in *= amp[sample];

                out->writeSample(in, sample, 0);
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

                for (int ch = 0; ch < numOutputs; ch++)
                {
                        out->writeSample(in, sample, ch);
                }
        }
}

//------------ White noise generator-----------
#include <cstdlib>
#include <ctime>

NoiseGen::NoiseGen(int bufferLenth) : NoiseGen::DspBlock(1, 1, bufferLenth){};

void NoiseGen::handle()
{

        float *amp = getInputReference(0);
        float noiseOut = 0;

        for (auto sample = 0; sample < bufferLength; sample++)
        {
                float r = (static_cast<float>(std::rand() / static_cast<float>(RAND_MAX / 2))) - 1;

                noiseOut = r;
                noiseOut *= amp[sample];

                out->writeSample(noiseOut, sample, 0);
        }
    
}

/* --------Compressor---------------*/
void dspblock::Compressor::initialize(float samplerate) {
    compressor.Init(samplerate);
    compressor.SetThreshold(-10.0f);
    compressor.SetRatio(40.0f);
    compressor.SetAttack(0.02f);
    compressor.SetRelease(0.001f); 
}


void dspblock::Compressor::handle() {
    float *input = getInputReference(0);
    compressor.ProcessBlock(input, out->getChannel(0), bufferLength);
}


void KnobMap::handle() {
    float val = dubby.GetKnobValue(knob);
    switch (knob) {
        case Dubby::Ctrl::CTRL_1:
            compressor.SetAttack(val); // Assuming val ranges between 0 and 1
            break;
        case Dubby::Ctrl::CTRL_2:
            compressor.SetRelease(val);
            break;
        case Dubby::Ctrl::CTRL_3:
            compressor.SetRatio(val);
            break;
        case Dubby::Ctrl::CTRL_4:
            compressor.SetThreshold(val);
            break;
        // Add more cases for other controls if needed
    }
    // Write the knob value to output
    for (int i = 0; i < bufferLength; i++) {
        out->writeSample(val, i, 0);
    }
}
