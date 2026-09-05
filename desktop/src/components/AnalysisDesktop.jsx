import React from 'react';
import '../styles/AnalysisDesktop.css';

export default function AnalysisDesktop() {
  const measurement = {
    id: 1,
    timestamp: new Date(),
    cps: 85.3,
    qualityScore: 92,
    durationMs: 15000,
    harmonicRatioH2: 0.45,
    harmonicRatioH3: 0.12,
    thd: 48.5,
    decayRate: 0.0234,
    qFactor: 134.5,
    peakAmplitude: 0.87,
    noiseFloor: 0.05,
  };

  return (
    <div className="analysis-page">
      <div className="analysis-header">
        <h1>📈 Detailed Analysis</h1>
        <p>{measurement.timestamp.toLocaleString()}</p>
      </div>

      <div className="analysis-content">
        <div className="analysis-section">
          <h2>Measurement Parameters</h2>
          <div className="parameter-grid">
            <div className="parameter-item">
              <label>Cycles Per Second</label>
              <value>{measurement.cps.toFixed(2)}</value>
            </div>
            <div className="parameter-item">
              <label>Quality Score</label>
              <value className="good">{measurement.qualityScore}%</value>
            </div>
            <div className="parameter-item">
              <label>Duration</label>
              <value>{measurement.durationMs}ms</value>
            </div>
            <div className="parameter-item">
              <label>Peak Amplitude</label>
              <value>{measurement.peakAmplitude.toFixed(3)}</value>
            </div>
          </div>
        </div>

        <div className="analysis-section">
          <h2>Harmonic Analysis</h2>
          <div className="parameter-grid">
            <div className="parameter-item">
              <label>H2/H1 Ratio</label>
              <value>{measurement.harmonicRatioH2.toFixed(3)}</value>
            </div>
            <div className="parameter-item">
              <label>H3/H1 Ratio</label>
              <value>{measurement.harmonicRatioH3.toFixed(3)}</value>
            </div>
            <div className="parameter-item">
              <label>THD</label>
              <value>{measurement.thd.toFixed(1)}%</value>
            </div>
            <div className="parameter-item">
              <label>Noise Floor</label>
              <value>{measurement.noiseFloor.toFixed(3)}</value>
            </div>
          </div>
        </div>

        <div className="analysis-section">
          <h2>Decay Analysis</h2>
          <div className="parameter-grid">
            <div className="parameter-item">
              <label>Decay Rate</label>
              <value>{measurement.decayRate.toFixed(4)}</value>
            </div>
            <div className="parameter-item">
              <label>Q-Factor</label>
              <value>{measurement.qFactor.toFixed(2)}</value>
            </div>
          </div>
        </div>

        <div className="analysis-actions">
          <button className="btn btn-primary">📊 Generate Report</button>
          <button className="btn btn-secondary">💾 Save CSV</button>
        </div>
      </div>
    </div>
  );
}
