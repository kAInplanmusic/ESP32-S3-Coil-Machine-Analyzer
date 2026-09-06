import React from 'react';
import '../styles/WaveformDisplay.css';

function WaveformDisplay({ measurement, results }) {
  const data = measurement?.data || results || {};

  return (
    <div className="waveform-display">
      <h4>Waveform Analysis</h4>
      <canvas id="waveform-chart" width="800" height="300"></canvas>
      
      <div className="waveform-info">
        <p>Waveform visualization will be rendered here using Chart.js</p>
        <p>Peak 1: {data.peak1?.toFixed(2) || 'N/A'}</p>
        <p>Peak 2: {data.peak2?.toFixed(2) || 'N/A'}</p>
      </div>
    </div>
  );
}

export default WaveformDisplay;
