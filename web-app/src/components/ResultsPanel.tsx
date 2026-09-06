import React from 'react'
import type { MeasurementResult } from '../types'

interface ResultsPanelProps {
  result: MeasurementResult
}

export const ResultsPanel: React.FC<ResultsPanelProps> = ({ result }) => {
  const [activeTab, setActiveTab] = React.useState<'summary' | 'waveform' | 'spectrum'>('summary')
  const canvasRef = React.useRef<HTMLCanvasElement>(null)

  React.useEffect(() => {
    if (activeTab === 'waveform' && canvasRef.current && result.waveformData) {
      drawWaveform()
    } else if (activeTab === 'spectrum' && canvasRef.current && result.spectrumData) {
      drawSpectrum()
    }
  }, [activeTab, result])

  const drawWaveform = () => {
    if (!canvasRef.current) return
    const canvas = canvasRef.current
    const ctx = canvas.getContext('2d')
    if (!ctx) return

    canvas.width = canvas.offsetWidth * window.devicePixelRatio
    canvas.height = canvas.offsetHeight * window.devicePixelRatio
    ctx.scale(window.devicePixelRatio, window.devicePixelRatio)

    const width = canvas.offsetWidth
    const height = canvas.offsetHeight
    const data = result.waveformData

    ctx.fillStyle = '#ffffff'
    ctx.fillRect(0, 0, width, height)

    ctx.strokeStyle = '#2196F3'
    ctx.lineWidth = 2
    ctx.beginPath()

    const maxValue = Math.max(...data.map(Math.abs), 0.1)
    const centerY = height / 2

    for (let i = 0; i < data.length; i++) {
      const x = (i / data.length) * width
      const y = centerY - (data[i] / maxValue) * (height / 2.5)

      if (i === 0) {
        ctx.moveTo(x, y)
      } else {
        ctx.lineTo(x, y)
      }
    }
    ctx.stroke()
  }

  const drawSpectrum = () => {
    if (!canvasRef.current) return
    const canvas = canvasRef.current
    const ctx = canvas.getContext('2d')
    if (!ctx) return

    canvas.width = canvas.offsetWidth * window.devicePixelRatio
    canvas.height = canvas.offsetHeight * window.devicePixelRatio
    ctx.scale(window.devicePixelRatio, window.devicePixelRatio)

    const width = canvas.offsetWidth
    const height = canvas.offsetHeight
    const spectrum = result.spectrumData

    ctx.fillStyle = '#ffffff'
    ctx.fillRect(0, 0, width, height)

    const barWidth = width / spectrum.length
    const maxMagnitude = Math.max(...spectrum.map(Math.abs), 0.1)

    ctx.fillStyle = '#FF9800'
    for (let i = 0; i < spectrum.length; i++) {
      const magnitude = Math.abs(spectrum[i]) / maxMagnitude
      const barHeight = magnitude * height

      ctx.fillRect(
        i * barWidth,
        height - barHeight,
        barWidth - 1,
        barHeight
      )
    }
  }

  const handleExportJSON = () => {
    const dataStr = JSON.stringify(result, null, 2)
    const dataBlob = new Blob([dataStr], { type: 'application/json' })
    const url = URL.createObjectURL(dataBlob)
    const link = document.createElement('a')
    link.href = url
    link.download = `measurement-${Date.now()}.json`
    link.click()
    URL.revokeObjectURL(url)

    // Save to local storage
    saveToHistory(result)
  }

  const handleExportCSV = () => {
    const csv = [
      ['Metric', 'Value'],
      ['Timestamp', result.timestamp],
      ['Duration (s)', result.duration],
      ['RMS', result.rms],
      ['Peak', result.peak],
      ['Mean', result.mean],
      ['Frequency (Hz)', result.frequency],
      ['Quality', result.quality],
      ['Sample Count', result.sampleCount]
    ]
      .map(row => row.join(','))
      .join('\n')

    const dataBlob = new Blob([csv], { type: 'text/csv' })
    const url = URL.createObjectURL(dataBlob)
    const link = document.createElement('a')
    link.href = url
    link.download = `measurement-${Date.now()}.csv`
    link.click()
    URL.revokeObjectURL(url)

    saveToHistory(result)
  }

  const saveToHistory = (measurement: MeasurementResult) => {
    try {
      const history = JSON.parse(localStorage.getItem('measurements') || '[]')
      history.push({
        ...measurement,
        id: Date.now()
      })
      localStorage.setItem('measurements', JSON.stringify(history.slice(-50))) // Keep last 50
    } catch (err) {
      console.error('Failed to save to history:', err)
    }
  }

  return (
    <div className="results-panel">
      <div className="results-header">
        <h2>Measurement Results</h2>
        <div className="quality-score">
          <span className="quality-label">Quality:</span>
          <span className={`quality-value quality-${result.quality}`}>
            {result.quality.toUpperCase()}
          </span>
        </div>
      </div>

      <div className="tabs">
        <button
          className={`tab ${activeTab === 'summary' ? 'active' : ''}`}
          onClick={() => setActiveTab('summary')}
        >
          Summary
        </button>
        <button
          className={`tab ${activeTab === 'waveform' ? 'active' : ''}`}
          onClick={() => setActiveTab('waveform')}
        >
          Waveform
        </button>
        <button
          className={`tab ${activeTab === 'spectrum' ? 'active' : ''}`}
          onClick={() => setActiveTab('spectrum')}
        >
          Spectrum
        </button>
      </div>

      <div className="tab-content">
        {activeTab === 'summary' && (
          <div className="summary-tab">
            <div className="metrics-grid">
              <div className="metric">
                <label>RMS Level</label>
                <span className="value">{result.rms.toFixed(4)}</span>
              </div>
              <div className="metric">
                <label>Peak Amplitude</label>
                <span className="value">{result.peak.toFixed(4)}</span>
              </div>
              <div className="metric">
                <label>Peak Frequency</label>
                <span className="value">{result.frequency} Hz</span>
              </div>
              <div className="metric">
                <label>Duration</label>
                <span className="value">{result.duration} s</span>
              </div>
              <div className="metric">
                <label>Samples</label>
                <span className="value">{result.sampleCount.toLocaleString()}</span>
              </div>
              <div className="metric">
                <label>Mean Value</label>
                <span className="value">{result.mean.toFixed(4)}</span>
              </div>
            </div>
            <div className="timestamp">
              <small>Measured: {new Date(result.timestamp).toLocaleString()}</small>
            </div>
          </div>
        )}

        {activeTab === 'waveform' && (
          <div className="waveform-tab">
            <canvas ref={canvasRef} className="result-canvas"></canvas>
          </div>
        )}

        {activeTab === 'spectrum' && (
          <div className="spectrum-tab">
            <canvas ref={canvasRef} className="result-canvas"></canvas>
          </div>
        )}
      </div>

      <div className="export-section">
        <button onClick={handleExportJSON} className="btn btn-export">
          📥 Export as JSON
        </button>
        <button onClick={handleExportCSV} className="btn btn-export">
          📥 Export as CSV
        </button>
      </div>
    </div>
  )
}
