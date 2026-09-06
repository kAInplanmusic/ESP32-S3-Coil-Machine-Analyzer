import React, { useState, useRef, useEffect } from 'react';
import WaveformDisplay from '../components/WaveformDisplay.jsx';
import QualityIndicator from '../components/QualityIndicator.jsx';
import { useAudioCapture } from '../hooks/useAudioCapture.js';
import '../styles/MeasurementPage.css';

function MeasurementPage() {
  const [isRecording, setIsRecording] = useState(false);
  const [progress, setProgress] = useState(0);
  const [results, setResults] = useState(null);
  const { startCapture, stopCapture, isSupported } = useAudioCapture();

  const handleStartMeasurement = async () => {
    if (!isSupported) {
      alert('Web Audio API not supported in your browser');
      return;
    }

    setIsRecording(true);
    setProgress(0);
    setResults(null);

    // Simulate measurement progress
    const interval = setInterval(() => {
      setProgress(prev => {
        if (prev >= 100) {
          clearInterval(interval);
          return 100;
        }
        return prev + Math.random() * 10;
      });
    }, 200);

    try {
      const audioContext = await startCapture();
      // Measurement logic will be added here
      
      setTimeout(() => {
        stopCapture();
        setIsRecording(false);
        setResults({
          cps: 85.3,
          qualityScore: 92,
          durationMs: 15000,
          harmonicRatioH2: 0.45,
          harmonicRatioH3: 0.12,
        });
      }, 5000);
    } catch (error) {
      console.error('Measurement error:', error);
      setIsRecording(false);
    }
  };

  return (
    <div className="measurement-page">
      <div className="measurement-container">
        <h2>Live Measurement</h2>

        <div className="measurement-controls">
          <button
            className="btn btn-primary"
            onClick={handleStartMeasurement}
            disabled={isRecording}
          >
            {isRecording ? '🔴 Recording...' : '▶️ Start Measurement'}
          </button>
        </div>

        {isRecording && (
          <div className="progress-section">
            <div className="progress-bar">
              <div className="progress-fill" style={{ width: `${progress}%` }}></div>
            </div>
            <p>{Math.round(progress)}%</p>
          </div>
        )}

        {results && (
          <div className="results-section">
            <h3>Measurement Results</h3>
            <div className="results-grid">
              <div className="result-card">
                <span className="label">CPS</span>
                <span className="value">{results.cps.toFixed(1)}</span>
              </div>
              <div className="result-card">
                <span className="label">Quality</span>
                <QualityIndicator score={results.qualityScore} />
              </div>
              <div className="result-card">
                <span className="label">H2/H1</span>
                <span className="value">{results.harmonicRatioH2.toFixed(2)}</span>
              </div>
              <div className="result-card">
                <span className="label">H3/H1</span>
                <span className="value">{results.harmonicRatioH3.toFixed(2)}</span>
              </div>
            </div>

            <WaveformDisplay results={results} />
          </div>
        )}
      </div>
    </div>
  );
}

export default MeasurementPage;
