/**
 * AudioCapture Service
 * Handles microphone input via WebAudio API
 */

export class AudioCapture {
  private audioContext: AudioContext | null = null
  private mediaStream: MediaStream | null = null
  private scriptProcessor: ScriptProcessorNode | null = null
  private isRecording = false
  private audioBuffer: Float32Array[] = []

  /**
   * Check if microphone is available and request permission
   */
  async checkPermission(): Promise<boolean> {
    try {
      const stream = await navigator.mediaDevices.getUserMedia({ audio: true })
      stream.getTracks().forEach(track => track.stop())
      return true
    } catch {
      return false
    }
  }

  /**
   * Start recording audio from microphone
   */
  async start(): Promise<void> {
    if (this.isRecording) {
      throw new Error('Already recording')
    }

    try {
      // Initialize audio context
      const AudioContextClass = window.AudioContext || (window as any).webkitAudioContext
      this.audioContext = new AudioContextClass()

      // Get microphone stream
      this.mediaStream = await navigator.mediaDevices.getUserMedia({
        audio: {
          echoCancellation: false,
          noiseSuppression: false,
          autoGainControl: false,
          sampleRate: { ideal: 44100 }
        }
      })

      const source = this.audioContext.createMediaStreamSource(this.mediaStream)

      // Create script processor (deprecated but still widely supported)
      this.scriptProcessor = this.audioContext.createScriptProcessor(4096, 1, 1)
      this.scriptProcessor.onaudioprocess = this.handleAudioProcess_.bind(this)

      // Connect nodes
      source.connect(this.scriptProcessor)
      this.scriptProcessor.connect(this.audioContext.destination)

      this.isRecording = true
      this.audioBuffer = []
    } catch (error) {
      this.cleanup()
      throw error
    }
  }

  /**
   * Stop recording and return collected data
   */
  async stop(): Promise<any> {
    if (!this.isRecording) {
      throw new Error('Not recording')
    }

    this.isRecording = false

    // Stop media stream
    if (this.mediaStream) {
      this.mediaStream.getTracks().forEach(track => track.stop())
    }

    // Process collected audio
    const result = this.processAudio_()
    this.cleanup()
    return result
  }

  /**
   * Handle audio process event
   */
  private handleAudioProcess_(event: AudioProcessingEvent): void {
    if (!this.isRecording) return

    const inputData = event.inputBuffer.getChannelData(0)
    // Store a copy of the float data
    this.audioBuffer.push(new Float32Array(inputData))
  }

  /**
   * Process collected audio samples
   */
  private processAudio_(): any {
    // Combine all audio buffers
    const totalLength = this.audioBuffer.reduce((sum, buf) => sum + buf.length, 0)
    const combinedBuffer = new Float32Array(totalLength)
    
    let offset = 0
    for (const buffer of this.audioBuffer) {
      combinedBuffer.set(buffer, offset)
      offset += buffer.length
    }

    // Return measurement data structure
    return {
      samples: combinedBuffer,
      sample_rate: this.audioContext?.sampleRate || 44100,
      duration_seconds: combinedBuffer.length / (this.audioContext?.sampleRate || 44100),
      timestamp: new Date().toISOString()
    }
  }

  /**
   * Cleanup resources
   */
  private cleanup(): void {
    if (this.scriptProcessor) {
      this.scriptProcessor.disconnect()
    }
    if (this.audioContext && this.audioContext.state !== 'closed') {
      try {
        this.audioContext.close()
      } catch (e) {
        // Ignore close errors
      }
    }
    this.audioContext = null
    this.scriptProcessor = null
    this.mediaStream = null
  }
}
