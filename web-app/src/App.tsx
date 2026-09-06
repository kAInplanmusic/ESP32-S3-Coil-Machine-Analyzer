import React from 'react'
import './App.css'
import { MicrophoneCapture } from './components/MicrophoneCapture'
import { MeasurementDisplay } from './components/MeasurementDisplay'
import { ResultsPanel } from './components/ResultsPanel'
import { MeasurementHistory } from './components/MeasurementHistory'
import type { MeasurementResult } from './types'

function App() {
  const [isRecording, setIsRecording] = React.useState(false)
  const [measurementResult, setMeasurementResult] = React.useState<MeasurementResult | null>(null)
  const [showHistory, setShowHistory] = React.useState(false)

  const handleStartRecording = async () => {
    setIsRecording(true)
  }

  const handleStopRecording = () => {
    setIsRecording(false)
  }

  const handleNewMeasurement = () => {
    setMeasurementResult(null)
    setShowHistory(false)
  }

  return (
    <div className="app">
      <header className="app-header">
        <div className="header-content">
          <div className="header-left">
            <h1>Coil Machine Analyzer</h1>
            <p>Professional waveform analysis using your microphone</p>
          </div>
          <div className="header-right">
            <button
              className={`tab-btn ${!showHistory ? 'active' : ''}`}
              onClick={() => setShowHistory(false)}
            >
              Analyze
            </button>
            <button
              className={`tab-btn ${showHistory ? 'active' : ''}`}
              onClick={() => setShowHistory(true)}
            >
              History
            </button>
          </div>
        </div>
      </header>

      <main className="app-main">
        {!showHistory ? (
          <>
            <div className="control-panel">
              <MicrophoneCapture
                isRecording={isRecording}
                onStartRecording={handleStartRecording}
                onStopRecording={handleStopRecording}
                onMeasurementComplete={setMeasurementResult}
              />
            </div>

            {isRecording && <MeasurementDisplay measurement={measurementResult} />}

            {measurementResult && (
              <>
                <ResultsPanel result={measurementResult} />
                <button className="btn-new-measurement" onClick={handleNewMeasurement}>
                  ➕ New Measurement
                </button>
              </>
            )}
          </>
        ) : (
          <MeasurementHistory />
        )}
      </main>

      <footer className="app-footer">
        <p>Cross-platform Coil Machine Analyzer | v1.0.0 | Built with React + TypeScript</p>
      </footer>
    </div>
  )
}

export default App
