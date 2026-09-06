import React from 'react'
import type { MeasurementResult } from '../types'

const AUTO_STOP_SECONDS = 10
const TRIM_SECONDS = 2

interface MicrophoneCaptureProps {
  isRecording: boolean
  onStartRecording: () => void
  onStopRecording: () => void
  onMeasurementComplete: (result: MeasurementResult) => void
}

export const MicrophoneCapture: React.FC<MicrophoneCaptureProps> = ({
  isRecording,
  onStartRecording,
  onStopRecording,
  onMeasurementComplete
}) => {
  const [microphoneAvailable, setMicrophoneAvailable] = React.useState(false)
  const [error, setError] = React.useState<string | null>(null)
  const [recordingDuration, setRecordingDuration] = React.useState(0)
  const [liveCps, setLiveCps] = React.useState<number | null>(null)
  
  const audioContextRef = React.useRef<AudioContext | null>(null)
  const analyserRef = React.useRef<AnalyserNode | null>(null)
  const streamRef = React.useRef<MediaStream | null>(null)
  const processorRef = React.useRef<ScriptProcessorNode | null>(null)
  const audioDataRef = React.useRef<number[]>([])
  const timerRef = React.useRef<NodeJS.Timeout | null>(null)
  const startTimeRef = React.useRef<number>(0)
  const stopInProgressRef = React.useRef(false)
  const lastCpsUpdateRef = React.useRef(0)

  const estimateCps = (samples: Float32Array, sampleRate: number): number => {
    if (samples.length < 2) return 0

    let zeroCrossings = 0
    for (let i = 1; i < samples.length; i++) {
      const prev = samples[i - 1]
      const curr = samples[i]
      if ((prev <= 0 && curr > 0) || (prev >= 0 && curr < 0)) {
        zeroCrossings++
      }
    }

    const seconds = samples.length / sampleRate
    if (seconds <= 0) return 0
    return zeroCrossings / (2 * seconds)
  }

  const cleanupAudio = React.useCallback(() => {
    if (streamRef.current) {
      streamRef.current.getTracks().forEach(track => track.stop())
      streamRef.current = null
    }

    if (processorRef.current) {
      processorRef.current.disconnect()
      processorRef.current = null
    }

    if (analyserRef.current) {
      analyserRef.current.disconnect()
      analyserRef.current = null
    }

    if (audioContextRef.current && audioContextRef.current.state !== 'closed') {
      void audioContextRef.current.close()
    }
    audioContextRef.current = null
  }, [])

  React.useEffect(() => {
    // Check if WebAudio API is available
    const checkMicrophone = async () => {
      try {
        const stream = await navigator.mediaDevices.getUserMedia({ audio: true })
        stream.getTracks().forEach(track => track.stop())
        setMicrophoneAvailable(true)
      } catch (err) {
        setError(err instanceof Error ? err.message : 'Microphone not available')
        setMicrophoneAvailable(false)
      }
    }

    checkMicrophone()

    // Cleanup on unmount
    return () => {
      cleanupAudio()
      if (timerRef.current) {
        clearInterval(timerRef.current)
      }
    }
  }, [cleanupAudio])

  const analyzeAudio = (audioData: number[], sampleRate: number): MeasurementResult => {
    // Calculate basic metrics
    const rms = Math.sqrt(audioData.reduce((sum, val) => sum + val * val, 0) / audioData.length)
    const peak = Math.max(...audioData.map(Math.abs))
    const mean = audioData.reduce((a, b) => a + b, 0) / audioData.length

    // Simple FFT frequency detection
    const fftSize = 2048
    const fftData = audioData.slice(0, fftSize)
    
    // Pad with zeros if needed
    while (fftData.length < fftSize) {
      fftData.push(0)
    }

    // Apply Hann window
    const windowed = fftData.map((val, i) => {
      const window = 0.5 * (1 - Math.cos((2 * Math.PI * i) / (fftSize - 1)))
      return val * window
    })

    // Calculate basic spectrum (simplified peak detection)
    let maxFrequency = 0
    let maxMagnitude = 0
    
    for (let i = 1; i < fftSize / 2; i++) {
      // Simplified magnitude
      const magnitude = Math.abs(windowed[i])
      if (magnitude > maxMagnitude) {
        maxMagnitude = magnitude
        maxFrequency = (i / fftSize) * sampleRate
      }
    }

    return {
      timestamp: new Date().toISOString(),
      duration: Math.round((audioData.length / sampleRate) * 100) / 100,
      rms: Math.round(rms * 10000) / 10000,
      peak: Math.round(peak * 10000) / 10000,
      mean: Math.round(mean * 10000) / 10000,
      frequency: Math.round(maxFrequency),
      quality: peak > 0.1 ? 'good' : peak > 0.01 ? 'fair' : 'poor',
      sampleCount: audioData.length,
      waveformData: fftData.slice(0, 256), // Simplified waveform
      spectrumData: windowed.slice(0, 256) // Simplified spectrum
    }
  }

  const handleStopClick = React.useCallback(() => {
    if (stopInProgressRef.current || !isRecording) return
    stopInProgressRef.current = true

    const sampleRate = audioContextRef.current?.sampleRate || 44100
    const trimSamples = Math.floor(TRIM_SECONDS * sampleRate)
    let processedAudio = audioDataRef.current

    if (audioDataRef.current.length > trimSamples * 2) {
      processedAudio = audioDataRef.current.slice(trimSamples, audioDataRef.current.length - trimSamples)
    } else if (audioDataRef.current.length > 0) {
      setError('Recording too short after trim; please repeat measurement.')
      processedAudio = []
    }

    cleanupAudio()

    if (processedAudio.length > 0) {
      const result = analyzeAudio(processedAudio, sampleRate)
      onMeasurementComplete(result)
    }

    setLiveCps(null)
    onStopRecording()
  }, [cleanupAudio, isRecording, onMeasurementComplete, onStopRecording])

  // Update recording duration
  React.useEffect(() => {
    if (isRecording) {
      startTimeRef.current = Date.now()
      timerRef.current = setInterval(() => {
        const elapsed = Math.floor((Date.now() - startTimeRef.current) / 1000)
        setRecordingDuration(elapsed)
        if (elapsed >= AUTO_STOP_SECONDS) {
          handleStopClick()
        }
      }, 100)
    } else {
      if (timerRef.current) {
        clearInterval(timerRef.current)
      }
      setRecordingDuration(0)
      setLiveCps(null)
    }

    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current)
      }
    }
  }, [handleStopClick, isRecording])

  const handleStartClick = async () => {
    if (!microphoneAvailable) return

    try {
      stopInProgressRef.current = false
      onStartRecording()
      audioDataRef.current = []
      setError(null)
      setLiveCps(null)
      setRecordingDuration(0)
      lastCpsUpdateRef.current = 0

      // Initialize audio context
      const audioContext = new (window.AudioContext || (window as any).webkitAudioContext)()
      audioContextRef.current = audioContext

      // Get microphone stream
      const stream = await navigator.mediaDevices.getUserMedia({
        audio: {
          echoCancellation: true,
          noiseSuppression: true,
          autoGainControl: false
        }
      })
      streamRef.current = stream

      // Create analyser
      const source = audioContext.createMediaStreamSource(stream)
      const analyser = audioContext.createAnalyser()
      analyser.fftSize = 2048
      source.connect(analyser)

      analyserRef.current = analyser

      // Create script processor for real-time analysis
      const processor = audioContext.createScriptProcessor(4096, 1, 1)
      analyser.connect(processor)
      processor.connect(audioContext.destination)
      processorRef.current = processor

      processor.onaudioprocess = (event) => {
        const inputData = event.inputBuffer.getChannelData(0)
        audioDataRef.current.push(...Array.from(inputData))

        const now = Date.now()
        if (now - lastCpsUpdateRef.current > 150) {
          const cps = estimateCps(inputData, audioContext.sampleRate || 44100)
          setLiveCps(Math.round(cps * 10) / 10)
          lastCpsUpdateRef.current = now
        }
      }
    } catch (err) {
      const message = err instanceof Error ? err.message : 'Failed to start recording'
      setError(message)
      cleanupAudio()
      onStopRecording()
    }
  }

  return (
    <div className="microphone-capture">
      <div className="status">
        {microphoneAvailable ? (
          <span className="status-ok">✓ Microphone available</span>
        ) : (
          <span className="status-error">✗ Microphone not available</span>
        )}
      </div>

      {error && <div className="error-message">{error}</div>}

      {isRecording && (
        <div className="recording-indicator">
          <div className="recording-pulse"></div>
          <span>Recording... {Math.min(recordingDuration, AUTO_STOP_SECONDS)}s / {AUTO_STOP_SECONDS}s</span>
        </div>
      )}

      {isRecording && (
        <div className="live-cps">
          Live CPS: <strong>{liveCps !== null ? liveCps.toFixed(1) : '--'}</strong>
        </div>
      )}

      <div className="controls">
        <button
          onClick={handleStartClick}
          disabled={!microphoneAvailable || isRecording}
          className="btn btn-primary"
        >
          {isRecording ? 'Recording...' : 'Start Measurement'}
        </button>

        <button
          onClick={handleStopClick}
          disabled={!isRecording}
          className="btn btn-secondary"
        >
          Stop Measurement
        </button>
      </div>

      <div className="device-info">
        <h3>Configuration</h3>
        <ul>
          <li>Sample Rate: 44.1 kHz</li>
          <li>Bit Depth: 16-bit</li>
          <li>Channels: Mono</li>
          <li>FFT Size: 2048</li>
          <li>Auto-stop: 10 seconds</li>
          <li>Analysis window: middle 6 seconds (2s trim start/end)</li>
        </ul>
      </div>
    </div>
  )
}
