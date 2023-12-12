import json
import sys
import os
import traceback

"""
Returns the variable with a prefix
"""
def getPrefixedVarname(varName: str) -> str:
    return f"block_{varName}"

""" 
Converts a string x to a DspBlock declaration. The corresponding output is
DspBlock * x;
"""
def genBlockDeclaration(varName: str) -> str:
    return f"DspBlock * {getPrefixedVarname(varName)};"

""" 
Creates an instantiation statement in the form of varName = new childClass(constrParamList..., AUDIO_BLOCK_SIZE);

varName - pointer the instance should be assigned to
childClass - the concrete implementation of DspBlock used
constrParamList - optional and additional constructor parameters

"""
def getInstantiation(varName: str, childClass: str, constrParamList) -> str:
    constrParamList.append('AUDIO_BLOCK_SIZE')
    return f"{getPrefixedVarname(varName)} = new {childClass}({', '.join(str(p) for p in constrParamList)});"

""" 
Optionally creates function-call to the initialize function of a given variable extending from DspBlock in the form of:
varName->initialize(samplerate);

varName - the pointer to a DspBlock instance
needsInit - returns the statement if true, an empty string otherwise

"""
def genInit(varName: str, needsInit: bool) -> str:
    return f"{getPrefixedVarname(varName)}->initialize(samplerate);" if needsInit else ""

""" 
Returns a list of one function invocation per input. Each statement has the form of:
varName->setInputReference(sourceChannel, internalChannel);

varName - a pointer to a DspBlock isntance
inputs - a map of input definitions
        the key defines the internal Input channel
        the value must contain a sourceId to reference the source dspBlock and a sourceChannel field to
        reference the output channel of the source dspBlock.

"""
def genRouting(varName: str, inputs):
    methodCalls = []
    for inCh in inputs:
        outCh = f"{getPrefixedVarname(inputs[inCh]['sourceId'])}->getOutputChannel({inputs[inCh]['sourceChannel']})"
        t = f"{getPrefixedVarname(varName)}->setInputReference({outCh},{inCh});"
        methodCalls.append(t)
    return methodCalls

""" 
Returns a single method invocation of handle() for a given dspBlock instance.
Form: varName->handle();

"""
def genHandleCall(varName: str) -> str:
    return f"{getPrefixedVarname(varName)}->handle();"

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
                if ins[inCh]['sourceId'] not in handledIds:
                    unhandledIns.append(ins[inCh])
            if len(unhandledIns) == 0:
                errorState = False
                handledBlocks.append(block)
                blocks.remove(block)
        if errorState:
            raise Exception("Cycle in routing detected")

    return [genHandleCall(x['id']) for x in handledBlocks]

def genOutputRouting(physicalOuts):
    if physicalOuts == None:
        raise Exception
    outRoutings = []
    print(physicalOuts)
    for i in range(0, 4):
        if str(i) not in physicalOuts:
            outRoutings.append(f'physical_outs->writeChannel({{0}}, {i});')
            continue
        outRouting = physicalOuts[f'{i}']
        sourceId = outRouting['sourceId']
        sourceChannel = outRouting['sourceChannel']
        outRoutings.append(f'physical_outs->writeChannel({getPrefixedVarname(sourceId)}->getOutputChannel({sourceChannel}), {i});')
    return outRoutings
def genCpp(jsonData, requestId):
    # file = open('test.json')

    blocks = jsonData['blocks']
    try:
        blockDeclarations = [genBlockDeclaration(x['id']) for x in blocks]
        blockInstanciation = [getInstantiation(x['id'], x['type'], x['constructorParams']) for x in blocks]
        blockInitializations =[genInit(x['id'], True) for x in blocks]
        blockRoutings = [genRouting(x['id'], x['inputs']) for x in blocks]
        flatRoutings = [item for sublist in blockRoutings for item in sublist]
        orderedHandleCalls = genOrderedHandleCalls(blocks)
        genOutputRoutings = genOutputRouting(jsonData['physicalOut'])
    except Exception as e:
        traceback.print_exc()
        raise e
    current_directory = os.getcwd()
    final_directory = os.path.join(current_directory, 'buildspace', rf'{str(requestId)}')
    if not os.path.exists(final_directory):
        os.makedirs(final_directory)

    with open("buildspace/main.cpp.template", 'r') as templatefile:
        template = templatefile.read()
    with open(f"{final_directory}/Main.cpp", 'w+') as writefile:
        template = template.replace('%declarations%', '\n'.join(blockDeclarations))
        template = template.replace('%handle_invocations%', '\n'.join(orderedHandleCalls))
        template = template.replace('%handle_output%', '\n'.join(genOutputRoutings))
        template = template.replace('%instanciation%', '\n'.join(blockInstanciation))
        template = template.replace('%initialization%', '\n'.join(blockInitializations))
        template = template.replace('%routing%', '\n'.join(flatRoutings))
        writefile.write(template)
    return True
