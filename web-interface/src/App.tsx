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

/*
#DFDFDF
#2D2E2F
#DF313C
#BABABA
*/

const BarStyle = styled.div`
  display: flex;
  align-items: center;
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
        <img src={logo} className="App-logo" alt="logo" width="200px" style={{ animation: 'none' }} />
        <h1>Dubby web programmer</h1>
        <ActionsStyle>
          
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
        <div ref={ref} className="rete"></div>
        <div id="console">
          <span> </span>
        </div>
        <p>SMC7 semester project - Ascari G., Hald K., Schnabel L., Tsakiris A. </p>
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
