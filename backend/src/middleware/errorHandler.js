import { logger } from '../utils/logger.js';

export function errorHandler(err, req, res, next) {
  logger.error('Request error:', {
    path: req.path,
    method: req.method,
    message: err.message,
    stack: err.stack,
  });

  const status = err.status || 500;
  const message = err.message || 'Internal Server Error';

  res.status(status).json({
    error: {
      status,
      message,
      timestamp: new Date().toISOString(),
    },
  });
}
