import axios from 'axios';

const API_BASE = '/api/v1';

const apiClient = axios.create({
  baseURL: API_BASE,
  timeout: 30000,
  headers: {
    'Content-Type': 'application/json',
  },
});

export const getMeasurements = async () => {
  const response = await apiClient.get('/measurements');
  return response.data;
};

export const getMeasurement = async (id) => {
  const response = await apiClient.get(`/measurements/${id}`);
  return response.data;
};

export const createMeasurement = async (data) => {
  const response = await apiClient.post('/measurements', data);
  return response.data;
};

export const deleteMeasurement = async (id) => {
  const response = await apiClient.delete(`/measurements/${id}`);
  return response.data;
};

export const exportMeasurement = async (id, format) => {
  const url = `/export/${id}/${format}`;
  window.open(url, '_blank');
};

export default apiClient;
