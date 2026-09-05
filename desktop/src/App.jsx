import React from 'react';
import { BrowserRouter as Router, Routes, Route, Link } from 'react-router-dom';
import './styles/index.css';
import MeasurementDesktop from './components/MeasurementDesktop';
import HistoryDesktop from './components/HistoryDesktop';
import AnalysisDesktop from './components/AnalysisDesktop';

export default function App() {
  const [darkMode, setDarkMode] = React.useState(false);

  React.useEffect(() => {
    if (darkMode) {
      document.body.classList.add('dark-mode');
    } else {
      document.body.classList.remove('dark-mode');
    }
  }, [darkMode]);

  return (
    <Router>
      <div className="app">
        <nav className="desktop-nav">
          <div className="nav-brand">🎯 Coil Machine Analyzer</div>
          <div className="nav-links">
            <Link to="/" className="nav-link">
              📊 Measurement
            </Link>
            <Link to="/history" className="nav-link">
              📈 History
            </Link>
            <Link to="/analysis" className="nav-link">
              🔍 Analysis
            </Link>
          </div>
          <button
            className="nav-toggle-theme"
            onClick={() => setDarkMode(!darkMode)}
            title={darkMode ? 'Light Mode' : 'Dark Mode'}
          >
            {darkMode ? '☀️' : '🌙'}
          </button>
        </nav>

        <main className="desktop-page">
          <Routes>
            <Route
              path="/"
              element={<MeasurementDesktop darkMode={darkMode} />}
            />
            <Route path="/history" element={<HistoryDesktop />} />
            <Route path="/analysis" element={<AnalysisDesktop />} />
          </Routes>
        </main>
      </div>
    </Router>
  );
}
