// Type definitions for the Coil Analyzer Web App

export interface AudioConfig {
  sampleRate: number
  fftSize: number
  bufferLength: number
  threshold: number
}

export interface MeasurementPoint {
  timestamp: number
  value: number
  frequency?: number
}

export interface MeasurementResult {
  id: string
  timestamp: number
  duration: number
  peakAmplitude: number
  rmsValue: number
  fundamentalFrequency: number
  harmonics: Harmonic[]
  decayRate: number
  qualityScore: number
  waveformData: number[]
  frequencyData: number[]
}

export interface Harmonic {
  order: number
  frequency: number
  magnitude: number
  phase: number
}

export interface VisualizationData {
  waveform: {
    x: number[]
    y: number[]
  }
  frequency: {
    x: number[]
    y: number[]
  }
  harmonics: {
    labels: string[]
    data: number[]
  }
}
