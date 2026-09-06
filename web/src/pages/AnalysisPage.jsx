import React, { useState, useEffect } from 'react';
import { useParams } from 'react-router-dom';
import { getMeasurement } from '../services/api.js';
import WaveformDisplay from '../components/WaveformDisplay.jsx';
import '../styles/AnalysisPage.css';

function AnalysisPage() {
  const { id } = useParams();
  const [measurement, setMeasurement] = useState(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    loadMeasurement();
  }, [id]);

  const loadMeasurement = async () => {
    try {
      const data = await getMeasurement(id);
      setMeasurement(data);
    } catch (error) {
      console.error('Failed to load measurement:', error);
    } finally {
      setLoading(false);
    }
  };

  if (loading) {
    return <div className="analysis-page"><p>Loading analysis...</p></div>;
  }

  if (!measurement) {
    return <div className="analysis-page"><p>Measurement not found</p></div>;
  }

  return (
    <div className="analysis-page">
      <h2>Detailed Analysis</h2>
      
      <div className="analysis-container">
        <div className="analysis-header">
          <h3>{new Date(measurement.created_at).toLocaleString()}</h3>
        </div>

        <div className="analysis-grid">
          <div className="analysis-card">
            <h4>Measurement Parameters</h4>
            <dl>
              <dt>CPS (Cycles/sec):</dt>
              <dd>{measurement.cps?.toFixed(2)}</dd>
              
              <dt>Quality Score:</dt>
              <dd>{measurement.quality_score}%</dd>
              
              <dt>Duration:</dt>
              <dd>{measurement.duration_ms}ms</dd>
              
              <dt>Samples:</dt>
              <dd>{measurement.sample_count}</dd>
            </dl>
          </div>

          <div className="analysis-card">
            <h4>Harmonic Analysis</h4>
            {measurement.data && (
              <dl>
                <dt>H2/H1 Ratio:</dt>
                <dd>{measurement.data.harmonicRatioH2?.toFixed(3)}</dd>
                
                <dt>H3/H1 Ratio:</dt>
                <dd>{measurement.data.harmonicRatioH3?.toFixed(3)}</dd>
                
                <dt>THD:</dt>
                <dd>{measurement.data.thd?.toFixed(1)}%</dd>
              </dl>
            )}
          </div>

          <div className="analysis-card">
            <h4>Decay Analysis</h4>
            {measurement.data && (
              <dl>
                <dt>Decay Rate:</dt>
                <dd>{measurement.data.decayRate?.toFixed(4)}</dd>
                
                <dt>Q-Factor:</dt>
                <dd>{measurement.data.qFactor?.toFixed(2)}</dd>
              </dl>
            )}
          </div>
        </div>

        <WaveformDisplay measurement={measurement} />
      </div>
    </div>
  );
}

export default AnalysisPage;
