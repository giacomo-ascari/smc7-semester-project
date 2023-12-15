import { ClassicPreset as Classic, GetSchemes, NodeEditor } from 'rete';

import { Area2D, AreaExtensions, AreaPlugin } from 'rete-area-plugin';
import log from '../utils'

import {
  ReactPlugin,
  ReactArea2D,
  Presets as ReactPresets,
} from 'rete-react-plugin';
import { createRoot } from 'react-dom/client';

import {
  AutoArrangePlugin,
  Presets as ArrangePresets,
  ArrangeAppliers
} from 'rete-auto-arrange-plugin';
import {
  ContextMenuPlugin,
  ContextMenuExtra,
  Presets as ContextMenuPresets,
} from 'rete-context-menu-plugin';
import { MinimapExtra, MinimapPlugin } from 'rete-minimap-plugin';
import {
  ReroutePlugin,
  RerouteExtra,
  RerouteExtensions,
} from 'rete-connection-reroute-plugin';
import {
  ConnectionPlugin,
  Presets as ConnectionPresets
} from "rete-connection-plugin";
import {
  HistoryExtensions,
  HistoryPlugin,
  Presets as HistoryPresets
} from "rete-history-plugin";

// custom imports
import * as Custom from './custom_nodes';
import { Root } from 'rete-react-plugin/_types/utils';

type Schemes = GetSchemes<Custom.Node, Custom.Connection<Custom.Node>>;

class Connection<A extends Custom.Node, B extends Custom.Node> extends Classic.Connection<
  A,
  B
> { }



type AreaExtra =
  | Area2D<Schemes>
  | ReactArea2D<Schemes>
  | ContextMenuExtra
  | MinimapExtra
  | RerouteExtra;

const socket = new Classic.Socket('socket');

export async function createEditor(container: HTMLElement) {

  // Used plugins

  const editor = new NodeEditor<Schemes>();
  const area = new AreaPlugin<Schemes, AreaExtra>(container);
  const connection = new ConnectionPlugin<Schemes, AreaExtra>();
  const reactRender = new ReactPlugin<Schemes, AreaExtra>({ createRoot });
  const minimap = new MinimapPlugin<Schemes>();
  const reroutePlugin = new ReroutePlugin<Schemes>();
  const history = new HistoryPlugin<Schemes>();

  // Context menu plugin (right click menu) configuration
  const contextMenu = new ContextMenuPlugin<Schemes>({
    items: ContextMenuPresets.classic.setup([
      ['Number', () => new Custom.NumberNode()],
      ['Oscillator', () => new Custom.OscillatorNode()],
      ['Feedback Delay', () => new Custom.FeedbackDelayNode()],
      ['Filter', [
        ['Lowpass', () => new Custom.FilterNode('lowpass')],
        ['Bandpas', () => new Custom.FilterNode('bandpass')],
        ['Highpass', () => new Custom.FilterNode('highpass')]
      ]],
      ['Math', [
        ['Add', [
          ['2 Channels', () => new Custom.AdderNode(2)],
          ['3 Channels', () => new Custom.AdderNode(3)],
          ['4 Channels', () => new Custom.AdderNode(4)],
          ['5 Channels', () => new Custom.AdderNode(5)],
        ]],
        ['Multiply', [
          ['2 Channels', () => new Custom.MultiplierNode(2)],
          ['3 Channels', () => new Custom.MultiplierNode(3)],
          ['4 Channels', () => new Custom.MultiplierNode(4)],
          ['5 Channels', () => new Custom.MultiplierNode(5)],
        ]],
        ['Subtract', [
          ['2 Channels', () => new Custom.SubstractNode(2)],
          ['3 Channels', () => new Custom.SubstractNode(3)],
          ['4 Channels', () => new Custom.SubstractNode(4)],
          ['5 Channels', () => new Custom.SubstractNode(5)],
        ]],
        ['Divide', [
          ['2 Channels', () => new Custom.DivisionNode(2)],
          ['3 Channels', () => new Custom.DivisionNode(3)],
          ['4 Channels', () => new Custom.DivisionNode(4)],
          ['5 Channels', () => new Custom.DivisionNode(5)],
        ]],
        ['Scaler', () => new Custom.ScalerNode()]
      ]],
      ['Unipolarise', () => new Custom.UnipolarsiserNode()],
      ['Compressor', () => new Custom.CompressorNode()],
      ['Noise', () => new Custom.NoiseNode()]
    ]),
  });

  // Plugin configuration
  HistoryExtensions.keyboard(history);

  history.addPreset(HistoryPresets.classic.setup());

  connection.addPreset(ConnectionPresets.classic.setup());

  editor.use(area);
  area.use(connection);
  area.use(reactRender);
  area.use(history);

  area.use(contextMenu);
  area.use(minimap);
  reactRender.use(reroutePlugin);

  reactRender.addPreset(ReactPresets.classic.setup());
  reactRender.addPreset(ReactPresets.contextMenu.setup());
  reactRender.addPreset(ReactPresets.minimap.setup());
  reactRender.addPreset(
    ReactPresets.reroute.setup({
      contextMenu(id) {
        reroutePlugin.remove(id);
      },
      translate(id, dx, dy) {
        reroutePlugin.translate(id, dx, dy);
      },
      pointerdown(id) {
        reroutePlugin.unselect(id);
        reroutePlugin.select(id);
      },
    })
  );

  // Custom event handler
  editor.addPipe((context) => {
    if (context.type == 'noderemove') {
      if(context.data.type == 'DubbyKnobs' || context.data.type == 'dubbyaudioout') {
        log('Sorry this node cannot deleted');
        return;
      }
    }
    return context;
  });

  // Default nodes in the editor
  const a = new Custom.NumberNode();
  const osc = new Custom.OscillatorNode();
  const dubOut = new Custom.DubbyAudioOutputsNode();
  const dubKnobs = new Custom.DubbyKnobInputsNode();
  const dubIn = new Custom.DubbyAudioInputsNode();

  await editor.addNode(osc);
  await editor.addNode(a);
  await editor.addNode(dubOut);
  await editor.addNode(dubKnobs);
  await editor.addNode(dubIn);

  // Default connections in the editor

  await editor.addConnection(new Connection(a, '0', osc, '0'));
  await editor.addConnection(new Connection(osc, '0', dubOut, '0'));
  await editor.addConnection(new Connection(osc, '0', dubOut, '1'));

  // Autoarrange elements in the area

  const arrange = new AutoArrangePlugin<Schemes>();
  arrange.addPreset(ArrangePresets.classic.setup());
  area.use(arrange);
  await arrange.layout();

  // Misc (don't know exactly what it does)

  AreaExtensions.zoomAt(area, editor.getNodes());

  AreaExtensions.simpleNodesOrder(area);

  const selector = AreaExtensions.selector();
  const accumulating = AreaExtensions.accumulateOnCtrl();

  AreaExtensions.selectableNodes(area, selector, { accumulating });
  RerouteExtensions.selectablePins(reroutePlugin, selector, accumulating);



  return {
    destroy: () => area.destroy(),
    getFlow: () => {
      let blocks: BlockDTO[] = [];

      // building the nodes

      const allBlocks = editor.getNodes().map(n => {
        let block: BlockDTO = {
          type: n.type,
          id: n.id,
          constructorParams: [],
          inputs: {},
          //outputs: {}
        };
        if (n.type == 'DubbyKnobs') {
          block.constructorParams.push('dubby');
        }
        if (n.controls) {
          Object.keys(n.controls).forEach(e => {
            // block.constructorParams = [(n.controls[e] as any).value];
            block.constructorParams.push((n.controls[e] as any).value)
          });
        };
        return block;
      })

      blocks = allBlocks.filter(n => n.type != 'dubbyaudioout' && n.type != 'DubbyAudioIns');

      // excluding connections
      // editor.getNodes()
      //   .filter(n => n.type != 'dubbyaudioout' && n.type != 'DubbyAudioIns')
      //   .forEach(n => {
      //     let block: BlockDTO = {
      //       type: n.type,
      //       id: n.id,
      //       constructorParams: [],
      //       inputs: {},
      //       //outputs: {}
      //     };
      //     if (n.type == 'DubbyKnobs') {
      //       block.constructorParams.push('dubby');
      //     }
      //     if (n.controls) {
      //       Object.keys(n.controls).forEach(e => {
      //         // block.constructorParams = [(n.controls[e] as any).value];
      //         block.constructorParams.push((n.controls[e] as any).value)
      //       });
      //     };
      //     blocks.push(block)
      //   })

      // building the connections
      // time to make montresor proud
      const audioInNodeId = allBlocks.find(b => b.type == 'DubbyAudioIns')?.id ?? '-1';

      editor.getConnections()
        .forEach(c => {
          allBlocks
            .filter(b => b.type != 'dubbyaudioout')
            .forEach(b => {
              if (c.target == b.id) {
                if (c.source == audioInNodeId) {
                  b.inputs = { ...b.inputs, [c.targetInput]: { sourceId: Custom.DubbyAudioInputsNode.DEFAULT_ID, sourceChannel: c.sourceOutput } };
                  return;
                }
                b.inputs = { ...b.inputs, [c.targetInput]: { sourceId: c.source, sourceChannel: c.sourceOutput } };
              }
            });
        })

      const outBlockIds = editor.getNodes()
        .filter(n => n.type == 'dubbyaudioout').map(n => n.id);
      let outputs = {}
      editor.getConnections()
        .filter(conn => outBlockIds.includes(conn.target))
        .sort((a, b) => a.targetInput.localeCompare(b.targetInput))
        .forEach(conn => {
          if(Object.keys(outputs).includes(conn.targetInput)) return;
          outputs = {
            ...outputs,
            [conn.targetInput]: { sourceId: conn.source, sourceChannel: conn.sourceOutput }
          }
        })

      return {
        physicalOut: outputs,
        blocks: blocks
      };
    },
    layout: async () => {
      await arrange.layout();
      AreaExtensions.zoomAt(area, editor.getNodes());
    }
  };
}

interface BlockDTO {
  type: string,
  id: string,
  constructorParams: string[],
  inputs: any,
  //outputs: {}
}