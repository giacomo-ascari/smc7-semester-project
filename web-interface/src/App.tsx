import React from 'react';
import axios from 'axios';
import { useRete } from 'rete-react-plugin';
import logo from './logo.svg';
import './App.css';
import './rete.css';
import { createEditor } from './rete';
import styled from "styled-components";
import { Button, Switch } from "antd";

import { bigFlash } from './dfu';

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
border-style: solid;
border-color: #737F96;
height:5.5em;
min-width: "100px";
overflow:auto;
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
          <ConsoleStyle id="console">
            <span> </span>
          </ConsoleStyle>
          <BarStyle>

            <Button onClick={() => {
              if (editor?.layout) editor.layout();
            }}>Rearrange</Button>

            <Button onClick={() => { 
              btnTestClick(editor);
            }}>Test</Button>

            <Button id="bigFlashButton" onClick={() => { 
              btnFlashClick(editor);
            }}>Flash!</Button>
          </BarStyle>
        </ActionsStyle>
        <div ref={ref} className="rete" style={{ height: "75vh", width: "80vw" }}></div>
      </header>
    </div>
  );
}

function btnFlashClick(editor: any) {
  if (editor?.getFlow) {
    let reqBody = editor.getFlow();
    bigFlash(reqBody, 'http://127.0.0.1:5000/compiler');
  }
}

function btnTestClick(editor: any) {
  if (editor?.getFlow) {
    console.log(editor.getFlow());
    return new Promise((resolve) => {
      let buffer;
      let raw = new XMLHttpRequest();
      let fname = 'http://127.0.0.1:5000/compiler';
      raw.open("POST", fname, true);
      raw.responseType = "arraybuffer"
      raw.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
      raw.onreadystatechange = function ()
      {
          if (this.readyState === 4 && this.status === 200) {
              resolve(this.response)
          }    
      }
      raw.send(JSON.stringify(editor.getFlow()))
  }).then(() => console.log('something'))
  }
  console.log(document.getElementById("root"));
}
export default App
