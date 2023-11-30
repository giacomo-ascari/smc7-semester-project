import React from 'react';
import { useRete } from 'rete-react-plugin';
import logo from './logo.svg';
import './App.css';
import './rete.css';
import { createEditor } from './rete';
import styled from "styled-components";
import { Button, Switch } from "antd";

const Actions = styled.div`
  position: absolute;
  top: 1em;
  right: 1em;
  display: flex;
  align-items: center;
  gap: 0.5em;
`;

function App() {
  const [ref, editor] = useRete(createEditor)
  console.log(ref)
  console.log(editor)

  return (
    <div className="App">
      <header className="App-header">
        <h1>Dubby web programmer</h1>
        <Actions>
          <Button onClick={() => { /*invoke editor functions*/ }  }>Layout</Button>
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

export default App
