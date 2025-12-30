import React, { useState } from 'react';
import { Download, FileText, Package } from 'lucide-react';

const DataExportPanel = ({ telemetry, waypoints, alerts, connectionStats }) => {
  const [exportFormat, setExportFormat] = useState('json');

  const exportTelemetryData = () => {
    const data = {
      exportDate: new Date().toISOString(),
      telemetry,
      connectionStats,
      alerts: alerts.slice(0, 10),
      waypoints,
      metadata: {
        dashboardVersion: '2.0.0',
        robotId: 'nightfall-01'
      }
    };

    let content, filename, type;

    if (exportFormat === 'json') {
      content = JSON.stringify(data, null, 2);
      filename = `telemetry-${Date.now()}.json`;
      type = 'application/json';
    } else if (exportFormat === 'csv') {
      // Convert to CSV
      const headers = Object.keys(telemetry).join(',');
      const values = Object.values(telemetry).join(',');
      content = `${headers}\n${values}`;
      filename = `telemetry-${Date.now()}.csv`;
      type = 'text/csv';
    } else {
      // Plain text report
      content = `
PROJECT NIGHTFALL - TELEMETRY REPORT
Generated: ${new Date().toLocaleString()}
=====================================

CURRENT STATUS:
- Battery: ${telemetry.battery}V
- Distance: ${telemetry.dist}cm
- Gas Level: ${telemetry.gas}
- Signal: ${telemetry.signal_strength}%
- Uptime: ${Math.floor(telemetry.uptime / 3600)}h ${Math.floor((telemetry.uptime % 3600) / 60)}m

SYSTEM HEALTH:
- Back ESP32: ${telemetry.back_status}
- Front ESP32: ${telemetry.front_status ? 'Online' : 'Offline'}
- Camera ESP32: ${telemetry.camera_status ? 'Online' : 'Offline'}

CONNECTION STATS:
- Messages Sent: ${connectionStats.messagesSent}
- Messages Received: ${connectionStats.messagesReceived}
- Average Latency: ${connectionStats.averageLatency}ms

WAYPOINTS: ${waypoints.length}
${waypoints.map((wp, i) => `${i + 1}. ${wp.name} (${wp.x.toFixed(0)}, ${wp.y.toFixed(0)})`).join('\n')}

RECENT ALERTS: ${alerts.length}
${alerts.slice(0, 5).map(a => `- ${a.message} (${a.timestamp.toLocaleTimeString()})`).join('\n')}
      `;
      filename = `report-${Date.now()}.txt`;
      type = 'text/plain';
    }

    const blob = new Blob([content], { type });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.href = url;
    link.download = filename;
    link.click();
    URL.revokeObjectURL(url);
  };

  return (
    <div className="space-y-4">
      <h3 className="text-sm font-semibold text-gray-300 flex items-center gap-2">
        <Download className="w-4 h-4 text-nightfall-accent" />
        Export Data
      </h3>

      <div className="bg-gray-800 p-4 rounded-lg space-y-3">
        <div>
          <label className="text-sm text-gray-400 mb-2 block">Export Format</label>
          <div className="grid grid-cols-3 gap-2">
            <button
              onClick={() => setExportFormat('json')}
              className={`p-2 rounded-lg text-sm transition-colors ${
                exportFormat === 'json'
                  ? 'bg-nightfall-accent text-white'
                  : 'bg-gray-700 text-gray-300 hover:bg-gray-600'
              }`}
            >
              <Package className="w-4 h-4 mx-auto mb-1" />
              JSON
            </button>
            <button
              onClick={() => setExportFormat('csv')}
              className={`p-2 rounded-lg text-sm transition-colors ${
                exportFormat === 'csv'
                  ? 'bg-nightfall-accent text-white'
                  : 'bg-gray-700 text-gray-300 hover:bg-gray-600'
              }`}
            >
              <FileText className="w-4 h-4 mx-auto mb-1" />
              CSV
            </button>
            <button
              onClick={() => setExportFormat('txt')}
              className={`p-2 rounded-lg text-sm transition-colors ${
                exportFormat === 'txt'
                  ? 'bg-nightfall-accent text-white'
                  : 'bg-gray-700 text-gray-300 hover:bg-gray-600'
              }`}
            >
              <FileText className="w-4 h-4 mx-auto mb-1" />
              TXT
            </button>
          </div>
        </div>

        <button
          onClick={exportTelemetryData}
          className="w-full btn-primary flex items-center justify-center gap-2"
        >
          <Download className="w-4 h-4" />
          Export {exportFormat.toUpperCase()}
        </button>
      </div>
    </div>
  );
};

export default DataExportPanel;
