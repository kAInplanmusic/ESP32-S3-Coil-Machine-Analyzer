import React from 'react'

export const MeasurementDisplay: React.FC = () => {
  return (
    <div className="measurement-display">
      <div className="progress-section">
        <h3>Measurement Progress</h3>
        <div className="progress-bar">
          <div className="progress-fill" style={{ width: '0%' }}></div>
        </div>
        <p className="progress-text">0 / 100 cycles</p>
      </div>

      <div className="stats-section">
        <div className="stat-card">
          <label>CPS (Cycles/Second)</label>
          <span className="stat-value">--</span>
        </div>
        <div className="stat-card">
          <label>Quality Score</label>
          <span className="stat-value">--</span>
        </div>
        <div className="stat-card">
          <label>Peak Amplitude</label>
          <span className="stat-value">--</span>
        </div>
      </div>

      <div className="waveform-section">
        <h3>Live Waveform</h3>
        <canvas id="waveform-canvas"></canvas>
      </div>
    </div>
  )
}
