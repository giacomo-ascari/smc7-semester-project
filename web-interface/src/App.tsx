import React, { useState } from 'react';
import axios from 'axios';
import { useRete } from 'rete-react-plugin';
import logo from './logo.svg';
import './App.css';
import './rete.css';
import { createEditor } from './rete';
import styled from "styled-components";

import { bigFlash } from './dfu';

/*
#DFDFDF
#2D2E2F
#DF313C
#BABABA
*/

const Button = styled.button`
  font-family: "Questrial", sans-serif;
  font-size: 13pt;
  background-color: #DF313C;
  color: white;
  padding: 0.5em 0.5em;
  margin: 0.5em 0.5em;
  border-radius: 0.5em;
  border-style: hidden;
  cursor: pointer;
  &:disabled {
    background-color: #2D2E2F;
    cursor: default;
  }
`;

const ActionsStyle = styled.div`
display: flex;
  flex-direction: row;
  align-items: left;
  justify-content: left;
`;



function App() {
  const [ref, editor] = useRete(createEditor) as any; // i know it's janky but let's roll
  const [isDisabled, setDisabled] = useState(false);

  return (
    <div className="App">
      <header className="App-header">
        <img src={logo} className="App-logo" alt="logo" style={{ width: 120, height: 120, marginTop: '0.5em', animation: 'none' }} />
        <h1>Dubby web programmer</h1>
        <p>Visual programming tool developed for Dubby, by Componental</p>
        
        <ActionsStyle>
        
          <Button onClick={() => {
            if (editor?.layout) editor.layout();
          }}>Rearrange nodes</Button>

          <Button onClick={() => { 
            alert("function not implemented. but CTRL+Z works!")
          }}>↩</Button>

          <Button onClick={() => { 
            alert("function not implemented. but CTRL+Y works!")
          }}>↪</Button>

          <Button disabled={isDisabled} onClick={async () => {
            setDisabled(true);
            btnFlashClick(editor, () => {setDisabled(false);});
          }}>Flash!</Button>

          <Button onClick={() => { 
            testWithoutFlash(editor);
            alert("function not implemented yet!")
          }}>❔</Button>

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

async function btnFlashClick(editor: any, callback: any) {
  if (editor?.getFlow) {
    let reqBody = editor.getFlow();
    await bigFlash(reqBody, 'http://localhost:5000/compiler', callback);
  }
}

// Please don't delete yet.
function testWithoutFlash(editor: any) {
  if (editor?.getFlow) {
    let reqBody = editor.getFlow();
    return new Promise((resolve) => {
      let buffer;
      let raw = new XMLHttpRequest();
      let fname = 'http://localhost:8000/compiler';
      raw.open("POST", fname, true);
      raw.responseType = "arraybuffer"
      raw.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
      raw.send(JSON.stringify(reqBody))
  }).then()
  }
}

export default App
