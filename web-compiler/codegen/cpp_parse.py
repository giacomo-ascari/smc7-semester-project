import json
import sys
import os
import traceback

"""
Returns the variable with a prefix
"""
def getPrefixedVarname(varName: str) -> str:
    return f"node_{varName}"

""" 
Converts a string x to a DspNode declaration. The corresponding output is
DspNode * x;
"""
def genNodeDeclaration(varName: str) -> str:
    return f"DspNode * {getPrefixedVarname(varName)};"

""" 
Creates an instantiation statement in the form of varName = new childClass(constrParamList..., AUDIO_BLOCK_SIZE);

varName - pointer the instance should be assigned to
childClass - the concrete implementation of DspNode used
constrParamList - optional and additional constructor parameters

"""
def getInstantiation(varName: str, childClass: str, constrParamList) -> str:
    constrParamList.append('AUDIO_BLOCK_SIZE')
    return f"{getPrefixedVarname(varName)} = new {childClass}({', '.join(str(p) for p in constrParamList)});"

""" 
Optionally creates function-call to the initialize function of a given variable extending from DspNode in the form of:
varName->initialize(samplerate);

varName - the pointer to a DspNode instance
needsInit - returns the statement if true, an empty string otherwise

"""
def genInit(varName: str, needsInit: bool) -> str:
    return f"{getPrefixedVarname(varName)}->initialize(samplerate);" if needsInit else ""

""" 
Returns a list of one function invocation per input. Each statement has the form of:
varName->setInputReference(sourceChannel, internalChannel);

varName - a pointer to a DspNoce isntance
inputs - a map of input definitions
        the key defines the internal Input channel
        the value must contain a sourceId to reference the source dspNode and a sourceChannel field to
        reference the output channel of the source dspNode.

"""
def genRouting(varName: str, inputs):
    methodCalls = []
    for inCh in inputs:
        outCh = f"{getPrefixedVarname(inputs[inCh]['sourceId'])}->getOutputChannel({inputs[inCh]['sourceChannel']})"
        t = f"{getPrefixedVarname(varName)}->setInputReference({outCh},{inCh});"
        methodCalls.append(t)
    return methodCalls

""" 
Returns a single method invocation of process() for a given dspNode instance.
Form: varName->process();

"""
def genProcessCall(varName: str) -> str:
    return f"{getPrefixedVarname(varName)}->process();"

""" 
Return a list of process() method invocations for all DspNode.
The invocations are ordered so that DspNodes without any input channels are the first elements, 
subsequent are invocations of DspNode that only rely on the first elements, then Nodes relying only on the first or the second elements, and so on.

The provided nodes may not contain cycles. If it does, this method will never halt!
"""
def genOrderedProcessCalls(nodes):
    handledNodes = [x for x in nodes if not x['inputs']]
    for node in handledNodes:
        nodes.remove(node)
    while len(nodes) > 0:
        handledIds = [x['id'] for x in handledNodes]
        handledIds.append('dubbyAudioIn')
        errorState = True
        for node in nodes:
            ins = node['inputs']
            unhandledIns = []
            for inCh in ins:
                if ins[inCh]['sourceId'] not in handledIds:
                    unhandledIns.append(ins[inCh])
            if len(unhandledIns) == 0:
                errorState = False
                handledNodes.append(node)
                nodes.remove(node)
        if errorState:
            raise Exception("Cycle in routing detected")

    return [genProcessCall(x['id']) for x in handledNodes]

def genOutputRouting(physicalOuts):
    if physicalOuts == None:
        raise Exception
    outRoutings = []
    print(physicalOuts)
    for i in range(0, 4):
        if str(i) not in physicalOuts:
            outRoutings.append(f'dubbyAudioOuts->writeChannel(EMPTY_BUFFER, {i});')
            continue
        outRouting = physicalOuts[f'{i}']
        sourceId = outRouting['sourceId']
        sourceChannel = outRouting['sourceChannel']
        outRoutings.append(f'dubbyAudioOuts->writeChannel({getPrefixedVarname(sourceId)}->getOutputChannel({sourceChannel}), {i});')
    return outRoutings
def genCpp(jsonData, requestId):
    # file = open('test.json')

    nodes = jsonData['nodes']
    try:
        nodeDeclarations = [genNodeDeclaration(x['id']) for x in nodes]
        nodeInstanciation = [getInstantiation(x['id'], x['type'], x['constructorParams']) for x in nodes]
        nodeInitializations =[genInit(x['id'], True) for x in nodes]
        nodeRoutings = [genRouting(x['id'], x['inputs']) for x in nodes]
        flatRoutings = [item for sublist in nodeRoutings for item in sublist]
        orderedProcessCalls = genOrderedProcessCalls(nodes)
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
        template = template.replace('%declarations%', '\n'.join(nodeDeclarations))
        template = template.replace('%handle_invocations%', '\n'.join(orderedProcessCalls))
        template = template.replace('%handle_output%', '\n'.join(genOutputRoutings))
        template = template.replace('%instanciation%', '\n'.join(nodeInstanciation))
        template = template.replace('%initialization%', '\n'.join(nodeInitializations))
        template = template.replace('%routing%', '\n'.join(flatRoutings))
        writefile.write(template)
    return True
