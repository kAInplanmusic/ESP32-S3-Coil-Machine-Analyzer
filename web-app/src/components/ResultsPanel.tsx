import React from 'react'

interface ResultsPanelProps {
  result: any
}

export const ResultsPanel: React.FC<ResultsPanelProps> = ({ result }) => {
  const [activeTab, setActiveTab] = React.useState<'summary' | 'waveform' | 'harmonics' | 'decay'>('summary')

  const handleExportJSON = () => {
    const dataStr = JSON.stringify(result, null, 2)
    const dataBlob = new Blob([dataStr], { type: 'application/json' })
    const url = URL.createObjectURL(dataBlob)
    const link = document.createElement('a')
    link.href = url
    link.download = `measurement-${Date.now()}.json`
    link.click()
    URL.revokeObjectURL(url)
  }

  const handleExportCSV = () => {
    // TODO: Implement CSV export
    console.log('CSV export not yet implemented')
  }

  return (
    <div className="results-panel">
      <div className="results-header">
        <h2>Measurement Results</h2>
        <div className="quality-score">
          <span className="quality-label">Overall Quality:</span>
          <span className="quality-value">--</span>
        </div>
      </div>

      <div className="tabs">
        <button
          className={`tab ${activeTab === 'summary' ? 'active' : ''}`}
          onClick={() => setActiveTab('summary')}
        >
          Summary
        </button>
        <button
          className={`tab ${activeTab === 'waveform' ? 'active' : ''}`}
          onClick={() => setActiveTab('waveform')}
        >
          Waveform
        </button>
        <button
          className={`tab ${activeTab === 'harmonics' ? 'active' : ''}`}
          onClick={() => setActiveTab('harmonics')}
        >
          Harmonics
        </button>
        <button
          className={`tab ${activeTab === 'decay' ? 'active' : ''}`}
          onClick={() => setActiveTab('decay')}
        >
          Decay Analysis
        </button>
      </div>

      <div className="tab-content">
        {activeTab === 'summary' && (
          <div className="summary-tab">
            <div className="metrics-grid">
              <div className="metric">
                <label>Average CPS</label>
                <span className="value">--</span>
              </div>
              <div className="metric">
                <label>Peak Amplitude</label>
                <span className="value">--</span>
              </div>
              <div className="metric">
                <label>THD %</label>
                <span className="value">--</span>
              </div>
              <div className="metric">
                <label>Decay Rate</label>
                <span className="value">--</span>
              </div>
            </div>
          </div>
        )}

        {activeTab === 'waveform' && (
          <div className="waveform-tab">
            <canvas id="waveform-result-canvas"></canvas>
          </div>
        )}

        {activeTab === 'harmonics' && (
          <div className="harmonics-tab">
            <canvas id="harmonics-canvas"></canvas>
          </div>
        )}

        {activeTab === 'decay' && (
          <div className="decay-tab">
            <canvas id="decay-canvas"></canvas>
          </div>
        )}
      </div>

      <div className="export-section">
        <button onClick={handleExportJSON} className="btn btn-export">
          Export as JSON
        </button>
        <button onClick={handleExportCSV} className="btn btn-export">
          Export as CSV
        </button>
      </div>
    </div>
  )
}
