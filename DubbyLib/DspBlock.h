#include <string>

class DspBlock {
public:
    static string NAME;
    static int numInputs;
    static int numOutputs;
    
    virtual void handle(float * in, float * out);
}

class Delay : DspBlock {
public
    Delay();

    void handle(float * in, float * out);

private:
    int * delayBuffer;
}