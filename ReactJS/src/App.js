import './App.css';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import LoginRegister from './Components/view/LoginRegister/LoginRegister';
import Home from './Components/view/Home/Home';
import Info from './Components/view/Information/Info';
import Logpage from './Components/view/Logpage/logpage';

function App() {
  return (
      <Router>
          <Routes>
              <Route path="/" element={<LoginRegister />} />
              <Route path="/home" element={<Home />} />
              <Route path="/infomation" element={<Info />} />
              <Route path="/log" element={<Logpage/>} />
          </Routes>
      </Router>
  );
}

export default App;
