import React, { useState, useEffect } from 'react';
import { useNavigate } from 'react-router-dom';
import { getMeasurements, deleteMeasurement } from '../services/api.js';
import '../styles/HistoryPage.css';

function HistoryPage() {
  const [measurements, setMeasurements] = useState([]);
  const [loading, setLoading] = useState(true);
  const navigate = useNavigate();

  useEffect(() => {
    loadMeasurements();
  }, []);

  const loadMeasurements = async () => {
    try {
      const data = await getMeasurements();
      setMeasurements(data);
    } catch (error) {
      console.error('Failed to load measurements:', error);
    } finally {
      setLoading(false);
    }
  };

  const handleDelete = async (id) => {
    if (confirm('Delete this measurement?')) {
      try {
        await deleteMeasurement(id);
        setMeasurements(measurements.filter(m => m.id !== id));
      } catch (error) {
        console.error('Failed to delete measurement:', error);
      }
    }
  };

  if (loading) {
    return <div className="history-page"><p>Loading measurements...</p></div>;
  }

  return (
    <div className="history-page">
      <h2>Measurement History</h2>
      
      {measurements.length === 0 ? (
        <p className="empty-message">No measurements yet. Start a new measurement!</p>
      ) : (
        <table className="measurements-table">
          <thead>
            <tr>
              <th>Date</th>
              <th>CPS</th>
              <th>Quality</th>
              <th>Duration</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {measurements.map(m => (
              <tr key={m.id}>
                <td>{new Date(m.created_at).toLocaleString()}</td>
                <td>{m.cps?.toFixed(1)}</td>
                <td>
                  <span className={`quality-badge quality-${m.quality_score > 80 ? 'good' : 'fair'}`}>
                    {m.quality_score}%
                  </span>
                </td>
                <td>{m.duration_ms}ms</td>
                <td className="actions">
                  <button onClick={() => navigate(`/analysis/${m.id}`)} className="btn-small">View</button>
                  <button onClick={() => handleDelete(m.id)} className="btn-small btn-danger">Delete</button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      )}
    </div>
  );
}

export default HistoryPage;
