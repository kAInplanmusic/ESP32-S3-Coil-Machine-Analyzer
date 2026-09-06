// Signal processing service for audio analysis
import type { AudioConfig, Harmonic, MeasurementResult } from '../types'

export class SignalProcessor {
  private config: AudioConfig
  private audioContext: AudioContext | null = null
  private analyser: AnalyserNode | null = null
  private scriptProcessor: ScriptProcessorNode | null = null

  constructor(config: Partial<AudioConfig> = {}) {
    this.config = {
      sampleRate: 44100,
      fftSize: 2048,
      bufferLength: 4096,
      threshold: 0.01,
      ...config,
    }
  }

  /**
   * Initialize Web Audio API context
   */
  async initialize(): Promise<void> {
    if (this.audioContext) return

    this.audioContext = new (window.AudioContext || (window as any).webkitAudioContext)({
      sampleRate: this.config.sampleRate,
    })

    this.analyser = this.audioContext.createAnalyser()
    this.analyser.fftSize = this.config.fftSize
  }

  /**
   * Simple FFT implementation using Cooley-Tukey algorithm
   */
  fft(input: number[]): { real: number[]; imag: number[] } {
    const n = input.length
    if (n <= 1) return { real: input, imag: new Array(n).fill(0) }
    if (n % 2 !== 0) {
      // Pad with zero if odd
      return this.fft([...input, 0])
    }

    const real = new Array(n)
    const imag = new Array(n)
    const even = input.filter((_, i) => i % 2 === 0)
    const odd = input.filter((_, i) => i % 2 === 1)

    const evenFFT = this.fft(even)
    const oddFFT = this.fft(odd)

    for (let k = 0; k < n / 2; k++) {
      const angle = (-2 * Math.PI * k) / n
      const wr = Math.cos(angle)
      const wi = Math.sin(angle)

      const tr = wr * oddFFT.real[k] - wi * oddFFT.imag[k]
      const ti = wr * oddFFT.imag[k] + wi * oddFFT.real[k]

      real[k] = evenFFT.real[k] + tr
      imag[k] = evenFFT.imag[k] + ti

      real[k + n / 2] = evenFFT.real[k] - tr
      imag[k + n / 2] = evenFFT.imag[k] - ti
    }

    return { real, imag }
  }

  /**
   * Calculate magnitude spectrum
   */
  getMagnitudeSpectrum(real: number[], imag: number[]): number[] {
    return real.map((r, i) => Math.sqrt(r * r + imag[i] * imag[i]))
  }

  /**
   * Find fundamental frequency
   */
  findFundamentalFrequency(magnitudes: number[]): { frequency: number; magnitude: number } {
    let maxMag = 0
    let maxIdx = 0

    for (let i = 1; i < magnitudes.length; i++) {
      if (magnitudes[i] > maxMag) {
        maxMag = magnitudes[i]
        maxIdx = i
      }
    }

    const frequency = (maxIdx * this.config.sampleRate) / magnitudes.length
    return { frequency, magnitude: maxMag }
  }

  /**
   * Detect harmonics
   */
  detectHarmonics(fundamental: number, magnitudes: number[], threshold: number = 0.1): Harmonic[] {
    const harmonics: Harmonic[] = []
    const maxMag = Math.max(...magnitudes)
    const freqResolution = this.config.sampleRate / magnitudes.length

    for (let order = 2; order <= 10; order++) {
      const expectedFreq = fundamental * order
      const expectedBin = Math.round(expectedFreq / freqResolution)

      if (expectedBin < magnitudes.length) {
        const magnitude = magnitudes[expectedBin]
        if (magnitude > maxMag * threshold) {
          harmonics.push({
            order,
            frequency: expectedBin * freqResolution,
            magnitude: magnitude / maxMag,
            phase: 0,
          })
        }
      }
    }

    return harmonics
  }

  /**
   * Calculate RMS value
   */
  calculateRMS(waveform: number[]): number {
    const sum = waveform.reduce((acc, val) => acc + val * val, 0)
    return Math.sqrt(sum / waveform.length)
  }

  /**
   * Calculate peak amplitude
   */
  calculatePeakAmplitude(waveform: number[]): number {
    return Math.max(...waveform.map(Math.abs))
  }

  /**
   * Calculate decay rate
   */
  calculateDecayRate(waveform: number[], windowSize: number = 100): number {
    if (waveform.length < windowSize * 2) return 0

    const firstWindow = waveform.slice(0, windowSize)
    const lastWindow = waveform.slice(-windowSize)

    const firstRMS = this.calculateRMS(firstWindow)
    const lastRMS = this.calculateRMS(lastWindow)

    if (firstRMS === 0) return 0
    return 1 - lastRMS / firstRMS
  }

  /**
   * Calculate quality score (0-100)
   */
  calculateQualityScore(
    peakAmplitude: number,
    rmsValue: number,
    harmonics: Harmonic[]
  ): number {
    let score = 50

    // Amplitude factor (peaks should be significant)
    if (peakAmplitude > 0.5) score += 25
    else if (peakAmplitude > 0.2) score += 15

    // RMS factor (good signal strength)
    if (rmsValue > 0.1) score += 15
    else if (rmsValue > 0.05) score += 8

    // Harmonic purity (fewer strong harmonics = cleaner signal)
    const strongHarmonics = harmonics.filter((h) => h.magnitude > 0.3).length
    if (strongHarmonics <= 2) score += 10

    return Math.min(100, score)
  }

  /**
   * Process audio data and return measurement result
   */
  processAudioData(waveform: number[], timestamp: number): MeasurementResult {
    const peakAmplitude = this.calculatePeakAmplitude(waveform)
    const rmsValue = this.calculateRMS(waveform)

    // Perform FFT
    const fftResult = this.fft(waveform)
    const magnitudes = this.getMagnitudeSpectrum(fftResult.real, fftResult.imag)

    // Find fundamental frequency
    const fundamental = this.findFundamentalFrequency(magnitudes)

    // Detect harmonics
    const harmonics = this.detectHarmonics(fundamental.frequency, magnitudes)

    // Calculate decay rate
    const decayRate = this.calculateDecayRate(waveform)

    // Calculate quality score
    const qualityScore = this.calculateQualityScore(peakAmplitude, rmsValue, harmonics)

    return {
      id: `measurement_${Date.now()}`,
      timestamp,
      duration: waveform.length / this.config.sampleRate,
      peakAmplitude,
      rmsValue,
      fundamentalFrequency: fundamental.frequency,
      harmonics,
      decayRate,
      qualityScore,
      waveformData: waveform,
      frequencyData: magnitudes.slice(0, magnitudes.length / 2),
    }
  }

  getAudioContext(): AudioContext | null {
    return this.audioContext
  }

  getAnalyser(): AnalyserNode | null {
    return this.analyser
  }
}

export default SignalProcessor
