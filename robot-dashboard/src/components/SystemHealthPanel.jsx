import React from 'react';
import { Cpu, Eye, Zap } from 'lucide-react';

const SystemStatus = {
  HEALTHY: 'healthy',
  WARNING: 'warning',
  CRITICAL: 'critical',
  OFFLINE: 'offline'
};

const SystemHealthPanel = ({ 
  systemHealth,
  telemetry,
  performanceMetrics,
  connectionStats
}) => {
  const getStatusColor = (status) => {
    switch (status) {
      case SystemStatus.HEALTHY: 
        return 'robot-status-brain'; // green
      case SystemStatus.WARNING: 
        return 'connection-warning'; // yellow
      case SystemStatus.CRITICAL: 
        return 'connection-error'; // red
      default: 
        return 'connection-error opacity-50'; // gray/offline
    }
  };

  const getDistanceColor = (distance) => {
    if (distance < 20) return 'text-red-400';
    if (distance < 50) return 'text-yellow-400';
    return 'text-green-400';
  };

  const getGasColor = (gas) => {
    if (gas > 3000) return 'text-red-400 animate-pulse';
    if (gas > 1500) return 'text-yellow-400';
    return 'text-green-400';
  };

  const getBatteryColor = (voltage) => {
    const percentage = Math.max(0, Math.min((voltage - 11) / (14.8 - 11) * 100, 100));
    if (percentage > 50) return 'text-green-400';
    if (percentage > 25) return 'text-yellow-400';
    return 'text-red-400';
  };

  const getBatteryBarColor = (voltage) => {
    const percentage = Math.max(0, Math.min((voltage - 11) / (14.8 - 11) * 100, 100));
    if (percentage > 50) return 'bg-green-400';
    if (percentage > 25) return 'bg-yellow-400';
    return 'bg-red-400';
  };

  const getGasBarColor = (gas) => {
    if (gas > 3000) return 'bg-red-400';
    if (gas > 1500) return 'bg-yellow-400';
    return 'bg-green-400';
  };

  return (
    <div className="card p-6">
      <h2 className="text-xl font-semibold mb-4 flex items-center gap-3">
        <Cpu className="w-6 h-6 text-robot-brain" />
        System Health
      </h2>

      {/* Connection Status Indicators */}
      <div className="space-y-4 mb-6">
        <div className="flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className={`robot-status-brain ${getStatusColor(systemHealth.brain)}`}></div>
            <span className="font-medium">BRAIN</span>
          </div>
          <span className="text-sm text-gray-400">Back ESP32</span>
        </div>

        <div className="flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className={`robot-status-motors ${getStatusColor(systemHealth.motors)}`}></div>
            <span className="font-medium">MOTORS</span>
          </div>
          <span className="text-sm text-gray-400">Front ESP32</span>
        </div>

        <div className="flex items-center justify-between">
          <div className="flex items-center gap-3">
            <div className={`robot-status-vision ${getStatusColor(systemHealth.vision)}`}></div>
            <span className="font-medium">VISION</span>
          </div>
          <span className="text-sm text-gray-400">Camera ESP32</span>
        </div>
      </div>

      {/* Real-time Sensor Metrics */}
      <div className="space-y-4">
        <h3 className="text-sm font-semibold text-gray-300">Real-time Sensors</h3>
        
        {/* Battery Level */}
        <div className="space-y-2">
          <div className="flex justify-between items-center">
            <span className="text-sm text-gray-400 flex items-center gap-2">
              <Cpu className="w-4 h-4" />
              Battery
            </span>
            <span className={`font-mono font-bold ${getBatteryColor(telemetry.battery)}`}>
              {telemetry.battery.toFixed(1)}V
            </span>
          </div>
          <div className="metric-bar">
            <div 
              className={`metric-fill ${getBatteryBarColor(telemetry.battery)}`}
              style={{ width: `${Math.max(0, Math.min((telemetry.battery - 11) / (14.8 - 11) * 100, 100))}%` }}
            ></div>
          </div>
        </div>

        {/* Distance Sensor */}
        <div className="space-y-2">
          <div className="flex justify-between items-center">
            <span className="text-sm text-gray-400">Distance</span>
            <span className={`font-mono font-bold ${getDistanceColor(telemetry.dist)}`}>
              {Math.round(telemetry.dist)}cm
            </span>
          </div>
          <div className="metric-bar">
            <div 
              className={`metric-fill ${getDistanceColor(telemetry.dist).replace('text-', 'bg-')}`}
              style={{ width: `${Math.min(telemetry.dist / 200 * 100, 100)}%` }}
            ></div>
          </div>
        </div>

        {/* Gas Level */}
        <div className="space-y-2">
          <div className="flex justify-between items-center">
            <span className="text-sm text-gray-400">Gas Level</span>
            <span className={`font-mono font-bold ${getGasColor(telemetry.gas)}`}>
              {Math.round(telemetry.gas)}
            </span>
          </div>
          <div className="metric-bar">
            <div 
              className={`metric-fill ${getGasBarColor(telemetry.gas)}`}
              style={{ width: `${Math.min(telemetry.gas / 4095 * 100, 100)}%` }}
            ></div>
          </div>
        </div>

        {/* Signal Strength */}
        {telemetry.signal_strength > 0 && (
          <div className="space-y-2">
            <div className="flex justify-between items-center">
              <span className="text-sm text-gray-400">WiFi Signal</span>
              <span className="font-mono font-bold text-blue-400">
                {telemetry.signal_strength}%
              </span>
            </div>
            <div className="metric-bar">
              <div 
                className="metric-fill bg-blue-500"
                style={{ width: `${telemetry.signal_strength}%` }}
              ></div>
            </div>
          </div>
        )}
      </div>

      {/* Performance Metrics */}
      <div className="mt-6 pt-4 border-t border-gray-700">
        <h3 className="text-sm font-semibold text-gray-300 mb-3">Performance</h3>
        <div className="grid grid-cols-1 gap-3 text-xs">
          <div className="bg-gray-800 p-3 rounded">
            <div className="text-gray-400">Data Rate</div>
            <div className="font-mono text-green-400">{performanceMetrics.dataRate} KB/s</div>
          </div>
          <div className="bg-gray-800 p-3 rounded">
            <div className="text-gray-400">Success Rate</div>
            <div className="font-mono text-blue-400">{performanceMetrics.commandSuccessRate}%</div>
          </div>
          <div className="bg-gray-800 p-3 rounded">
            <div className="text-gray-400">Messages Received</div>
            <div className="font-mono text-purple-400">{connectionStats.messagesReceived}</div>
          </div>
          <div className="bg-gray-800 p-3 rounded">
            <div className="text-gray-400">Average Latency</div>
            <div className="font-mono text-yellow-400">{connectionStats.averageLatency}ms</div>
          </div>
        </div>
      </div>
    </div>
  );
};

export default SystemHealthPanel;