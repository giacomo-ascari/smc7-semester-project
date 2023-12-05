import json
import sys

""" 
Converts a string x to a DspBlock declaration. The corresponding output is
DspBlock * x;
"""
def genBlockDeclaration(varName: str) -> str:
    return f"DspBlock * {varName};"

""" 
Creates an instantiation statement in the form of varName = new childClass(constrParamList..., AUDIO_BLOCK_SIZE);

varName - pointer the instance should be assigned to
childClass - the concrete implementation of DspBlock used
constrParamList - optional and additional constructor parameters

"""
def getInstantiation(varName: str, childClass: str, constrParamList) -> str:
    constrParamList.append('AUDIO_BLOCK_SIZE')
    return f"{varName} = new {childClass}({', '.join(constrParamList)});"

""" 
Optionally creates function-call to the initialize function of a given variable extending from DspBlock in the form of:
varName->initialize(samplerate);

varName - the pointer to a DspBlock instance
needsInit - returns the statement if true, an empty string otherwise

"""
def genInit(varName: str, needsInit: bool) -> str:
    return f"{varName}->initialize(samplerate);" if needsInit else ""

""" 
Returns a list of one function invocation per input. Each statement has the form of:
varName->setInputReference(sourceChannel, internalChannel);

varName - a pointer to a DspBlock isntance
inputs - a map of input definitions
        the key defines the internal Input channel
        the value must contain a fromId to reference the source dspBlock and a fromChannel field to
        reference the output channel of the source dspBlock.

"""
def genRouting(varName: str, inputs):
    methodCalls = []
    for inCh in inputs:
        outCh = f"{inputs[inCh]['fromId']}->getOutputChannel({inputs[inCh]['fromChannel']})"
        t = f"{varName}->setInputReference({outCh},{inCh});"
        methodCalls.append(t)
    return methodCalls

""" 
Returns a single method invocation of handle() for a given dspBlock instance.
Form: varName->handle();

"""
def genHandleCall(varName: str) -> str:
    return f"{varName}->handle();"

""" 
Return a list of handle() method invocations for all DspBlocks.
The invocations are ordered so that DspBlocks without any input channels are the first elements, 
subsequent are invocations of DspBlock that only rely on the first elements, then Blocks relying only on the first or the second elements, and so on.

The provided blocks may not contain cycles. If it does, this method will never halt!
"""
def genOrderedHandleCalls(blocks):
    handledBlocks = [x for x in blocks if not x['inputs']]
    for block in handledBlocks:
        blocks.remove(block)
    while len(blocks) > 0:
        handledIds = [x['id'] for x in handledBlocks]
        errorState = True
        for block in blocks:
            ins = block['inputs']
            unhandledIns = []
            for inCh in ins:
                if ins[inCh]['fromId'] not in handledIds:
                    unhandledIns.append(ins[inCh])
            if len(unhandledIns) == 0:
                errorState = False
                handledBlocks.append(block)
                blocks.remove(block)
        if errorState:
            sys.exit('Cycle in input routing')

    return [genHandleCall(x['id']) for x in handledBlocks]


file = open('test.json')

blocks = json.load(file)['blocks']

blockDeclarations = [genBlockDeclaration(x['id']) for x in blocks]
blockInstanciation = [getInstantiation(x['id'], x['type'], x['constructorParams']) for x in blocks]
blockInitializations =[genInit(x['id'], True) for x in blocks]
blockRoutings = [genRouting(x['id'], x['inputs']) for x in blocks]
flatRoutings = [item for sublist in blockRoutings for item in sublist]
orderedHandleCalls = genOrderedHandleCalls(blocks)

with open("main.cpp.template", 'r') as templatefile:
    template = templatefile.read()
with open(f"main2.cpp", 'w+') as writefile:
    template = template.replace('%declarations%', '\n'.join(blockDeclarations))
    template = template.replace('%handle_invocations%', '\n'.join(orderedHandleCalls))
    template = template.replace('%instanciation%', '\n'.join(blockInstanciation))
    template = template.replace('%initialization%', '\n'.join(blockInitializations))
    template = template.replace('%routing%', '\n'.join(flatRoutings))
    writefile.write(template)

