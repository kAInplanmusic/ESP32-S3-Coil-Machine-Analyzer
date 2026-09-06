import React from 'react';
import '../styles/MeasurementPage.css';
import QualityIndicator from '../components/QualityIndicator';
import WaveformDisplay from '../components/WaveformDisplay';

export default function MeasurementDesktop({ darkMode }) {
  const [isRecording, setIsRecording] = React.useState(false);
  const [progress, setProgress] = React.useState(0);
  const [results, setResults] = React.useState(null);
  const [waveformData, setWaveformData] = React.useState([]);

  const handleStartMeasurement = async () => {
    setIsRecording(true);
    setProgress(0);
    setResults(null);
    setWaveformData([]);

    // Generate mock waveform data
    const audioData = new Int16Array(44100 * 2); // 2 seconds
    for (let i = 0; i < audioData.length; i++) {
      const freq = 50 * Math.sin(i / 1000) + 100;
      audioData[i] = Math.sin((i / 44100) * freq * 2 * Math.PI) * 30000;
    }
    setWaveformData(Array.from(audioData));

    // Simulate progress
    const interval = setInterval(() => {
      setProgress((prev) => {
        if (prev >= 100) {
          clearInterval(interval);
          return 100;
        }
        return prev + Math.random() * 5;
      });
    }, 100);

    // Simulate completion
    setTimeout(() => {
      setIsRecording(false);
      setProgress(100);
      setResults({
        cps: 85.3,
        qualityScore: 92,
        durationMs: 15000,
        harmonicRatioH2: 0.45,
        harmonicRatioH3: 0.12,
        thd: 48.5,
        decayRate: 0.0234,
        qFactor: 134.5,
      });
    }, 5000);
  };

  return (
    <div className="measurement-page">
      <div className="measurement-header">
        <h1>🎯 Live Measurement</h1>
        <p>Capture and analyze coil machine impacts</p>
      </div>

      <div className="measurement-content">
        <div className="measurement-controls">
          <button
            className={`btn btn-primary ${isRecording ? 'disabled' : ''}`}
            onClick={handleStartMeasurement}
            disabled={isRecording}
          >
            {isRecording ? '🔴 Recording...' : '▶️ Start Measurement'}
          </button>

          {isRecording && (
            <div className="progress-section">
              <div className="progress-bar">
                <div
                  className="progress-fill"
                  style={{ width: `${progress}%` }}
                ></div>
              </div>
              <span className="progress-text">{Math.round(progress)}%</span>
            </div>
          )}
        </div>

        {waveformData.length > 0 && (
          <div className="waveform-section">
            <h3>Waveform</h3>
            <WaveformDisplay data={waveformData} darkMode={darkMode} />
          </div>
        )}

        {results && (
          <div className="results-section">
            <h3>Measurement Results</h3>
            <div className="results-grid">
              <div className="result-card">
                <div className="result-header">
                  <span className="result-label">Quality Score</span>
                  <QualityIndicator score={results.qualityScore} />
                </div>
              </div>

              <div className="result-card">
                <h4>Frequency</h4>
                <p className="result-value">{results.cps.toFixed(1)} CPS</p>
              </div>

              <div className="result-card">
                <h4>Duration</h4>
                <p className="result-value">{results.durationMs}ms</p>
              </div>

              <div className="result-card">
                <h4>THD</h4>
                <p className="result-value">{results.thd.toFixed(1)}%</p>
              </div>

              <div className="result-card">
                <h4>H2/H1</h4>
                <p className="result-value">
                  {results.harmonicRatioH2.toFixed(3)}
                </p>
              </div>

              <div className="result-card">
                <h4>H3/H1</h4>
                <p className="result-value">
                  {results.harmonicRatioH3.toFixed(3)}
                </p>
              </div>

              <div className="result-card">
                <h4>Decay Rate</h4>
                <p className="result-value">
                  {results.decayRate.toFixed(4)}
                </p>
              </div>

              <div className="result-card">
                <h4>Q-Factor</h4>
                <p className="result-value">{results.qFactor.toFixed(2)}</p>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  );
}
