import express from 'express';
import { dbGet } from '../database/db.js';
import { logger } from '../utils/logger.js';

export function exportRoutes() {
  const router = express.Router();

  // Export measurement as CSV
  router.get('/:id/csv', async (req, res, next) => {
    try {
      const measurement = await dbGet(
        'SELECT * FROM measurements WHERE id = ?',
        [req.params.id]
      );

      if (!measurement) {
        return res.status(404).json({ error: 'Measurement not found' });
      }

      const csvData = generateCSV(measurement);
      res.type('text/csv');
      res.set('Content-Disposition', `attachment; filename="measurement-${req.params.id}.csv"`);
      res.send(csvData);
    } catch (error) {
      next(error);
    }
  });

  // Export measurement as JSON
  router.get('/:id/json', async (req, res, next) => {
    try {
      const measurement = await dbGet(
        'SELECT * FROM measurements WHERE id = ?',
        [req.params.id]
      );

      if (!measurement) {
        return res.status(404).json({ error: 'Measurement not found' });
      }

      res.type('application/json');
      res.set('Content-Disposition', `attachment; filename="measurement-${req.params.id}.json"`);
      res.json(measurement);
    } catch (error) {
      next(error);
    }
  });

  return router;
}

function generateCSV(measurement) {
  const data = JSON.parse(measurement.data_json || '{}');
  let csv = 'Coil Machine Analyzer - Measurement Export\n';
  csv += `ID,${measurement.id}\n`;
  csv += `Created,${measurement.created_at}\n`;
  csv += `CPS,${measurement.cps}\n`;
  csv += `Quality Score,${measurement.quality_score}%\n`;
  csv += `Duration,${measurement.duration_ms}ms\n\n`;
  csv += 'Waveform Data:\n';
  csv += 'Time,Amplitude,Energy,Slope\n';
  return csv;
}
