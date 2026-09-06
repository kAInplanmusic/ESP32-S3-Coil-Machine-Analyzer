import React from 'react'
import { MeasurementStorage } from '../utils/storage'
import type { MeasurementResult } from '../types'
import '../styles/MeasurementHistory.css'

export const MeasurementHistory: React.FC = () => {
  const [measurements, setMeasurements] = React.useState<MeasurementResult[]>([])

  React.useEffect(() => {
    const allMeasurements = MeasurementStorage.getAllMeasurements()
    setMeasurements(allMeasurements)
  }, [])

  const handleDelete = (id: string) => {
    MeasurementStorage.deleteMeasurement(id)
    setMeasurements(measurements.filter(m => m.id !== id))
  }

  const handleExportAll = () => {
    const csv = MeasurementStorage.exportAsCSV(measurements)
    MeasurementStorage.downloadFile(csv, 'all_measurements.csv', 'text/csv')
  }

  const handleClearAll = () => {
    if (confirm('Are you sure you want to delete all measurements?')) {
      MeasurementStorage.clearAll()
      setMeasurements([])
    }
  }

  if (measurements.length === 0) {
    return (
      <div className="measurement-history">
        <h2>📋 Measurement History</h2>
        <p className="empty-message">No measurements yet. Start by analyzing a coil machine sound!</p>
      </div>
    )
  }

  return (
    <div className="measurement-history">
      <h2>📋 Measurement History ({measurements.length})</h2>

      <div className="history-controls">
        <button className="btn btn-primary" onClick={handleExportAll}>
          📥 Export All as CSV
        </button>
        <button className="btn btn-danger" onClick={handleClearAll}>
          🗑️ Clear History
        </button>
      </div>

      <div className="measurements-table-wrapper">
        <table className="measurements-table">
          <thead>
            <tr>
              <th>Date/Time</th>
              <th>Duration (s)</th>
              <th>Fundamental (Hz)</th>
              <th>Quality</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {measurements.map(m => (
              <tr key={m.id}>
                <td>{new Date(m.timestamp).toLocaleString()}</td>
                <td>{m.duration.toFixed(2)}</td>
                <td>{m.fundamentalFrequency.toFixed(1)}</td>
                <td>
                  <div className="quality-badge">
                    <span className={`quality-${m.qualityScore >= 70 ? 'high' : m.qualityScore >= 50 ? 'medium' : 'low'}`}>
                      {m.qualityScore.toFixed(0)}%
                    </span>
                  </div>
                </td>
                <td>
                  <button
                    className="btn-small btn-danger"
                    onClick={() => handleDelete(m.id)}
                    title="Delete this measurement"
                  >
                    ❌
                  </button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  )
}
