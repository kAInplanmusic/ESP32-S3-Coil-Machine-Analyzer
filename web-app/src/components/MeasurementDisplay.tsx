import React from 'react'
import type { MeasurementResult } from '../types'

interface MeasurementDisplayProps {
  measurement: MeasurementResult | null
}

export const MeasurementDisplay: React.FC<MeasurementDisplayProps> = ({ measurement }) => {
  const canvasRef = React.useRef<HTMLCanvasElement>(null)
  const spectrumCanvasRef = React.useRef<HTMLCanvasElement>(null)

  React.useEffect(() => {
    if (!canvasRef.current || !measurement) return

    const canvas = canvasRef.current
    const ctx = canvas.getContext('2d')
    if (!ctx) return

    // Set canvas size
    canvas.width = canvas.offsetWidth * window.devicePixelRatio
    canvas.height = canvas.offsetHeight * window.devicePixelRatio
    ctx.scale(window.devicePixelRatio, window.devicePixelRatio)

    const width = canvas.offsetWidth
    const height = canvas.offsetHeight

    // Draw waveform
    ctx.fillStyle = '#f0f0f0'
    ctx.fillRect(0, 0, width, height)

    // Draw grid
    ctx.strokeStyle = '#e0e0e0'
    ctx.lineWidth = 1
    for (let i = 0; i <= 10; i++) {
      const y = (height / 10) * i
      ctx.beginPath()
      ctx.moveTo(0, y)
      ctx.lineTo(width, y)
      ctx.stroke()
    }

    // Draw waveform
    const data = measurement.waveformData || []
    if (data.length > 0) {
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
  }, [measurement])

  React.useEffect(() => {
    if (!spectrumCanvasRef.current || !measurement) return

    const canvas = spectrumCanvasRef.current
    const ctx = canvas.getContext('2d')
    if (!ctx) return

    canvas.width = canvas.offsetWidth * window.devicePixelRatio
    canvas.height = canvas.offsetHeight * window.devicePixelRatio
    ctx.scale(window.devicePixelRatio, window.devicePixelRatio)

    const width = canvas.offsetWidth
    const height = canvas.offsetHeight

    // Draw background
    ctx.fillStyle = '#f0f0f0'
    ctx.fillRect(0, 0, width, height)

    // Draw grid
    ctx.strokeStyle = '#e0e0e0'
    ctx.lineWidth = 1
    for (let i = 0; i <= 10; i++) {
      const y = (height / 10) * i
      ctx.beginPath()
      ctx.moveTo(0, y)
      ctx.lineTo(width, y)
      ctx.stroke()
    }

    // Draw spectrum bars
    const spectrum = measurement.spectrumData || []
    if (spectrum.length > 0) {
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
  }, [measurement])

  return (
    <div className="measurement-display">
      <div className="stats-section">
        <div className="stat-card">
          <label>RMS Level</label>
          <span className="stat-value">{measurement?.rms.toFixed(4) || '--'}</span>
        </div>
        <div className="stat-card">
          <label>Peak Amplitude</label>
          <span className="stat-value">{measurement?.peak.toFixed(4) || '--'}</span>
        </div>
        <div className="stat-card">
          <label>Peak Frequency</label>
          <span className="stat-value">{measurement?.frequency || '--'} Hz</span>
        </div>
        <div className="stat-card">
          <label>Quality</label>
          <span className="stat-value">{measurement?.quality?.toUpperCase() || '--'}</span>
        </div>
      </div>

      <div className="waveform-section">
        <h3>Live Waveform</h3>
        <canvas ref={canvasRef} className="measurement-canvas"></canvas>
      </div>

      <div className="spectrum-section">
        <h3>Frequency Spectrum</h3>
        <canvas ref={spectrumCanvasRef} className="measurement-canvas"></canvas>
      </div>

      <div className="progress-section">
        <h3>Recording Progress</h3>
        <p>Duration: {measurement?.duration || 0}s | Samples: {measurement?.sampleCount || 0}</p>
      </div>
    </div>
  )
}
