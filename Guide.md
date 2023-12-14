# How to create new DspBlocks

## Extend DspBlock.h
The file is located in `web-compiler/buildtemplate/lib/DaisyDub/DspBlock.h`

1. Copy one of the existing blocks -> the structure stays roghly the same
2. Update class name, con- and destructor and number of in-/outputs.
3. Add documentation to your new block. Which channel does what? What kind of values are expected?

*Example*
```
class <NameDspBlock> : public DspBlock {
public:
    <NameDspBlock>(<possible addition parameters,> int bufferLength) : DspBlock(<numberOfInputs>, <numberOfOutputs>, bufferLength) {
       // Add Custom code here if needed.
       // This could be for example the place to initialize delay lines
    };
    ~<NameDspBlock>() = default;
    void initialize(float samplerate) override;
    void process() override { };
private:
    // Add private variables here, if needed
    // e.g., a buffer for a delay
};
```

## Extend DspBlock.cpp
The file is located in `web-compiler/buildtemplate/lib/DaisyDub/DspBlock.cpp`

**Implement `initialize(float samplerate)`, if required**

>Useful for stuff, that needs to be done once on the startup of the DaisyDub. A good example is `void ConstValue::initialize(float samplerate)` which just fills up the output buffer with the chosen value.

## Implement `process()`, if required
Here, all the actual processing takes place.

### Accessing input channels data
- You can get a buffer of input samples of the **nth** channel with
`float * freqIn = getInputReference(n);`
    - there are no real safe-guards against selecting a channel that does not exist. Please make sure you have set the right amount of channels in `DspBlock.h` earlier and that the first channel is channel 0 instead of 1;
- all input buffers are expected to have the same size as the general `AUDIO_BLOCK_SIZE`
- all input samples are a `float`. But there is yet not strict definition of the values going in there.
    -  e.g., an oscillator expects values in Hz for frequency but the "physical"output expects values between -1 and 1.
    - the consequence is: The end-user will have the responsibility of converting the values, using Multiplier, Addition, maybe Division and Substraction Blocks

> Good example might be `void Multiplier::process()`

### Writing output data

- You can write a single sample to the buffer with `out->writeSample(<yourSample>, samplePosition, channelNumber);`
    - again make sure that channelNumber is max. the number of outputs defined in the header file **- 1**
    - also samplesPosition should not be higher or equal to our buffer size

# Testing your newly created DspBlock
Of course, you want to test your changes! You can do that in the Playgrounds As the name suggest, go crazy here! ᕦ(òᴥó)ᕥ It's most fun with the Dubby but the DaisySeed also works. 

1. Open `DubbyPlayground.cpp` or `SeedPlayground.cpp` accordingly
2. Create a variable for you block below the `#include`s

*E.g.,*
`DspBlock * <someName>;`

3. Set your block up
    - `<someName> = new <YourBlock>(<eventual parameters, that you have specified,> AUDIO_BLOCK_SIZE);`
    - if the DspBlock implements the initialize method, you need to call it: `<someName> -> initialize(48000);`
4. Route inputs with outputs appropriateley
    - supposing you have setup more blocks:
        - `<someName> -> setInputReference(someOtherBlock -> getOutputChannel(n), k);`
        - this routes the *nth* output of `someOtherBlock` to the *kth* input channel of `someName`
        - make sure, that `someOtherBlock` has been properly initialized (meaning the constructor here) before
    - to route input from the physical input
        - `<someName>->setInputReference(physical_ins->getChannel(n), 0);` with n being the input channel number 0-3
5. Within `void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)` call the process method of your block by `<someName> -> process()`
    - Keep in mind to call all the `process()`-methods in the correct order
    - If a block you want to "process" requires input of another block, the other block needs to be called first!

> Special note: there should be only once instance of `KnobMap` per knob. If you want to route a knob to multiple Blocks, just do step 4. for every Block you want to feed with knob values

## Flash the Dubby:
- open a command line / or do it in Terminal from VSCode
- navigate to the folder of either the DubbySandbox or the SeedSandbox
    - use the console cmd `pwd` to show your current directory
    - use `cd someFolder` to navigate to a folder within your current directory
- run `make clean; make` to compile
    - make sure make is installed
- bring the Dubby/Seed into dfu mode
- flash by running `make program-dfu`
