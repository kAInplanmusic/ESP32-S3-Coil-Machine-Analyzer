import { logger } from '../utils/logger.js';

const connections = new Set();

export function setupWebSocket(wss) {
  wss.on('connection', (ws) => {
    logger.info('WebSocket client connected');
    connections.add(ws);

    ws.on('message', (data) => {
      try {
        const message = JSON.parse(data.toString());
        handleMessage(ws, message);
      } catch (error) {
        logger.error('WebSocket message error:', error.message);
        ws.send(JSON.stringify({ error: 'Invalid message format' }));
      }
    });

    ws.on('close', () => {
      logger.info('WebSocket client disconnected');
      connections.delete(ws);
    });

    ws.on('error', (error) => {
      logger.error('WebSocket error:', error.message);
    });
  });
}

function handleMessage(ws, message) {
  const { type, payload } = message;

  logger.debug('WebSocket message received', { type });

  switch (type) {
    case 'MEASUREMENT_START':
      handleMeasurementStart(ws, payload);
      break;
    case 'AUDIO_DATA':
      handleAudioData(ws, payload);
      break;
    case 'MEASUREMENT_END':
      handleMeasurementEnd(ws, payload);
      break;
    case 'PING':
      ws.send(JSON.stringify({ type: 'PONG', timestamp: Date.now() }));
      break;
    default:
      logger.warn('Unknown message type:', type);
  }
}

function handleMeasurementStart(ws, payload) {
  ws.send(JSON.stringify({
    type: 'MEASUREMENT_STARTED',
    measurementId: payload?.measurementId || 'unknown',
    timestamp: Date.now(),
  }));
}

function handleAudioData(ws, payload) {
  // Process audio chunks as they arrive
  logger.debug('Audio data chunk received', {
    size: payload?.data?.length || 0,
  });
}

function handleMeasurementEnd(ws, payload) {
  ws.send(JSON.stringify({
    type: 'MEASUREMENT_COMPLETED',
    measurementId: payload?.measurementId,
    result: payload?.result || {},
    timestamp: Date.now(),
  }));
}

export function broadcastMessage(message) {
  const data = JSON.stringify(message);
  connections.forEach((ws) => {
    if (ws.readyState === 1) { // WebSocket.OPEN
      ws.send(data);
    }
  });
}
