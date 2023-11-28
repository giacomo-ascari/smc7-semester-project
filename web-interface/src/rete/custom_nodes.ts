import { ClassicPreset } from 'rete';

const socket = new ClassicPreset.Socket('socket');

export class Node extends ClassicPreset.Node {
  width = 180;
  height = 120;
}

export class Connection<N extends Node> extends ClassicPreset.Connection<N, N> {}

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

export class NumberNode extends Node {
  width = 180;
  height = 120;

  constructor(initial: number) {
    super('Number');

    let change = (arg: any) => {
      const value = (this.controls['value'] as ClassicPreset.InputControl<'number'>).value;
      return { value };
    }
    
    this.addOutput('value', new ClassicPreset.Output(socket, 'Output'));
    this.addControl('value', new ClassicPreset.InputControl('number', { initial, change }));
  }


}

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
