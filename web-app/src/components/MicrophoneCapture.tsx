import React from 'react'

interface MicrophoneCaptureProps {
  isRecording: boolean
  onStartRecording: () => void
  onStopRecording: () => void
  onMeasurementComplete: (result: any) => void
}

export const MicrophoneCapture: React.FC<MicrophoneCaptureProps> = ({
  isRecording,
  onStartRecording,
  onStopRecording,
  onMeasurementComplete
}) => {
  const [microphoneAvailable, setMicrophoneAvailable] = React.useState(false)
  const [error, setError] = React.useState<string | null>(null)

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
  }, [])

  const handleStartClick = () => {
    if (microphoneAvailable) {
      onStartRecording()
    }
  }

  const handleStopClick = () => {
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
          <li>Target Cycles: 100</li>
        </ul>
      </div>
    </div>
  )
}
