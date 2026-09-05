import { useEffect, useState } from 'react';

export function useAudioCapture() {
  const [isSupported, setIsSupported] = useState(false);
  const [audioContext, setAudioContext] = useState(null);
  const [mediaStream, setMediaStream] = useState(null);

  useEffect(() => {
    const supported = !!(
      window.AudioContext ||
      window.webkitAudioContext ||
      navigator.mediaDevices?.getUserMedia
    );
    setIsSupported(supported);
  }, []);

  const startCapture = async () => {
    try {
      const stream = await navigator.mediaDevices.getUserMedia({
        audio: {
          echoCancellation: false,
          noiseSuppression: false,
          autoGainControl: false,
          sampleRate: 44100,
        },
      });

      const AudioContext = window.AudioContext || window.webkitAudioContext;
      const ctx = new AudioContext({ sampleRate: 44100 });
      const source = ctx.createMediaStreamSource(stream);

      setMediaStream(stream);
      setAudioContext(ctx);

      return ctx;
    } catch (error) {
      console.error('Failed to access microphone:', error);
      throw error;
    }
  };

  const stopCapture = () => {
    if (mediaStream) {
      mediaStream.getTracks().forEach(track => track.stop());
      setMediaStream(null);
    }
    if (audioContext) {
      audioContext.close();
      setAudioContext(null);
    }
  };

  return {
    isSupported,
    audioContext,
    mediaStream,
    startCapture,
    stopCapture,
  };
}
