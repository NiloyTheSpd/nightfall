import React, { useEffect, useRef, useState } from 'react';
import { TrendingUp, Battery, Wind, Gauge } from 'lucide-react';

// Simple line chart component for real-time telemetry data
const TelemetryChart = ({ data, title, color, icon: Icon, unit = '', maxDataPoints = 50 }) => {
  const canvasRef = useRef(null);
  const [stats, setStats] = useState({ min: 0, max: 0, avg: 0, current: 0 });

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas || data.length === 0) return;

    const ctx = canvas.getContext('2d');
    const width = canvas.width;
    const height = canvas.height;
    const padding = 10;
    const chartWidth = width - padding * 2;
    const chartHeight = height - padding * 2;

    // Calculate statistics
    const values = data.map(d => d.value);
    const min = Math.min(...values);
    const max = Math.max(...values);
    const avg = values.reduce((a, b) => a + b, 0) / values.length;
    const current = values[values.length - 1];

    setStats({ min, max, avg, current });

    // Clear canvas
    ctx.clearRect(0, 0, width, height);

    // Background
    ctx.fillStyle = '#0b1220';
    ctx.fillRect(0, 0, width, height);

    // Grid lines
    ctx.strokeStyle = '#1e293b';
    ctx.lineWidth = 1;
    
    // Horizontal grid lines
    for (let i = 0; i <= 4; i++) {
      const y = padding + (chartHeight / 4) * i;
      ctx.beginPath();
      ctx.moveTo(padding, y);
      ctx.lineTo(width - padding, y);
      ctx.stroke();
    }

    // Draw area under line
    if (data.length > 1) {
      const gradient = ctx.createLinearGradient(0, padding, 0, height - padding);
      gradient.addColorStop(0, `${color}40`);
      gradient.addColorStop(1, `${color}00`);
      
      ctx.fillStyle = gradient;
      ctx.beginPath();
      
      data.forEach((point, index) => {
        const x = padding + (chartWidth / (maxDataPoints - 1)) * index;
        const normalizedValue = (point.value - min) / (max - min || 1);
        const y = height - padding - normalizedValue * chartHeight;
        
        if (index === 0) {
          ctx.moveTo(x, y);
        } else {
          ctx.lineTo(x, y);
        }
      });
      
      ctx.lineTo(width - padding, height - padding);
      ctx.lineTo(padding, height - padding);
      ctx.closePath();
      ctx.fill();
    }

    // Draw line
    if (data.length > 0) {
      ctx.strokeStyle = color;
      ctx.lineWidth = 2;
      ctx.beginPath();
      
      data.forEach((point, index) => {
        const x = padding + (chartWidth / (maxDataPoints - 1)) * index;
        const normalizedValue = (point.value - min) / (max - min || 1);
        const y = height - padding - normalizedValue * chartHeight;
        
        if (index === 0) {
          ctx.moveTo(x, y);
        } else {
          ctx.lineTo(x, y);
        }
      });
      
      ctx.stroke();

      // Draw point at the end
      const lastPoint = data[data.length - 1];
      const lastX = padding + (chartWidth / (maxDataPoints - 1)) * (data.length - 1);
      const lastNormalizedValue = (lastPoint.value - min) / (max - min || 1);
      const lastY = height - padding - lastNormalizedValue * chartHeight;
      
      ctx.fillStyle = color;
      ctx.beginPath();
      ctx.arc(lastX, lastY, 3, 0, Math.PI * 2);
      ctx.fill();
    }

  }, [data, color, maxDataPoints]);

  return (
    <div className="card p-4">
      <div className="flex items-center justify-between mb-3">
        <div className="flex items-center gap-2">
          {Icon && <Icon className="w-5 h-5" style={{ color }} />}
          <h3 className="text-sm font-semibold text-gray-300">{title}</h3>
        </div>
        <div className="text-xl font-mono font-bold" style={{ color }}>
          {stats.current.toFixed(1)}{unit}
        </div>
      </div>

      <canvas 
        ref={canvasRef} 
        width={400} 
        height={120}
        className="w-full rounded-lg"
      />

      <div className="grid grid-cols-3 gap-2 mt-3 text-xs">
        <div className="bg-gray-800 p-2 rounded text-center">
          <div className="text-gray-500">Min</div>
          <div className="font-mono text-gray-300">{stats.min.toFixed(1)}</div>
        </div>
        <div className="bg-gray-800 p-2 rounded text-center">
          <div className="text-gray-500">Avg</div>
          <div className="font-mono text-gray-300">{stats.avg.toFixed(1)}</div>
        </div>
        <div className="bg-gray-800 p-2 rounded text-center">
          <div className="text-gray-500">Max</div>
          <div className="font-mono text-gray-300">{stats.max.toFixed(1)}</div>
        </div>
      </div>
    </div>
  );
};

export default TelemetryChart;

// Hook for managing telemetry history
export const useTelemetryHistory = (maxDataPoints = 50) => {
  const [batteryHistory, setBatteryHistory] = useState([]);
  const [gasHistory, setGasHistory] = useState([]);
  const [distanceHistory, setDistanceHistory] = useState([]);
  const [signalHistory, setSignalHistory] = useState([]);

  const addTelemetryPoint = (telemetry) => {
    const timestamp = Date.now();

    const addPoint = (setter, value) => {
      setter(prev => {
        const newData = [...prev, { timestamp, value }];
        return newData.slice(-maxDataPoints);
      });
    };

    addPoint(setBatteryHistory, telemetry.battery || 0);
    addPoint(setGasHistory, telemetry.gas || 0);
    addPoint(setDistanceHistory, telemetry.dist || 0);
    addPoint(setSignalHistory, telemetry.signal_strength || 0);
  };

  const clearHistory = () => {
    setBatteryHistory([]);
    setGasHistory([]);
    setDistanceHistory([]);
    setSignalHistory([]);
  };

  return {
    batteryHistory,
    gasHistory,
    distanceHistory,
    signalHistory,
    addTelemetryPoint,
    clearHistory
  };
};
