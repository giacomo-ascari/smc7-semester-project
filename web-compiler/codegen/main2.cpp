#include "daisysp.h"
#include "../../web-compiler/build_template/lib/DaisyDub/Dubby.h"
#include "../../web-compiler/build_template/lib/DaisyDub/DspBlock.h"

using namespace daisy;
using namespace daisysp;

Dubby dubby;


MultiChannelBuffer * physical_ins;
DspBlock * knob1;
DspBlock * knob2;
DspBlock * knob3;
DspBlock * knob4;

DspBlock * block_9b133f86b7cb36f0;
DspBlock * block_0f7e668700029dd5;
DspBlock * block_3015bc1d1e622637;
DspBlock * block_8f9945f757ff2517;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(int i = 0; i < 4; i++)
    {
        physical_ins->writeChannel(in[i], i);
    }
    double sumSquared[4] = { 0.0f };
    
    knob1->handle();
    knob2->handle();
    knob3->handle();
    knob4->handle();
    
    block_9b133f86b7cb36f0->handle();
block_0f7e668700029dd5->handle();
block_3015bc1d1e622637->handle();
block_8f9945f757ff2517->handle();
    // float * oscOut = sine->getOutputChannel(0);
    // physical out routing
	for (size_t i = 0; i < size; i++)
	{
        for (int j = 0; j < 4; j++) 
        {
            float sample = out[j][i];
            sumSquared[j] += sample * sample;
            // out[j][i] = oscOut[i];
        } 
        dubby.scope_buffer[i] = (out[0][i] + out[1][i])  * .1f;   
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

    physical_ins = new MultiChannelBuffer(4, AUDIO_BLOCK_SIZE);

    knob1 = new KnobMap(dubby, 0, AUDIO_BLOCK_SIZE);
    knob2 = new KnobMap(dubby, 1, AUDIO_BLOCK_SIZE);
    knob3 = new KnobMap(dubby, 2, AUDIO_BLOCK_SIZE);
    knob4 = new KnobMap(dubby, 3, AUDIO_BLOCK_SIZE);

    block_9b133f86b7cb36f0 = new oscillator(AUDIO_BLOCK_SIZE);
block_0f7e668700029dd5 = new const(1, AUDIO_BLOCK_SIZE);
block_3015bc1d1e622637 = new const(10, AUDIO_BLOCK_SIZE);
block_8f9945f757ff2517 = new adder(AUDIO_BLOCK_SIZE);
   
    block_9b133f86b7cb36f0->initialize(samplerate);
block_0f7e668700029dd5->initialize(samplerate);
block_3015bc1d1e622637->initialize(samplerate);
block_8f9945f757ff2517->initialize(samplerate);

    block_8f9945f757ff2517->setInputReference(block_0f7e668700029dd5->getOutputChannel(0),0);
block_8f9945f757ff2517->setInputReference(block_3015bc1d1e622637->getOutputChannel(val0ue),1);

    dubby.DrawLogo(); 
    System::Delay(2000);
	dubby.seed.StartAudio(AudioCallback);
    dubby.UpdateMenu(0, false);

	while(1) { 
        dubby.ProcessAllControls();
        dubby.UpdateDisplay();
	}
}
