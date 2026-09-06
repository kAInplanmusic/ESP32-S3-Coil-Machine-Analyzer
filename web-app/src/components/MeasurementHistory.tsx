import React from 'react'
import type { MeasurementResult } from '../types'

interface StoredMeasurement extends MeasurementResult {
  id: number
}

export const MeasurementHistory: React.FC = () => {
  const [measurements, setMeasurements] = React.useState<StoredMeasurement[]>([])
  const [selectedId, setSelectedId] = React.useState<number | null>(null)

  React.useEffect(() => {
    loadMeasurements()
  }, [])

  const loadMeasurements = () => {
    try {
      const stored = localStorage.getItem('measurements')
      const measurements = stored ? JSON.parse(stored) : []
      setMeasurements(measurements.sort((a: StoredMeasurement, b: StoredMeasurement) => 
        new Date(b.timestamp).getTime() - new Date(a.timestamp).getTime()
      ))
    } catch (err) {
      console.error('Failed to load measurements:', err)
    }
  }

  const handleDelete = (id: number) => {
    if (confirm('Delete this measurement?')) {
      const updated = measurements.filter(m => m.id !== id)
      localStorage.setItem('measurements', JSON.stringify(updated))
      setMeasurements(updated)
      setSelectedId(null)
    }
  }

  const handleExportAll = () => {
    const csv = [
      ['Timestamp', 'Duration (s)', 'RMS', 'Peak', 'Frequency (Hz)', 'Quality', 'Samples'].join(','),
      ...measurements.map(m => 
        [m.timestamp, m.duration, m.rms.toFixed(4), m.peak.toFixed(4), m.frequency, m.quality, m.sampleCount].join(',')
      )
    ].join('\n')

    const blob = new Blob([csv], { type: 'text/csv' })
    const url = URL.createObjectURL(blob)
    const link = document.createElement('a')
    link.href = url
    link.download = `all-measurements-${Date.now()}.csv`
    link.click()
    URL.revokeObjectURL(url)
  }

  const handleClearAll = () => {
    if (confirm('Clear all measurements? This cannot be undone.')) {
      localStorage.setItem('measurements', JSON.stringify([]))
      setMeasurements([])
      setSelectedId(null)
    }
  }

  const handleExportSelected = (measurement: StoredMeasurement) => {
    const json = JSON.stringify(measurement, null, 2)
    const blob = new Blob([json], { type: 'application/json' })
    const url = URL.createObjectURL(blob)
    const link = document.createElement('a')
    link.href = url
    link.download = `measurement-${measurement.id}.json`
    link.click()
    URL.revokeObjectURL(url)
  }

  if (measurements.length === 0) {
    return (
      <div className="measurement-history">
        <h2>📋 Measurement History</h2>
        <p className="empty-message">No measurements yet. Start by analyzing a sound!</p>
      </div>
    )
  }

  const selected = selectedId ? measurements.find(m => m.id === selectedId) : null

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

      <div className="history-container">
        <div className="measurements-list">
          <table className="measurements-table">
            <thead>
              <tr>
                <th>Date/Time</th>
                <th>Duration</th>
                <th>Peak (Hz)</th>
                <th>Quality</th>
                <th>Actions</th>
              </tr>
            </thead>
            <tbody>
              {measurements.map(m => (
                <tr 
                  key={m.id} 
                  className={selectedId === m.id ? 'selected' : ''}
                  onClick={() => setSelectedId(m.id)}
                >
                  <td>{new Date(m.timestamp).toLocaleString()}</td>
                  <td>{m.duration.toFixed(1)}s</td>
                  <td>{m.frequency} Hz</td>
                  <td>
                    <span className={`quality-badge quality-${m.quality}`}>
                      {m.quality.toUpperCase()}
                    </span>
                  </td>
                  <td>
                    <div className="action-buttons">
                      <button
                        className="btn-small"
                        onClick={(e) => {
                          e.stopPropagation()
                          handleExportSelected(m)
                        }}
                        title="Export"
                      >
                        📥
                      </button>
                      <button
                        className="btn-small btn-danger"
                        onClick={(e) => {
                          e.stopPropagation()
                          handleDelete(m.id)
                        }}
                        title="Delete"
                      >
                        ❌
                      </button>
                    </div>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        {selected && (
          <div className="measurement-detail">
            <h3>Details</h3>
            <div className="detail-grid">
              <div className="detail-item">
                <label>Timestamp</label>
                <p>{new Date(selected.timestamp).toLocaleString()}</p>
              </div>
              <div className="detail-item">
                <label>Duration</label>
                <p>{selected.duration} seconds</p>
              </div>
              <div className="detail-item">
                <label>RMS Level</label>
                <p>{selected.rms.toFixed(4)}</p>
              </div>
              <div className="detail-item">
                <label>Peak Amplitude</label>
                <p>{selected.peak.toFixed(4)}</p>
              </div>
              <div className="detail-item">
                <label>Peak Frequency</label>
                <p>{selected.frequency} Hz</p>
              </div>
              <div className="detail-item">
                <label>Quality</label>
                <p>{selected.quality.toUpperCase()}</p>
              </div>
              <div className="detail-item">
                <label>Mean Value</label>
                <p>{selected.mean.toFixed(4)}</p>
              </div>
              <div className="detail-item">
                <label>Sample Count</label>
                <p>{selected.sampleCount.toLocaleString()}</p>
              </div>
            </div>
          </div>
        )}
      </div>
    </div>
  )
}
