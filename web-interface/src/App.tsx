import React from 'react';
import { useRete } from 'rete-react-plugin';
import logo from './logo.svg';
import './App.css';
import './rete.css';
import { createEditor } from './rete';
import styled from "styled-components";
import { Button, Switch } from "antd";

import { connectButton } from './dfu';

const BarStyle = styled.div`
  display: flex;
  align-items: center;
`;

const ConsoleStyle = styled.div`
font-size: 11pt;
color:white;
font-family:Consolas;
background-color: black;
padding: 5pt;
`;

const ActionsStyle = styled.div`
display: flex;
align-items: left;
justify-content: left;
`;

function App() {
  const [ref, editor] = useRete(createEditor) as any; // i know it's janky but let's roll

  return (
    <div className="App">
      <header className="App-header">
        <h1>Dubby web programmer</h1>
        <ActionsStyle>
          <ConsoleStyle>
            <p id="console">Console, id=console</p>
          </ConsoleStyle>
          <BarStyle>

            <Button onClick={() => {
              if (editor?.layout) editor.layout();
            }}>Layout</Button>

            <Button onClick={() => { 
              if (editor?.getFlow) console.log(editor.getFlow());
              console.log(document.getElementById("root"));
            }}>Test</Button>

            <Button onClick={() => { 
              connectButton();
            }}>ConnectTest</Button>
          </BarStyle>
        </ActionsStyle>
        <div ref={ref} className="rete" style={{ height: "75vh", width: "80vw" }}></div>
      </header>
    </div>
  );
}

function compile(editor: any) {
  if (editor?.getFlow) {
    axios.post('http://127.0.0.1:5000/compiler', {
      "blocks": editor.getFlow()},{
        responseType: 'blob',
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/bin'
        }
    }).then((response) => {
  }).catch((error) => console.log(error));

    console.log(editor.getFlow());
  }
}

export default App
