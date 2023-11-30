import { ClassicPreset } from 'rete';

const socket = new ClassicPreset.Socket('socket');

export class Node extends ClassicPreset.Node {
  width = 180;
  height = 120;
}

export class Connection<N extends Node> extends ClassicPreset.Connection<N, N> {}


// adder node
// equivalent to a 2 channel mixer with no gain control
export class AdderNode extends Node {
  width = 180;
  height = 195;

  constructor() {
    super('Adder');

    this.addInput('a', new ClassicPreset.Input(socket, 'A'));
    this.addInput('b', new ClassicPreset.Input(socket, 'B'));
    this.addOutput('value', new ClassicPreset.Output(socket, 'Output'));
  }
}

// dubby knob 1 node
export class DubbyKnob1Node extends Node {
  width = 180;
  height = 90;
  constructor() {
    super('Dubby knob 1');
    this.addOutput('value', new ClassicPreset.Output(socket, 'Output'));
  }
}

// dubby output 1 node
export class DubbyOutput1Node extends Node {
  width = 180;
  height = 90;
  constructor() {
    super('Dubby output 1');
    this.addInput('value', new ClassicPreset.Input(socket, 'Input'));
  }
}

// number node
// emits a constant value
export class NumberNode extends Node {
  width = 180;
  height = 120;

  constructor() {
    super('Number');

    let change = (arg: any) => {
      const value = (this.controls['value'] as ClassicPreset.InputControl<'number'>).value;
      return { value };
    }
    
    let initial: number = 1;
    this.addOutput('value', new ClassicPreset.Output(socket, 'Output'));
    this.addControl('value', new ClassicPreset.InputControl('number', { initial, change }));
  }
}

// oscillator node
// oscialttor, with 2 inputs and 1 output
export class OscillatorNode extends Node {
  width = 180;
  height = 195;

  constructor() {
    super('Oscillator');
    
    this.addInput('f', new ClassicPreset.Input(socket, 'Frequency'));
    this.addInput('a', new ClassicPreset.Input(socket, 'Amplitude'));
    this.addOutput('value', new ClassicPreset.Output(socket, 'Output'));
  }
}

