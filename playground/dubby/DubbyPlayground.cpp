
#include "daisysp.h"
#include "../../web-compiler/build_template/lib/DaisyDub/Dubby.h"
#include "../../web-compiler/build_template/lib/DaisyDub/DspBlock.h"

using namespace daisy;
using namespace daisysp;

Dubby dubby;
DspBlock * osc1;
DspBlock * lfo;
DspBlock * f_osc1;
DspBlock * a_osc1;
DspBlock * f_lfo;
DspBlock * freqMulti;
DspBlock * unipolariser;
DspBlock * ampMulti;
DspBlock * knobMulti;
DspBlock * knob1;
DspBlock * knob2;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    float * ins = &in[0];
    double sumSquared[4] = { 0.0f };
    knob2->handle();
    knob1->handle();
    knobMulti->handle();
    lfo->handle();
    unipolariser->handle();
    freqMulti->handle();
    osc1->handle();
    ampMulti->handle();

    float * oscOut = ampMulti->getOutputChannel(0);
	for (size_t i = 0; i < size; i++)
	{
        for (int j = 0; j < 4; j++) 
        {
            float sample = out[j][i];
            sumSquared[j] += sample * sample;
            out[j][i] = oscOut[i] * 0.3;
        } 
        dubby.scope_buffer[i] = (out[0][i] + out[1][i])  * .5f;   
	}

    for (int j = 0; j < 4; j++) dubby.currentLevels[j] = sqrt(sumSquared[j] / AUDIO_BLOCK_SIZE);
}

int main(void)
{
	dubby.seed.Init();
    // dubby.InitAudio();
	// dubby.seed.StartLog(true);

    dubby.Init();
    
	dubby.seed.SetAudioBlockSize(AUDIO_BLOCK_SIZE); // number of samples handled per callback
	dubby.seed.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);
    dubby.ProcessAllControls();

    knob2 = new KnobMap(dubby, 1, AUDIO_BLOCK_SIZE);
    knob1 = new KnobMap(dubby, 0, AUDIO_BLOCK_SIZE);


    f_osc1 = new ConstValue(800, AUDIO_BLOCK_SIZE);
    f_osc1->initialize(48000);
    a_osc1 = new ConstValue(1, AUDIO_BLOCK_SIZE);
    a_osc1->initialize(48000);

    f_lfo = new ConstValue(100, AUDIO_BLOCK_SIZE);
    f_lfo->initialize(48000);

    knobMulti = new Multiplier(AUDIO_BLOCK_SIZE);
    knobMulti->setInputReference(f_lfo->getOutputChannel(0), 0);
    knobMulti->setInputReference(knob2->getOutputChannel(0), 1);

    lfo = new Osc(AUDIO_BLOCK_SIZE);
    lfo->initialize(48000);
    lfo->setInputReference(knobMulti->getOutputChannel(0), 0);

    unipolariser = new Unipolariser(AUDIO_BLOCK_SIZE);
    unipolariser->setInputReference(lfo->getOutputChannel(0), 0);

    freqMulti = new Multiplier(AUDIO_BLOCK_SIZE);
    freqMulti->setInputReference(unipolariser->getOutputChannel(0), 0);
    freqMulti->setInputReference(f_osc1->getOutputChannel(0), 1);

    osc1 = new Osc(AUDIO_BLOCK_SIZE);
    osc1->initialize(48000);
    osc1->setInputReference(freqMulti->getOutputChannel(0), 0);

    ampMulti = new Multiplier(AUDIO_BLOCK_SIZE);
    ampMulti->setInputReference(osc1->getOutputChannel(0), 0);
    ampMulti->setInputReference(knob1->getOutputChannel(0), 1);

    dubby.DrawLogo(); 
    System::Delay(2000);
	dubby.seed.StartAudio(AudioCallback);
    dubby.UpdateMenu(0, false);

	while(1) { 
        dubby.ProcessAllControls();
        dubby.UpdateDisplay();
	}
}
