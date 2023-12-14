#include "DspNode.h"

void Clock::initialize(float samplerate)
{
        this->samplerate = samplerate;
}

void Clock::process()
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

Osc::Osc(int bufferLenth) : Osc::DspNode(1, 1, bufferLenth){};

// Initializes the DaisySp::Oscillator with some default values
void Osc::initialize(float samplerate)
{
        osc.Init(samplerate);
        osc.SetWaveform(osc.WAVE_SIN);
        osc.SetAmp(1.f);
        osc.SetFreq(800);
}

void Osc::process()
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

void ADSREnv::process()
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

void FeedbackDelay::process()
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

void KnobMap::process()
{
        float val = dubby.GetKnobValue(knob);
        for (int i = 0; i < bufferLength; i++)
        {
                out->writeSample(val, i, 0);
        }
}

void DubbyKnobs::process()
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
void NMultiplier::process()
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
void Sum::process()
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
void Sub::process()
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

void Div::process()
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

void Scaler::process()
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

void Unipolariser::process()
{
        float *in = getInputReference(0);

        for (int i = 0; i < bufferLength; i++)
        {
                out->writeSample(abs(in[i]), i, 0);
        }
}

//--------Volume Controller----------//

void VolumeControl::process()
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

void Mix::process()
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

NoiseGen::NoiseGen(int bufferLenth) : NoiseGen::DspNode(1, 1, bufferLenth){};

void NoiseGen::process()
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

//--------BPM related time signature to samples converter------//
// the user insert BPM , Note value , and dotted (if it is preferred)
// then the node converts the musical time into time in samples depending on BPM
// Half Note = 2, Quarter Note = 1, Eigth Note = 0.5, Sixteenth Note = 0.25;
// Dotted Off = 0; Dotted On = 1;
#include <cmath>

void MusicalTime::process()
{

        int fs = 48000;
        float *bpm = getInputReference(0);
        float *notevalue = getInputReference(1);
        float *dotted = getInputReference(2);
        float delayInsamples;
        for (int sample = 0; sample < bufferLength; sample++)
        {

                if (dotted[sample] == 1)
                {
                        dotted[sample] = 0.5 * notevalue[sample];
                }
                else if (dotted[sample] == 0)
                {
                        dotted[sample] = 0;
                }

                if (bpm[sample] > 0)
                {
                        bpm[sample] = bpm[sample];
                }

                delayInsamples = round((60 / bpm[sample]) * fs * (notevalue[sample] + dotted[sample]));

                out->writeSample(delayInsamples, sample, 0);
        }
}

void StoF::process()
{
        float *tsamples = getInputReference(0); // Time in samples (input)
        float tHz;                              // time in HZ (output)
        int fs = 48000;

        for (int sample = 0; sample < bufferLength; sample++)
        {
                tHz = (fs / tsamples[sample]) / 2;
                out->writeSample(tHz, sample, 0);
        }
}

//----fliter----

// Band Pass Filter

void BPF::process()
{
        float *in = getInputReference(0);
        float *Fc = getInputReference(1);
        float *Q = getInputReference(2);

        float k;
        float norm;
        float b_0, b_1, b_2, a_1, a_2;
        float fltOut;

        for (int sample = 0; sample < bufferLength; sample++)
        {
                cirBuffin[sample % 4] = in[sample];
                cirBuffout[sample % 4] = fltOut;

                k = tanf(M_PI * Fc[sample] / 48000);
                norm = 1 / (1 + k / Q[sample] + k * k);
                b_0 = k / Q[sample] * norm;
                b_1 = 0;
                b_2 = -b_0;

                a_1 = 2 * (k * k - 1) * norm;
                a_2 = (1 - k / Q[sample] + k * k) * norm;

                float n_1 = cirBuffin[(4 + sample - 1) % 4];
                float n_2 = cirBuffin[(4 + sample - 2) % 4];
                float yn_1 = cirBuffout[(4 + sample - 1) % 4];
                float yn_2 = cirBuffout[(4 + sample - 2) % 4];

                fltOut = in[sample] * b_0 + n_1 * b_1 + n_2 * b_2 - yn_1 * a_1 - yn_2 * a_2;

                out->writeSample(fltOut, sample, 0);
        }
}

//-----LPF----

void LPF::process()
{
        float *in = getInputReference(0);
        float *Fc = getInputReference(1);
        float *Q = getInputReference(2);

        float k;
        float norm;
        float b_0, b_1, b_2, a_1, a_2;
        float fltOut;

        for (int sample = 0; sample < bufferLength; sample++)
        {
                cirBuffin[sample % 4] = in[sample];
                cirBuffout[sample % 4] = fltOut;

                k = tanf(M_PI * Fc[sample] / 48000);
                norm = 1 / (1 + k / Q[sample] + k * k);
                b_0 = k * k * norm;
                b_1 = 2 * b_0;
                b_2 = b_0;

                a_1 = 2 * (k * k - 1) * norm;
                a_2 = (1 - k / Q[sample] + k * k) * norm;

                float n_1 = cirBuffin[(4 + sample - 1) % 4];
                float n_2 = cirBuffin[(4 + sample - 2) % 4];
                float yn_1 = cirBuffout[(4 + sample - 1) % 4];
                float yn_2 = cirBuffout[(4 + sample - 2) % 4];

                fltOut = in[sample] * b_0 + n_1 * b_1 + n_2 * b_2 - yn_1 * a_1 - yn_2 * a_2;

                out->writeSample(fltOut, sample, 0);
        }
}

//----High Pass Filter HPF-----

void HPF::process()
{
        float *in = getInputReference(0);
        float *Fc = getInputReference(1);
        float *Q = getInputReference(2);

        float k;
        float norm;
        float b_0, b_1, b_2, a_1, a_2;
        float fltOut;

        for (int sample = 0; sample < bufferLength; sample++)
        {
                cirBuffin[sample % 4] = in[sample];
                cirBuffout[sample % 4] = fltOut;

                k = tanf(M_PI * Fc[sample] / 48000);
                norm = 1 / (1 + k / Q[sample] + k * k);
                b_0 = 1 * norm;
                b_1 = -2 * b_0;
                b_2 = b_0;

                a_1 = 2 * (k * k - 1) * norm;
                a_2 = (1 - k / Q[sample] + k * k) * norm;

                float n_1 = cirBuffin[(4 + sample - 1) % 4];
                float n_2 = cirBuffin[(4 + sample - 2) % 4];
                float yn_1 = cirBuffout[(4 + sample - 1) % 4];
                float yn_2 = cirBuffout[(4 + sample - 2) % 4];

                fltOut = in[sample] * b_0 + n_1 * b_1 + n_2 * b_2 - yn_1 * a_1 - yn_2 * a_2;

                out->writeSample(fltOut, sample, 0);
        }
}