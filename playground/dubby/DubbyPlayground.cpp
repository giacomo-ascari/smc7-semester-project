
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

DspBlock * f;
DspBlock * sine;



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
    
    f->handle();
    sine->handle();
  

    float * oscOut = sine->getOutputChannel(0);
	for (size_t i = 0; i < size; i++)
	{
        for (int j = 0; j < 4; j++) 
        {
            float sample = out[j][i];
            sumSquared[j] += sample * sample;
            out[j][i] = oscOut[i];
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


   

    f = new ConstValue(220, AUDIO_BLOCK_SIZE);
    f->initialize(48000);

    sine = new Osc(AUDIO_BLOCK_SIZE);
    sine->initialize(48000);
    sine->setInputReference(f->getOutputChannel(0), 0);
    

    dubby.DrawLogo(); 
    System::Delay(2000);
	dubby.seed.StartAudio(AudioCallback);
    dubby.UpdateMenu(0, false);

	while(1) { 
        dubby.ProcessAllControls();
        dubby.UpdateDisplay();
	}
}
