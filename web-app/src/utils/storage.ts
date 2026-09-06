// Storage utility for measurements
import type { MeasurementResult } from '../types'

const STORAGE_KEY = 'coil-analyzer-measurements'
const MAX_STORED_MEASUREMENTS = 50

export class MeasurementStorage {
  static saveMeasurement(result: MeasurementResult): void {
    try {
      const measurements = this.getAllMeasurements()
      measurements.unshift(result)

      // Keep only recent measurements
      if (measurements.length > MAX_STORED_MEASUREMENTS) {
        measurements.splice(MAX_STORED_MEASUREMENTS)
      }

      localStorage.setItem(STORAGE_KEY, JSON.stringify(measurements))
    } catch (error) {
      console.error('Failed to save measurement:', error)
    }
  }

  static getAllMeasurements(): MeasurementResult[] {
    try {
      const data = localStorage.getItem(STORAGE_KEY)
      return data ? JSON.parse(data) : []
    } catch (error) {
      console.error('Failed to load measurements:', error)
      return []
    }
  }

  static deleteMeasurement(id: string): void {
    try {
      const measurements = this.getAllMeasurements()
      const filtered = measurements.filter(m => m.id !== id)
      localStorage.setItem(STORAGE_KEY, JSON.stringify(filtered))
    } catch (error) {
      console.error('Failed to delete measurement:', error)
    }
  }

  static clearAll(): void {
    try {
      localStorage.removeItem(STORAGE_KEY)
    } catch (error) {
      console.error('Failed to clear measurements:', error)
    }
  }

  static exportAsJSON(measurement: MeasurementResult): string {
    return JSON.stringify(measurement, null, 2)
  }

  static exportAsCSV(measurements: MeasurementResult[]): string {
    if (measurements.length === 0) return ''

    const headers = [
      'ID',
      'Timestamp',
      'Duration (s)',
      'Peak Amplitude',
      'RMS Value',
      'Fundamental Frequency (Hz)',
      'Decay Rate',
      'Quality Score',
    ]

    const rows = measurements.map(m => [
      m.id,
      new Date(m.timestamp).toISOString(),
      m.duration.toFixed(2),
      m.peakAmplitude.toFixed(4),
      m.rmsValue.toFixed(4),
      m.fundamentalFrequency.toFixed(1),
      m.decayRate.toFixed(3),
      m.qualityScore.toFixed(0),
    ])

    const csvContent = [
      headers.join(','),
      ...rows.map(row => row.join(',')),
    ].join('\n')

    return csvContent
  }

  static downloadFile(content: string, filename: string, mimeType: string): void {
    const blob = new Blob([content], { type: mimeType })
    const url = URL.createObjectURL(blob)
    const link = document.createElement('a')
    link.href = url
    link.download = filename
    document.body.appendChild(link)
    link.click()
    document.body.removeChild(link)
    URL.revokeObjectURL(url)
  }
}

export default MeasurementStorage
