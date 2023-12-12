import React from 'react';
import { useRete } from 'rete-react-plugin';
import logo from './logo.svg';
import './App.css';
import './rete.css';
import { createEditor } from './rete';
import styled from "styled-components";
import { Button, Switch } from "antd";
import axios from 'axios';


const Actions = styled.div`
  position: absolute;
  top: 1em;
  right: 1em;
  display: flex;
  align-items: center;
  gap: 0.5em;
`;

function App() {
  const [ref, editor] = useRete(createEditor) as any; // i know it's janky but let's roll

  return (
    <div className="App">
      <header className="App-header">
        <h1>Dubby web programmer</h1>
        <Actions>
          <Button onClick={() => { if (editor?.layout) editor.layout(); }  }>Layout</Button>
          <Button id='li45' onClick={() => { 
            if (editor?.getFlow) console.log(editor.getFlow());
            console.log(document.getElementById("root"));
          }}>Test</Button>
          <Button onClick={() => compile(editor)}>Compile</Button>
        </Actions>
        {/*<img src={logo} className="App-logo" alt="logo" style={{ animation: 'none' }} width="0.5"/>
        <a
          className="App-link"
          href="https://rete.js.org"
          target="_blank"
          rel="noopener noreferrer"
        >
          Learn Rete.js
        </a>*/}
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
