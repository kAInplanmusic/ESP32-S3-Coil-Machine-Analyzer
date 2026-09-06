import React from 'react'
import type { MeasurementResult } from '../types'

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
  
  const audioContextRef = React.useRef<AudioContext | null>(null)
  const analyserRef = React.useRef<AnalyserNode | null>(null)
  const streamRef = React.useRef<MediaStream | null>(null)
  const processorRef = React.useRef<ScriptProcessorNode | null>(null)
  const audioDataRef = React.useRef<number[]>([])
  const timerRef = React.useRef<NodeJS.Timeout | null>(null)
  const startTimeRef = React.useRef<number>(0)

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
      if (streamRef.current) {
        streamRef.current.getTracks().forEach(track => track.stop())
      }
      if (timerRef.current) {
        clearInterval(timerRef.current)
      }
    }
  }, [])

  // Update recording duration
  React.useEffect(() => {
    if (isRecording) {
      startTimeRef.current = Date.now()
      timerRef.current = setInterval(() => {
        const elapsed = Math.floor((Date.now() - startTimeRef.current) / 1000)
        setRecordingDuration(elapsed)
      }, 100)
    } else {
      if (timerRef.current) {
        clearInterval(timerRef.current)
      }
      setRecordingDuration(0)
    }

    return () => {
      if (timerRef.current) {
        clearInterval(timerRef.current)
      }
    }
  }, [isRecording])

  const analyzeAudio = (audioData: number[]): MeasurementResult => {
    // Calculate basic metrics
    const rms = Math.sqrt(audioData.reduce((sum, val) => sum + val * val, 0) / audioData.length)
    const peak = Math.max(...audioData.map(Math.abs))
    const mean = audioData.reduce((a, b) => a + b, 0) / audioData.length

    // Simple FFT frequency detection
    const sampleRate = 44100
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
      duration: recordingDuration,
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

  const handleStartClick = async () => {
    if (!microphoneAvailable) return

    try {
      onStartRecording()
      audioDataRef.current = []
      setError(null)

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
      }
    } catch (err) {
      const message = err instanceof Error ? err.message : 'Failed to start recording'
      setError(message)
      onStopRecording()
    }
  }

  const handleStopClick = () => {
    // Stop recording
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

    // Analyze collected audio
    if (audioDataRef.current.length > 0) {
      const result = analyzeAudio(audioDataRef.current)
      onMeasurementComplete(result)
    }

    onStopRecording()
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
          <span>Recording... {recordingDuration}s</span>
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
        </ul>
      </div>
    </div>
  )
}
