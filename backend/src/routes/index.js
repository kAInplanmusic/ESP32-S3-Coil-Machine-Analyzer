import express from 'express';
import { measurementRoutes } from './measurements.js';
import { analysisRoutes } from './analysis.js';
import { exportRoutes } from './export.js';

export function setupRoutes() {
  const router = express.Router();

  // Mount route modules
  router.use('/measurements', measurementRoutes());
  router.use('/analysis', analysisRoutes());
  router.use('/export', exportRoutes());

  // API info
  router.get('/', (req, res) => {
    res.json({
      name: 'Coil Machine Analyzer Backend',
      version: '3.0.0',
      endpoints: {
        measurements: '/measurements',
        analysis: '/analysis',
        export: '/export',
      },
    });
  });

  return router;
}
