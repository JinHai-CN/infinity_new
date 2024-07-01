import logo from './logo.svg';
import './App.css';
import { HashRouter, Routes, Route } from 'react-router-dom';
import Home from './pages/home';
import Databases from './pages/databases';
import Database from './pages/database';

function App() {
  return (
    <div className="App">
      <header className="App-header">
      <HashRouter>
          <Routes>
            <Route path="/" element={<Home />} />
            <Route path="/databases" element={<Databases />} />
            <Route path="/databases/:databaseName" element={<Database />} />
          </Routes>
        </HashRouter>
      </header>
    </div>
  );
}

export default App;
