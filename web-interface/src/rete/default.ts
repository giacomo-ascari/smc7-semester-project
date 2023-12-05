import { ClassicPreset as Classic, GetSchemes, NodeEditor } from 'rete';

import { Area2D, AreaExtensions, AreaPlugin } from 'rete-area-plugin';

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

type Schemes = GetSchemes<Custom.Node, Custom.Connection<Custom.Node>>;

class Connection<A extends Custom.Node, B extends Custom.Node> extends Classic.Connection<
  A,
  B
> {}



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
      ['Add', () => new Custom.AdderNode()],
      ["Dubby", [
        ['Dubby knobs IN', () => new Custom.DubbyKnobInputsNode()],
        ['Dubby audio OUT', () => new Custom.DubbyAudioOutputsNode()],
      ]],
      ['Number', () => new Custom.NumberNode()],
      ['Oscillator', () => new Custom.OscillatorNode()],
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

  // Default nodes in the editor

  console.log("made it here 1")

  const a = new Custom.NumberNode();
  const b = new Custom.NumberNode();
  const add = new Custom.AdderNode();
  const osc = new Custom.OscillatorNode();

  console.log("made it here 2")


  await editor.addNode(osc);
  await editor.addNode(a);
  await editor.addNode(b);
  await editor.addNode(add);

  // Default connections in the editor

  await editor.addConnection(new Connection(a, '0', add, '0'));
  await editor.addConnection(new Connection(b, '0', add, '1'));

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
      let blocks: any = [];

      // building the nodes
      // excluding connections
      editor.getNodes().forEach(n => {
        let block: any = {
          type: n.type,
          id: n.id,
          constructorParams: {},
          inputs: {},
          //outputs: {}
        };
        if (n.controls) {
          Object.keys(n.controls).forEach(e => {
            block.constructorParams = {...block.constructorParams, [e]: (n.controls[e] as any).value};
          });
        };
        blocks.push(block)
      })

      // building the connections
      // time to make montresor proud
      editor.getConnections().forEach(c => {
        blocks.forEach((b: any) => {
          //if (c.source == b.id) {
          //  b.outputs = {...b.outputs, [c.sourceOutput]: {target: c.target, targetInput: c.targetInput}};
          //}
          if (c.target == b.id) {
            b.inputs = {...b.inputs, [c.targetInput]: {source: c.source, sourceOutput: c.sourceOutput}};
          }
        });
      })

      return blocks;
    },
    layout: async () => {
      await arrange.layout();
      AreaExtensions.zoomAt(area, editor.getNodes());
    }
  };
}