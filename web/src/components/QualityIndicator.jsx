import React from 'react';
import '../styles/QualityIndicator.css';

function QualityIndicator({ score }) {
  let color = '#d32f2f'; // Red
  let text = 'Poor';

  if (score >= 95) {
    color = '#00c853'; // Bright green
    text = 'Perfect';
  } else if (score >= 86) {
    color = '#4caf50'; // Green
    text = 'Excellent';
  } else if (score >= 75) {
    color = '#fbc02d'; // Yellow
    text = 'Good';
  } else if (score >= 61) {
    color = '#ff9800'; // Orange
    text = 'Fair';
  }

  return (
    <div className="quality-indicator" style={{ color }}>
      <div className="quality-value">{score}%</div>
      <div className="quality-text">{text}</div>
    </div>
  );
}

export default QualityIndicator;
