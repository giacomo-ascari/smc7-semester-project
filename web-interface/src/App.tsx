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
              if (editor?.getFlow) console.log(editor.getFlow());
              console.log(document.getElementById("root"));
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
    bigFlash({blocks: editor.getFlow()}, 'http://127.0.0.1:5000/compiler');
    /*axios.post('http://127.0.0.1:5000/compiler', {
      "blocks": editor.getFlow()},{
        responseType: 'arraybuffer',
        headers: {
            'Content-Type': 'application/json',
            'Accept': 'application/bin'
        }
    }).then((response: any) => {
      bigFlash(response.data);
    }).catch((error: any) => console.log(error));

    console.log(editor.getFlow());*/
  }
}

export default App
