import { ClassicPreset } from 'rete';

const socket = new ClassicPreset.Socket('socket');

export class Node extends ClassicPreset.Node {
  width = 180;
  height = 120;
  type = "node";
}

export class Connection<N extends Node> extends ClassicPreset.Connection<N, N> {}


// adder node
// equivalent to a 2 channel mixer with no gain control
export class AdderNode extends Node {
  width = 180;
  height = 195;
  type = "adder";
  constructor() {
    super('Adder');
    this.addInput('0', new ClassicPreset.Input(socket, 'A'));
    this.addInput('1', new ClassicPreset.Input(socket, 'B'));
    this.addOutput('0', new ClassicPreset.Output(socket, 'Output'));
  }
}

// dubby knobs INPUT node
export class DubbyKnobInputsNode extends Node {
  width = 180;
  height = 200;
  type = "dubbyknobs";
  constructor() {
    super('Dubby knob INs');
    this.addOutput('0', new ClassicPreset.Output(socket, 'Knob 1'));
    this.addOutput('1', new ClassicPreset.Output(socket, 'Knob 2'));
    this.addOutput('2', new ClassicPreset.Output(socket, 'Knob 3'));
    this.addOutput('3', new ClassicPreset.Output(socket, 'Knob 4'));
  }
}

// dubby audio OUTPUT node
export class DubbyAudioOutputsNode extends Node {
  width = 180;
  height = 200;
  type = "dubbyaudioout";
  constructor() {
    super('Dubby audio OUTs');
    this.addInput('0', new ClassicPreset.Input(socket, 'Left 1'));
    this.addInput('1', new ClassicPreset.Input(socket, 'Right 1'));
    this.addInput('2', new ClassicPreset.Input(socket, 'Left 2'));
    this.addInput('3', new ClassicPreset.Input(socket, 'Right 2'));
  }
}

// number node
// emits a constant value
export class NumberNode extends Node {
  width = 180;
  height = 120;
  type = "const";
  constructor() {
    super('Number');
    let change = (arg: any) => {
      const value = (this.controls['0'] as ClassicPreset.InputControl<'number'>).value;
      return { value };
    }
    let initial: number = 1;
    this.addOutput('0', new ClassicPreset.Output(socket, 'Output'));
    this.addControl('0', new ClassicPreset.InputControl('number', { initial, change }));
  }
}

// oscillator node
// oscialttor, with 2 inputs and 1 output
export class OscillatorNode extends Node {
  width = 180;
  height = 180;
  type = "oscillator";
  constructor() {
    super('Oscillator');
    this.addInput('0', new ClassicPreset.Input(socket, 'Frequency'));
    this.addInput('1', new ClassicPreset.Input(socket, 'Amplitude'));
    this.addOutput('0', new ClassicPreset.Output(socket, 'Output'));
  }
}

