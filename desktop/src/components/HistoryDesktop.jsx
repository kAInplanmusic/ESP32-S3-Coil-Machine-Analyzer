import React from 'react';
import '../styles/HistoryDesktop.css';

export default function HistoryDesktop() {
  const [measurements, setMeasurements] = React.useState([
    {
      id: 1,
      timestamp: new Date(Date.now() - 3600000),
      cps: 85.3,
      qualityScore: 92,
      durationMs: 15000,
    },
    {
      id: 2,
      timestamp: new Date(Date.now() - 7200000),
      cps: 82.1,
      qualityScore: 88,
      durationMs: 14500,
    },
    {
      id: 3,
      timestamp: new Date(Date.now() - 10800000),
      cps: 87.6,
      qualityScore: 94,
      durationMs: 15200,
    },
  ]);

  const handleExport = () => {
    const csv = [
      ['Timestamp', 'CPS', 'Quality Score', 'Duration (ms)'],
      ...measurements.map((m) => [
        m.timestamp.toLocaleString(),
        m.cps,
        m.qualityScore,
        m.durationMs,
      ]),
    ]
      .map((row) => row.join(','))
      .join('\n');

    const blob = new Blob([csv], { type: 'text/csv' });
    const url = window.URL.createObjectURL(blob);
    const a = document.createElement('a');
    a.href = url;
    a.download = 'measurements.csv';
    a.click();
  };

  return (
    <div className="history-page">
      <div className="history-header">
        <h1>📊 Measurement History</h1>
        <button className="btn btn-secondary" onClick={handleExport}>
          💾 Export CSV
        </button>
      </div>

      <div className="history-content">
        {measurements.length === 0 ? (
          <div className="empty-state">
            <p>No measurements yet</p>
            <p>Start a measurement to see results here</p>
          </div>
        ) : (
          <table className="history-table">
            <thead>
              <tr>
                <th>Timestamp</th>
                <th>CPS</th>
                <th>Quality Score</th>
                <th>Duration</th>
              </tr>
            </thead>
            <tbody>
              {measurements.map((m) => (
                <tr key={m.id}>
                  <td>{m.timestamp.toLocaleString()}</td>
                  <td>{m.cps.toFixed(1)}</td>
                  <td>
                    <span
                      className={`quality-badge ${
                        m.qualityScore > 80 ? 'good' : 'fair'
                      }`}
                    >
                      {m.qualityScore}%
                    </span>
                  </td>
                  <td>{m.durationMs}ms</td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </div>
  );
}
