#include "daisy_seed.h"
#include "daisysp.h"
#include "../../web-compiler/build_template/lib/DaisyDub/DspNode.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisySeed object called hardware
DaisySeed  hardware;
// Declare the DspNodes we want to use later
DspNode* oscillator;
DspNode* freqLfo;
DspNode* ampLfo;

void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    // Calling the process method of each dsp node
    //  Note: As oscillator depends on the output of lfo, lfo needs to be called first
    //  Note_2: ConstValue->process(); does not need to be called - well, because the value stays constant ^^
    // lfo->process();
    oscillator->process();
    
    // Set the "final" pointer to the "last" dspNode, meaning the DspNode that is directly connected to the output, visually speaking
    // float * oscOut = oscillator->getOutputChannel(0);
    float * osc_out = oscillator->getOutputChannel(0);
    int k = 0;
    for(size_t i = 0; i < size; i += 2)
    {

        // Copy values into Daisy's actual output buffer (two channels)
        out[i] = osc_out[i];
        out[i+1] = osc_out[i];
        k++;
    }
}

 
int main(void)
{
    // Configure and Initialize the Daisy Seed
    // These are separate to allow reconfiguration of any of the internal
    // components before initialization.
    hardware.Configure();
    hardware.Init();
    hardware.SetAudioBlockSize(4);

    //How many samples we'll output per second
    float samplerate = hardware.AudioSampleRate();

    // Initialize two "ConstNodes" as input for the lfo osc
    freqLfo = new ConstValue(1000, 4);
    freqLfo->initialize(samplerate);
    ampLfo = new ConstValue(1, 4);
    ampLfo->initialize(samplerate);

    // // Create an instance of OscNode with the aforementioned vector as input
    // lfo = new Osc(4);
    // lfo->initialize(samplerate);
    // lfo->setInputReference(freqLfo->getOutputChannel(0), 0);
    // lfo->setInputReference(ampLfo->getOutputChannel(0), 1);

    // See above
    // freqOsc = new ConstValue(10000, 4);
    // freqOsc->initialize(samplerate);

    // Initialize the oscillator with frequency 800hz and amplitude modulated by the lfo
    oscillator = new Osc(4);
    oscillator->initialize(samplerate);
    oscillator->setInputReference(freqLfo->getOutputChannel(0), 0);
    oscillator->setInputReference(ampLfo->getOutputChannel(0), 1);

    //Start calling the audio callback
    hardware.StartAudio(AudioCallback);
    // Loop forever
    for(;;) {}
}
