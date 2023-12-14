
#include "daisysp.h"
#include "../../web-compiler/build_template/lib/DaisyDub/Dubby.h"
#include "../../web-compiler/build_template/lib/DaisyDub/DspBlock.h"



using namespace daisy;
using namespace daisysp;
using namespace dspblock;

Dubby dubby;


DubbyAudioIns * dubbyAudioIn;
MultiChannelBuffer * dubbyAudioOuts;
DspBlock * knob1;
DspBlock * knob2;
DspBlock * knob3;
DspBlock * knob4;

DspBlock * BPM;
DspBlock * NoteVal;
DspBlock * dotSwitch;
DspBlock * noise;
DspBlock * rhythm;
DspBlock * lfofreq;
DspBlock * lfo;


DspBlock * f;
DspBlock * sine;
DspBlock * f_lfo;
DspBlock * lfo;
DspBlock * amp;
DspBlock * compressor;



void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(int i = 0; i < 4; i++)
    {
        dubbyAudioIn->writeChannel(in[i], i);
    }
    double sumSquared[4] = { 0.0f };
    
    knob1->handle();
    knob2->handle();
    knob3->handle();
    knob4->handle();
   
   BPM->handle();
   dotSwitch->handle();
   NoteVal->handle();
   rhythm->handle();
   lfofreq->handle();
   lfo->handle();
   noise->handle();
    
    f->handle();
    sine->handle();
    f_lfo->handle();
    lfo->handle();
    amp->handle();
    compressor->handle();
  

    dubbyAudioOuts->writeChannel({0}, 0);
    dubbyAudioOuts->writeChannel(noise->getOutputChannel(0), 1);

    float * oscOut = demoCompressor ->getOutputChannel(0);
	
    for (size_t i = 0; i < size; i++)
	{
        for (int j = 0; j < 4; j++) 
        {
            float sample = out[j][i];
            sumSquared[j] += sample * sample;
            out[j][i] = dubbyAudioOuts->getChannel(0)[i] * 0.25;
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

    dubbyAudioIn = new DubbyAudioIns(AUDIO_BLOCK_SIZE);
    dubbyAudioOuts = new MultiChannelBuffer(4, AUDIO_BLOCK_SIZE);

    // knob1 = new KnobMap(dubby, 0, AUDIO_BLOCK_SIZE);
    // knob2 = new KnobMap(dubby, 1, AUDIO_BLOCK_SIZE);
    // knob3 = new KnobMap(dubby, 2, AUDIO_BLOCK_SIZE);
    // knob4 = new KnobMap(dubby, 3, AUDIO_BLOCK_SIZE);

    
   
    BPM = new ConstValue(120, AUDIO_BLOCK_SIZE);
    BPM->initialize(48000);

    dotSwitch = new ConstValue(1,AUDIO_BLOCK_SIZE);
    dotSwitch->initialize(48000);

    NoteVal = new ConstValue(0.5,AUDIO_BLOCK_SIZE);
    NoteVal->initialize(48000);

    rhythm = new MusicalTime(AUDIO_BLOCK_SIZE);
    rhythm->setInputReference(BPM->getOutputChannel(0),0);
    rhythm->setInputReference(NoteVal->getOutputChannel(0),1);
    rhythm->setInputReference(dotSwitch->getOutputChannel(0),2);
 
    lfofreq = new StoF(AUDIO_BLOCK_SIZE);
    lfofreq->setInputReference(rhythm->getOutputChannel(0),0);

    
    lfo = new Osc(AUDIO_BLOCK_SIZE);
    lfo->initialize(48000);
    lfo->setInputReference(lfofreq->getOutputChannel(0),0);

    noise = new NoiseGen(AUDIO_BLOCK_SIZE); 
    noise->setInputReference(lfo->getOutputChannel(0),0);

    f = new ConstValue(500, AUDIO_BLOCK_SIZE);
    f->initialize(48000);

    sine = new Osc(AUDIO_BLOCK_SIZE);
    sine->initialize(48000);
    sine->setInputReference(f->getOutputChannel(0), 0);
    
    f_lfo = new ConstValue(1, AUDIO_BLOCK_SIZE);
    f_lfo->initialize(48000);

    lfo = new Osc(AUDIO_BLOCK_SIZE);
    lfo->initialize(48000);
    lfo->setInputReference(f_lfo->getOutputChannel(0), 0);

    amp = new NMultiplier(2, AUDIO_BLOCK_SIZE);
    amp->initialize(48000);
    amp->setInputReference(sine->getOutputChannel(0), 0);
    amp->setInputReference(lfo->getOutputChannel(0), 1);


    compressor = new dspblock::Compressor(AUDIO_BLOCK_SIZE);
    compressor->initialize(48000);
    compressor->setInputReference(amp->getOutputChannel(0), 0);
    
    
    dubby.DrawLogo(); 
    System::Delay(2000);
	dubby.seed.StartAudio(AudioCallback);
    dubby.UpdateMenu(0, false);

	while(1) { 
        dubby.ProcessAllControls();
        dubby.UpdateDisplay();
	}
}

