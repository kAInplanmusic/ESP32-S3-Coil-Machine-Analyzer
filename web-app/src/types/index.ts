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
  timestamp: string
  duration: number
  rms: number
  peak: number
  mean: number
  frequency: number
  quality: 'poor' | 'fair' | 'good'
  sampleCount: number
  waveformData: number[]
  spectrumData: number[]
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
