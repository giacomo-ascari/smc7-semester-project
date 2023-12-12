import { ClassicPreset } from 'rete';

const socket = new ClassicPreset.Socket('socket'); 

export class Node extends ClassicPreset.Node {
  width = 180;
  height = 120;
  type = "node";
  cppClassName = "DspBlock";
}

export class Connection<N extends Node> extends ClassicPreset.Connection<N, N> {}


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
  type = "ConstValue";
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
  type = "Osc";
  constructor() {
    super('Oscillator');
    this.addInput('0', new ClassicPreset.Input(socket, 'Frequency'));
    this.addOutput('0', new ClassicPreset.Output(socket, 'Output'));
  }
}

export class FeedbackDelayNode extends Node {
  width = 180;
  height = 180;
  type = "FeedbackDelay";
  constructor() {
    super('Feedback Delay');
    this.addInput('0', new ClassicPreset.Input(socket, 'Audio In'));
    this.addInput('1', new ClassicPreset.Input(socket, 'Dry/Wet'));
    this.addOutput('0', new ClassicPreset.Output(socket, 'Output'));
    this.addControl('0', new ClassicPreset.InputControl('number', { initial: 1024 }));
  }
}


// adder node
// equivalent to a 2 channel mixer with no gain control
export class AdderNode extends Node {
  width = 180;
  height = 195;
  type = "Sum";
  constructor(channelAmount: number) {
    super('Adder');
    for (let i = 0; i < channelAmount; i++) {
      this.addInput(i.toString(), new ClassicPreset.Input(socket, `Audio In ${i + 1}`));
      this.height += 22;
    }
    this.addControl('0', new ClassicPreset.InputControl('number', { initial: channelAmount, readonly: true }));
    this.addOutput('0', new ClassicPreset.Output(socket, 'Output'));
  }
}

export class MultiplierNode extends Node {
  width = 180;
  height = 180;
  type = "NMultiplier";
  constructor(channelAmount: number) {
    super('Multiplier');
    for (let i = 0; i < channelAmount; i++) {
      this.addInput(i.toString(), new ClassicPreset.Input(socket, `Audio In ${i + 1}`));
      this.height += 22;
    }
    this.addOutput('0', new ClassicPreset.Output(socket, 'Result'));
    this.addControl('0', new ClassicPreset.InputControl('number', {initial: channelAmount, readonly: true}))
  }
}

export class SubstractNode extends Node {
  width = 180;
  height = 180;
  type = "Sub";
  constructor(channelAmount: number) {
    super('Subtract');
    for (let i = 0; i < channelAmount; i++) {
      this.addInput(i.toString(), new ClassicPreset.Input(socket, `Audio In ${i + 1}`));
      this.height += 22;
    }
    this.addOutput('0', new ClassicPreset.Output(socket, 'Result'));
    this.addControl('0', new ClassicPreset.InputControl('number', {initial: channelAmount, readonly: true}))
  }
}


export class DivisionNode extends Node {
  width = 180;
  height = 180;
  type = "Div";
  constructor(channelAmount: number) {
    super('Division');
    for (let i = 0; i < channelAmount; i++) {
      this.addInput(i.toString(), new ClassicPreset.Input(socket, `Audio In ${i + 1}`));
      this.height += 22;
    }
    this.addOutput('0', new ClassicPreset.Output(socket, 'Result'));
    this.addControl('0', new ClassicPreset.InputControl('number', {initial: channelAmount, readonly: true}))
  }
}

export class UnipolarsiserNode extends Node {
  width = 180;
  height = 180;
  type = "Unipolariser";
  constructor() {
    super('Unipolarise');
    this.addInput('0', new ClassicPreset.Input(socket, 'Audio In'))
    this.addOutput('0', new ClassicPreset.Output(socket, 'Positive Out'))
  }
}

export class NoiseNode extends Node {
  width = 180;
  height = 180;
  type = "NoiseGen";
  constructor() {
    super('Noise');
    this.addInput('0', new ClassicPreset.Input(socket, 'Amp'))
    this.addOutput('0', new ClassicPreset.Output(socket, 'Noise Out'))
  }
}

export type FilterType = 'bandpass' | 'lowpass' | 'highpass';

export class FilterNode extends Node {
  width = 180;
  height = 180;
  type = "BPF";
  constructor(type: FilterType) {
    const display = type.charAt(0).toUpperCase() + type.slice(1);
    super(`${display} Filter`);
    switch (type) {
      case 'bandpass': {
        this.type = 'BPF';
        break;
      }
      case 'lowpass': {
        this.type = 'LPF';
        break;
      }
      case 'highpass': {
        this.type = 'HPF';
        break;
      }
    }
    this.addInput('0', new ClassicPreset.Input(socket, 'Audio In'));
    this.addInput('1', new ClassicPreset.Input(socket, 'Freq'));
    this.addInput('2', new ClassicPreset.Input(socket, 'Q'));
    this.addOutput('0', new ClassicPreset.Output(socket, 'Audio out'));
  }
}

