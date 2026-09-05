import React from 'react';
import { Link } from 'react-router-dom';
import '../styles/Navigation.css';

function Navigation({ isDarkMode, onToggleDarkMode }) {
  return (
    <nav className="navigation">
      <div className="nav-brand">
        <h1>🎯 Coil Analyzer</h1>
      </div>
      <ul className="nav-links">
        <li><Link to="/">Measurement</Link></li>
        <li><Link to="/history">History</Link></li>
      </ul>
      <button className="theme-toggle" onClick={onToggleDarkMode}>
        {isDarkMode ? '☀️' : '🌙'}
      </button>
    </nav>
  );
}

export default Navigation;
