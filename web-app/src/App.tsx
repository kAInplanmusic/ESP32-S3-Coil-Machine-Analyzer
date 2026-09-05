import React from 'react'
import './App.css'
import { MicrophoneCapture } from './components/MicrophoneCapture'
import { MeasurementDisplay } from './components/MeasurementDisplay'
import { ResultsPanel } from './components/ResultsPanel'

function App() {
  const [isRecording, setIsRecording] = React.useState(false)
  const [measurementResult, setMeasurementResult] = React.useState(null)

  const handleStartRecording = async () => {
    setIsRecording(true)
    // Audio capture logic will be implemented in MicrophoneCapture component
  }

  const handleStopRecording = () => {
    setIsRecording(false)
  }

  return (
    <div className="app">
      <header className="app-header">
        <h1>Coil Machine Analyzer</h1>
        <p>Real-time waveform analysis from your microphone</p>
      </header>

      <main className="app-main">
        <div className="control-panel">
          <MicrophoneCapture
            isRecording={isRecording}
            onStartRecording={handleStartRecording}
            onStopRecording={handleStopRecording}
            onMeasurementComplete={setMeasurementResult}
          />
        </div>

        {isRecording && <MeasurementDisplay />}

        {measurementResult && <ResultsPanel result={measurementResult} />}
      </main>

      <footer className="app-footer">
        <p>Cross-platform Coil Machine Analyzer | v1.0.0</p>
      </footer>
    </div>
  )
}

export default App
