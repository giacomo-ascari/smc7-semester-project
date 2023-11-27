import { ClassicPreset as Classic, GetSchemes, NodeEditor } from 'rete';
import { DataflowEngine, DataflowNode } from 'rete-engine';

const socket = new Classic.Socket('socket');

class OscillatorNode extends Classic.Node implements DataflowNode {
  width = 180;
  height = 195;

  constructor() {
    super('Oscillator');

    this.addInput('f', new Classic.Input(socket, 'Frequency'));
    this.addInput('a', new Classic.Input(socket, 'Amplitude'));
    this.addOutput('value', new Classic.Output(socket, 'Output'));
    //this.addControl(
    //  'result',
    //  new Classic.InputControl('number', { initial: 0, readonly: true })
    //);
  }
  data(inputs: { f?: number[]; a?: number[] }) {
    const { f = [], a = [] } = inputs;
    //const sum = (a[0] || 0) + (b[0] || 0);
    //(this.controls['result'] as Classic.InputControl<'number'>).setValue(sum);
    let output = 0;
    return {
      value: output,
    };
  }
}

class NumberNode extends Classic.Node implements DataflowNode {
  width = 180;
  height = 120;

  constructor(initial: number, change?: (value: number) => void) {
    super('Number');

    this.addOutput('value', new Classic.Output(socket, 'Number'));
    this.addControl(
      'value',
      new Classic.InputControl('number', { initial, change })
    );
  }
  data() {
    const value = (this.controls['value'] as Classic.InputControl<'number'>)
      .value;

    return {
      value,
    };
  }
}

class AddNode extends Classic.Node implements DataflowNode {
  width = 180;
  height = 195;

  constructor() {
    super('Add');

    this.addInput('a', new Classic.Input(socket, 'A'));
    this.addInput('b', new Classic.Input(socket, 'B'));
    this.addOutput('value', new Classic.Output(socket, 'Number'));
    this.addControl(
      'result',
      new Classic.InputControl('number', { initial: 0, readonly: true })
    );
  }
  data(inputs: { a?: number[]; b?: number[] }) {
    const { a = [], b = [] } = inputs;
    const sum = (a[0] || 0) + (b[0] || 0);

    (this.controls['result'] as Classic.InputControl<'number'>).setValue(sum);

    return {
      value: sum,
    };
  }
}

export { OscillatorNode, NumberNode, AddNode }
