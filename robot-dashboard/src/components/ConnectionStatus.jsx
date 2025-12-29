import React from 'react';
import { Wifi, WifiOff, Radio, AlertTriangle } from 'lucide-react';

const ConnectionStates = {
  DISCONNECTED: 'disconnected',
  CONNECTING: 'connecting', 
  CONNECTED: 'connected',
  RECONNECTING: 'reconnecting',
  ERROR: 'error'
};

const ConnectionStatus = ({ 
  connectionState, 
  reconnectAttempts = 0, 
  latency = 0,
  messagesReceived = 0 
}) => {
  const getConnectionDisplay = () => {
    switch (connectionState) {
      case ConnectionStates.CONNECTED:
        return { 
          icon: Wifi, 
          text: 'Connected', 
          className: 'status-connected',
          bgColor: 'bg-nightfall-success'
        };
      case ConnectionStates.CONNECTING:
        return { 
          icon: Radio, 
          text: 'Connecting...', 
          className: 'status-reconnecting',
          bgColor: 'bg-yellow-500'
        };
      case ConnectionStates.RECONNECTING:
        return { 
          icon: Radio, 
          text: `Reconnecting (${reconnectAttempts}/4)`, 
          className: 'status-reconnecting',
          bgColor: 'bg-nightfall-warning'
        };
      case ConnectionStates.ERROR:
        return { 
          icon: AlertTriangle, 
          text: 'Error', 
          className: 'status-disconnected',
          bgColor: 'bg-nightfall-error'
        };
      default:
        return { 
          icon: WifiOff, 
          text: 'Disconnected', 
          className: 'status-disconnected',
          bgColor: 'bg-gray-500'
        };
    }
  };

  const connectionDisplay = getConnectionDisplay();
  const StatusIcon = connectionDisplay.icon;

  return (
    <div className="flex items-center gap-4">
      <div className={`status-indicator ${connectionDisplay.className}`}>
        <StatusIcon className="w-5 h-5 mr-2" />
        {connectionDisplay.text}
      </div>
      
      {latency > 0 && (
        <div className="bg-gray-700 px-3 py-2 rounded-lg text-sm">
          <div className="text-gray-400">Latency</div>
          <div className="font-mono font-bold text-blue-400">{latency}ms</div>
        </div>
      )}
      
      {messagesReceived > 0 && (
        <div className="bg-gray-700 px-3 py-2 rounded-lg text-sm">
          <div className="text-gray-400">Messages</div>
          <div className="font-mono font-bold text-green-400">{messagesReceived}</div>
        </div>
      )}
    </div>
  );
};

export default ConnectionStatus;