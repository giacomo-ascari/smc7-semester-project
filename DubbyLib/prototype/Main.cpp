#include "daisy_seed.h"
#include "daisysp.h"
#include "DspBlock.h"

// Use the daisy namespace to prevent having to type
// daisy:: before all libdaisy functions
using namespace daisy;
using namespace daisysp;

// Declare a DaisySeed object called hardware
DaisySeed  hardware;
// Declare the DspBlocks we want to use later
DspBlock* oscillator;
DspBlock* freqLfo;
DspBlock* ampLfo;
DspBlock* freqOsc;
DspBlock* lfo;


void AudioCallback(AudioHandle::InterleavingInputBuffer  in,
                   AudioHandle::InterleavingOutputBuffer out,
                   size_t                                size)
{
    // Calling the handle method of each dsp block
    //  Note: As oscillator depends on the output of lfo, lfo needs to be called first
    //  Note_2: ConstValue->handle(); does not need to be called - well, because the value stays constant ^^
    lfo->handle();
    oscillator->handle();
    
    // Set the "final" pointer to the "last" dspBlock, meaning the DspBlock that is directly connected to the output, visually speaking
    float * oscOut = oscillator->getOutputChannel(0);
    int k = 0;
    for(size_t i = 0; i < size; i += 2)
    {
        // Copy values into Daisy's actual output buffer (two channels)
        out[i] = oscOut[k];
        out[i+1] = oscOut[k];
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

    // Initialize two "ConstBlocks" as input for the lfo osc
    freqLfo = new ConstValue(10, 4);
    freqLfo->initialize(samplerate);
    ampLfo = new ConstValue(1, 4);
    ampLfo->initialize(samplerate);
    // Initialize a vector to store the referencens to the outputs of the two const blocks
    std::vector<float*> lfoIn;
    // Push the pointer to the vector
    lfoIn.push_back(freqLfo->getOutputChannel(0));
    lfoIn.push_back(ampLfo->getOutputChannel(0));
    // Create an instance of OscBlock with the aforementioned vector as input
    lfo = new Osc(lfoIn, 4);
    lfo->initialize(samplerate);
    
    // See above
    freqOsc = new ConstValue(800, 4);
    freqOsc->initialize(samplerate);

    std::vector<float*> oscIn;
    oscIn.push_back(freqOsc->getOutputChannel(0));
    // The principle is the same as above. But instead of a constant buffer we here point to a location, that will contain the changing values generated through the lfo
    oscIn.push_back(lfo->getOutputChannel(0));

    // Initialize the oscillator with frequency 800hz and amplitude modulated by the lfo
    oscillator = new Osc(oscIn, 4);
    oscillator->initialize(samplerate);

    //Start calling the audio callback
    hardware.StartAudio(AudioCallback);
    // Loop forever
    for(;;) {}
}
