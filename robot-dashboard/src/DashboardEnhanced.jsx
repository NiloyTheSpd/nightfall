import React, { useEffect, useRef, useState, useCallback } from 'react';
import {
  Camera,
  Radio,
  Gauge,
  Battery,
  AlertTriangle,
  Wind,
  Navigation,
  Power,
  Activity,
  Wifi,
  WifiOff,
  Circle,
  Square,
  ArrowUp,
  ArrowDown,
  ArrowLeft,
  ArrowRight,
  RotateCw,
  StopCircle,
  MapPin,
  Play,
  Pause,
  Trash2,
  Save,
  Upload,
  Download,
  Target,
  CheckCircle,
  AlertCircle,
  Cpu,
  Eye,
  Zap,
  Clock,
  Signal,
  Settings,
  Maximize2,
  BarChart3,
  Info
} from 'lucide-react';

// Import new components
import { DashboardLoader } from './components/LoadingSkeleton';
import TelemetryChart, { useTelemetryHistory } from './components/TelemetryChart';
import SettingsPanel from './components/SettingsPanel';
import FullscreenVideo from './components/FullscreenVideo';
import DataExportPanel from './components/DataExportPanel';
import Tooltip from './components/Tooltip';

// Enhanced Mission Control Dashboard for Project Nightfall Rescue Robot
// Features: Real WebSocket client, Three-zone layout, Performance monitoring, Error handling

// WebSocket connection states
const ConnectionStates = {
  DISCONNECTED: 'disconnected',
  CONNECTING: 'connecting', 
  CONNECTED: 'connected',
  RECONNECTING: 'reconnecting',
  ERROR: 'error'
};

// Robot system status types
const SystemStatus = {
  HEALTHY: 'healthy',
  WARNING: 'warning',
  CRITICAL: 'critical',
  OFFLINE: 'offline'
};

// Types for better TypeScript-like development
/**
 * @typedef {Object} TelemetryData
 * @property {string} back_status - Status of rear ESP32
 * @property {boolean} front_status - Status of front ESP32 
 * @property {boolean} camera_status - Status of camera ESP32
 * @property {number} dist - Distance reading in cm
 * @property {number} gas - Gas level (0-4095)
 * @property {string} cam_ip - Camera IP address
 * @property {number} battery - Battery voltage
 * @property {number} signal_strength - WiFi signal strength
 * @property {number} uptime - System uptime in seconds
 */

/**
 * @typedef {Object} Waypoint
 * @property {number} id - Unique identifier
 * @property {number} x - X coordinate
 * @property {number} y - Y coordinate  
 * @property {string} name - Waypoint name
 * @property {string} [action] - Action to perform
 */

export default function DashboardEnhanced() {
  // --- State for new features ---
  const [isLoading, setIsLoading] = useState(true);
  const [showSettings, setShowSettings] = useState(false);
  const [showFullscreenVideo, setShowFullscreenVideo] = useState(false);
  const [showAnalytics, setShowAnalytics] = useState(false);
  const [settings, setSettings] = useState(() => {
    const saved = localStorage.getItem('dashboard-settings');
    return saved ? JSON.parse(saved) : {
      theme: 'dark',
      enableSounds: false,
      autoReconnect: true,
      showNotifications: true,
      videoQuality: 'high',
      dataRefreshRate: 100,
      commandThrottle: 50,
      maxTelemetryHistory: 50
    };
  });

  // --- Telemetry history for charts ---
  const {
    batteryHistory,
    gasHistory,
    distanceHistory,
    signalHistory,
    addTelemetryPoint,
    clearHistory
  } = useTelemetryHistory(settings.maxTelemetryHistory);

  // --- WebSocket connection management ---
  const [connectionState, setConnectionState] = useState(ConnectionStates.DISCONNECTED);
  const [lastError, setLastError] = useState(null);
  const [reconnectAttempts, setReconnectAttempts] = useState(0);
  const [connectionStats, setConnectionStats] = useState({
    messagesSent: 0,
    messagesReceived: 0,
    lastMessageTime: null,
    averageLatency: 0
  });

  // --- Real-time telemetry data ---
  const [telemetry, setTelemetry] = useState({
    back_status: 'offline',
    front_status: false,
    camera_status: false,
    dist: 0,
    gas: 0,
    cam_ip: '192.168.4.3',
    battery: 14.8,
    signal_strength: 0,
    uptime: 0
  });

  // --- System health monitoring ---
  const [systemHealth, setSystemHealth] = useState({
    brain: SystemStatus.OFFLINE,    // Back ESP32
    motors: SystemStatus.OFFLINE,   // Front ESP32 UART
    vision: SystemStatus.OFFLINE    // Camera ESP32
  });

  // --- Motor control state ---
  const [motorState, setMotorState] = useState({ left: 0, right: 0 });
  const [commandQueue, setCommandQueue] = useState([]);
  const [lastCommand, setLastCommand] = useState(null);

  // --- Mission and navigation state ---
  const [mode, setMode] = useState<'manual' | 'autonomous'>('manual');
  const [robotState, setRobotState] = useState('IDLE');
  const [waypoints, setWaypoints] = useState([]);
  const [currentWaypoint, setCurrentWaypoint] = useState(0);
  const [isNavigating, setIsNavigating] = useState(false);
  const [robotPosition, setRobotPosition] = useState({ x: 400, y: 200 });
  const canvasRef = useRef(null);

  // --- Performance monitoring ---
  const [performanceMetrics, setPerformanceMetrics] = useState({
    fps: 0,
    videoLatency: 0,
    commandSuccessRate: 100,
    dataRate: 0,
    memoryUsage: 0
  });

  // --- Alerts and notifications ---
  const [alerts, setAlerts] = useState([]);
  const [videoFps, setVideoFps] = useState(0);

  // --- WebSocket ref for direct communication ---
  const wsRef = useRef(null);
  const commandTimeoutRef = useRef(null);
  const reconnectTimeoutRef = useRef(null);
  const pingIntervalRef = useRef(null);
  const fpsCounterRef = useRef({ frames: 0, lastTime: Date.now() });

  // --- Constants ---
  const WEBSOCKET_URL = 'ws://192.168.4.1:8888';
  const RECONNECT_DELAYS = [2000, 4000, 8000, 16000]; // Exponential backoff
  const COMMAND_THROTTLE = 50; // Minimum ms between commands

  // --- Utility functions ---
  const addAlert = useCallback((message, type = 'info', duration = 5000) => {
    const alert = {
      id: Date.now() + Math.random(),
      message,
      type,
      timestamp: new Date()
    };
    
    setAlerts(prev => [alert, ...prev].slice(0, 10));
    
    if (duration > 0) {
      setTimeout(() => {
        setAlerts(prev => prev.filter(a => a.id !== alert.id));
      }, duration);
    }
  }, []);

  const updateSystemHealth = useCallback((newTelemetry) => {
    setSystemHealth({
      brain: newTelemetry.back_status === 'ok' ? SystemStatus.HEALTHY : SystemStatus.CRITICAL,
      motors: newTelemetry.front_status ? SystemStatus.HEALTHY : SystemStatus.CRITICAL,
      vision: newTelemetry.camera_status ? SystemStatus.HEALTHY : SystemStatus.OFFLINE
    });
  }, []);

  // --- WebSocket connection management ---
  const connectWebSocket = useCallback(() => {
    if (wsRef.current?.readyState === WebSocket.OPEN) {
      return; // Already connected
    }

    try {
      setConnectionState(ConnectionStates.CONNECTING);
      setLastError(null);

      const ws = new WebSocket(WEBSOCKET_URL);
      wsRef.current = ws;

      ws.onopen = () => {
        console.log('WebSocket connected to robot');
        setConnectionState(ConnectionStates.CONNECTED);
        setReconnectAttempts(0);
        addAlert('Connected to robot successfully', 'success');
        
        // Start ping interval for latency measurement
        pingIntervalRef.current = setInterval(() => {
          if (ws.readyState === WebSocket.OPEN) {
            const pingTime = Date.now();
            ws.send(JSON.stringify({ type: 'ping', timestamp: pingTime }));
          }
        }, 5000);
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          const now = Date.now();
          
          setConnectionStats(prev => ({
            ...prev,
            messagesReceived: prev.messagesReceived + 1,
            lastMessageTime: now
          }));

          if (data.type === 'pong') {
            const latency = now - data.timestamp;
            setConnectionStats(prev => ({
              ...prev,
              averageLatency: Math.round((prev.averageLatency + latency) / 2)
            }));
            return;
          }

          if (data.type === 'telemetry') {
            setTelemetry(data.payload);
            updateSystemHealth(data.payload);
            addTelemetryPoint(data.payload); // Add to history for charts
            
            // Update performance metrics
            setPerformanceMetrics(prev => ({
              ...prev,
              dataRate: prev.dataRate + JSON.stringify(data).length
            }));
          }

        } catch (error) {
          console.error('Failed to parse WebSocket message:', error);
          addAlert('Invalid data received from robot', 'warning');
        }
      };

      ws.onerror = (error) => {
        console.error('WebSocket error:', error);
        setConnectionState(ConnectionStates.ERROR);
        setLastError('Connection error');
        addAlert('WebSocket connection error', 'error');
      };

      ws.onclose = (event) => {
        console.log('WebSocket closed:', event.code, event.reason);
        setConnectionState(ConnectionStates.DISCONNECTED);
        
        // Clear intervals
        if (pingIntervalRef.current) {
          clearInterval(pingIntervalRef.current);
          pingIntervalRef.current = null;
        }

        // Auto-reconnect with exponential backoff
        if (reconnectAttempts < RECONNECT_DELAYS.length) {
          setConnectionState(ConnectionStates.RECONNECTING);
          const delay = RECONNECT_DELAYS[reconnectAttempts];
          
          addAlert(`Reconnecting in ${delay/1000}s...`, 'warning', delay);
          
          reconnectTimeoutRef.current = setTimeout(() => {
            setReconnectAttempts(prev => prev + 1);
            connectWebSocket();
          }, delay);
        } else {
          addAlert('Failed to reconnect to robot. Please check connection.', 'error');
        }
      };

    } catch (error) {
      console.error('Failed to create WebSocket:', error);
      setConnectionState(ConnectionStates.ERROR);
      setLastError(error.message);
      addAlert('Failed to establish connection', 'error');
    }
  }, [addAlert, reconnectAttempts, updateSystemHealth]);

  // --- Command transmission ---
  const sendMotorCommand = useCallback((left, right) => {
    if (!wsRef.current || wsRef.current.readyState !== WebSocket.OPEN) {
      addAlert('Cannot send command: Not connected to robot', 'error');
      return false;
    }

    // Command throttling
    const now = Date.now();
    if (lastCommand && now - lastCommand.timestamp < COMMAND_THROTTLE) {
      return false;
    }

    const command = { L: left, R: right };
    
    try {
      wsRef.current.send(JSON.stringify(command));
      
      setMotorState({ left, right });
      setLastCommand({ command, timestamp: now });
      setConnectionStats(prev => ({
        ...prev,
        messagesSent: prev.messagesSent + 1
      }));

      return true;
    } catch (error) {
      console.error('Failed to send command:', error);
      addAlert('Failed to send motor command', 'error');
      return false;
    }
  }, [addAlert, lastCommand]);

  // --- Movement commands ---
  const moveForward = useCallback(() => sendMotorCommand(200, 200), [sendMotorCommand]);
  const moveBackward = useCallback(() => sendMotorCommand(-150, -150), [sendMotorCommand]);
  const turnLeft = useCallback(() => sendMotorCommand(-100, 100), [sendMotorCommand]);
  const turnRight = useCallback(() => sendMotorCommand(100, -100), [sendMotorCommand]);
  const rotateClockwise = useCallback(() => sendMotorCommand(140, -140), [sendMotorCommand]);
  const emergencyStop = useCallback(() => {
    sendMotorCommand(0, 0);
    setMode('manual');
    setRobotState('EMERGENCY');
    addAlert('EMERGENCY STOP ACTIVATED', 'error', 0);
  }, [sendMotorCommand, addAlert]);

  // --- Keyboard controls ---
  useEffect(() => {
    const handleKeyPress = (event) => {
      if (mode === 'autonomous') return;
      
      switch (event.key.toLowerCase()) {
        case 'w':
        case 'arrowup':
          event.preventDefault();
          moveForward();
          break;
        case 's':
        case 'arrowdown':
          event.preventDefault();
          moveBackward();
          break;
        case 'a':
        case 'arrowleft':
          event.preventDefault();
          turnLeft();
          break;
        case 'd':
        case 'arrowright':
          event.preventDefault();
          turnRight();
          break;
        case ' ':
        case 'escape':
          event.preventDefault();
          emergencyStop();
          break;
        case 'r':
          if (event.ctrlKey) {
            event.preventDefault();
            rotateClockwise();
          }
          break;
      }
    };

    window.addEventListener('keydown', handleKeyPress);
    return () => window.removeEventListener('keydown', handleKeyPress);
  }, [mode, moveForward, moveBackward, turnLeft, turnRight, emergencyStop, rotateClockwise]);

  // --- Mode management ---
  const toggleMode = useCallback(() => {
    const newMode = mode === 'manual' ? 'autonomous' : 'manual';
    setMode(newMode);
    setRobotState(newMode === 'autonomous' ? 'AUTONOMOUS' : 'IDLE');
    addAlert(`Switched to ${newMode.toUpperCase()} mode`, 'success');
  }, [mode, addAlert]);

  // --- Connection lifecycle ---
  useEffect(() => {
    // Simulate initial loading
    setTimeout(() => setIsLoading(false), 1000);
    
    connectWebSocket();

    return () => {
      // Cleanup on unmount
      if (wsRef.current) {
        wsRef.current.close();
      }
      if (reconnectTimeoutRef.current) {
        clearTimeout(reconnectTimeoutRef.current);
      }
      if (pingIntervalRef.current) {
        clearInterval(pingIntervalRef.current);
      }
    };
  }, [connectWebSocket]);

  // --- FPS counter for video ---
  useEffect(() => {
    const interval = setInterval(() => {
      const now = Date.now();
      const counter = fpsCounterRef.current;
      const deltaTime = now - counter.lastTime;
      
      if (deltaTime >= 1000) {
        setVideoFps(counter.frames);
        counter.frames = 0;
        counter.lastTime = now;
      }
    }, 100);

    return () => clearInterval(interval);
  }, []);

  // --- Performance monitoring update ---
  useEffect(() => {
    const interval = setInterval(() => {
      setPerformanceMetrics(prev => ({
        ...prev,
        dataRate: Math.round(prev.dataRate / 1024), // Convert to KB/s
        fps: videoFps
      }));
    }, 2000);

    return () => clearInterval(interval);
  }, [videoFps]);

  // --- Connection status helpers ---
  const getConnectionStatusDisplay = () => {
    switch (connectionState) {
      case ConnectionStates.CONNECTED:
        return { icon: Wifi, text: 'Connected', className: 'status-connected' };
      case ConnectionStates.CONNECTING:
        return { icon: Radio, text: 'Connecting...', className: 'status-reconnecting' };
      case ConnectionStates.RECONNECTING:
        return { icon: Radio, text: `Reconnecting (${reconnectAttempts + 1}/4)`, className: 'status-reconnecting' };
      case ConnectionStates.ERROR:
        return { icon: AlertTriangle, text: 'Error', className: 'status-disconnected' };
      default:
        return { icon: WifiOff, text: 'Disconnected', className: 'status-disconnected' };
    }
  };

  const getStatusColor = (status) => {
    switch (status) {
      case SystemStatus.HEALTHY: return 'robot-status-brain';
      case SystemStatus.WARNING: return 'connection-warning';
      case SystemStatus.CRITICAL: return 'connection-error';
      default: return 'connection-error opacity-50';
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

  // --- Waypoint management functions ---
  const addWaypoint = useCallback((x, y, name) => {
    const waypoint = {
      id: Date.now() + Math.round(Math.random() * 999),
      x,
      y,
      name: name || `WP${waypoints.length + 1}`,
      action: 'navigate'
    };
    setWaypoints(prev => [...prev, waypoint]);
    addAlert(`Waypoint added: ${waypoint.name}`, 'info');
  }, [waypoints.length, addAlert]);

  const removeWaypoint = useCallback((id) => {
    setWaypoints(prev => prev.filter(w => w.id !== id));
    addAlert('Waypoint removed', 'info');
  }, [addAlert]);

  const clearWaypoints = useCallback(() => {
    setWaypoints([]);
    setCurrentWaypoint(0);
    setIsNavigating(false);
    addAlert('All waypoints cleared', 'info');
  }, [addAlert]);

  const startMission = useCallback(() => {
    if (waypoints.length === 0) {
      addAlert('No waypoints to navigate', 'error');
      return;
    }
    setIsNavigating(true);
    setIsPaused(false);
    setCurrentWaypoint(0);
    addAlert('Mission started', 'success');
  }, [waypoints.length, addAlert]);

  const pauseMission = useCallback(() => {
    if (!isNavigating) return;
    setIsPaused(prev => !prev);
    addAlert(isPaused ? 'Mission resumed' : 'Mission paused', 'info');
  }, [isNavigating, isPaused, addAlert]);

  const stopMission = useCallback(() => {
    setIsNavigating(false);
    setIsPaused(false);
    setCurrentWaypoint(0);
    addAlert('Mission stopped', 'info');
  }, [addAlert]);

  // --- Canvas click handler ---
  const handleCanvasClick = useCallback((e) => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    
    const rect = canvas.getBoundingClientRect();
    const x = (e.clientX - rect.left) * (canvas.width / rect.width);
    const y = (e.clientY - rect.top) * (canvas.height / rect.height);
    
    addWaypoint(x, y);
  }, [addWaypoint]);

  // Mission navigation simulation
  const [isPaused, setIsPaused] = useState(false);

  useEffect(() => {
    if (!isNavigating || isPaused || waypoints.length === 0) return;

    const interval = setInterval(() => {
      if (currentWaypoint < waypoints.length) {
        const target = waypoints[currentWaypoint];
        setRobotPosition(prev => {
          const dx = target.x - prev.x;
          const dy = target.y - prev.y;
          const dist = Math.sqrt(dx * dx + dy * dy);
          
          if (dist < 10) {
            setCurrentWaypoint(curr => {
              if (curr + 1 >= waypoints.length) {
                setIsNavigating(false);
                addAlert('Mission completed!', 'success');
                return curr;
              }
              return curr + 1;
            });
            return prev;
          }
          
          const speed = 1.5;
          const r = speed / dist;
          return { x: prev.x + dx * r, y: prev.y + dy * r };
        });
      }
    }, 50);

    return () => clearInterval(interval);
  }, [isNavigating, isPaused, waypoints, currentWaypoint, addAlert]);

  // Mission map rendering
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    
    const ctx = canvas.getContext('2d');
    const w = canvas.width;
    const h = canvas.height;

    // Clear canvas
    ctx.clearRect(0, 0, w, h);

    // Background grid
    ctx.fillStyle = '#0b1220';
    ctx.fillRect(0, 0, w, h);
    ctx.strokeStyle = '#122033';
    ctx.lineWidth = 1;
    
    for (let i = 0; i <= w; i += 40) {
      ctx.beginPath();
      ctx.moveTo(i, 0);
      ctx.lineTo(i, h);
      ctx.stroke();
    }
    
    for (let i = 0; i <= h; i += 40) {
      ctx.beginPath();
      ctx.moveTo(0, i);
      ctx.lineTo(w, i);
      ctx.stroke();
    }

    // Draw path
    if (waypoints.length > 0) {
      ctx.strokeStyle = '#54a0ff';
      ctx.lineWidth = 2;
      ctx.setLineDash([6, 6]);
      ctx.beginPath();
      ctx.moveTo(robotPosition.x, robotPosition.y);
      waypoints.forEach((wp, idx) => {
        if (idx >= currentWaypoint) ctx.lineTo(wp.x, wp.y);
      });
      ctx.stroke();
      ctx.setLineDash([]);
    }

    // Draw waypoints
    waypoints.forEach((wp, idx) => {
      if (idx < currentWaypoint) {
        ctx.fillStyle = '#27ae60';
        ctx.strokeStyle = '#27ae60';
      } else if (idx === currentWaypoint) {
        ctx.fillStyle = '#f39c12';
        ctx.strokeStyle = '#f39c12';
      } else {
        ctx.fillStyle = '#3498db';
        ctx.strokeStyle = '#3498db';
      }
      
      ctx.lineWidth = 3;
      ctx.beginPath();
      ctx.arc(wp.x, wp.y, 8, 0, Math.PI * 2);
      ctx.fill();
      ctx.stroke();

      // Waypoint labels
      ctx.fillStyle = '#fff';
      ctx.font = 'bold 10px Arial';
      ctx.textAlign = 'center';
      ctx.textBaseline = 'middle';
      ctx.fillText((idx + 1).toString(), wp.x, wp.y);
    });

    // Draw robot
    ctx.save();
    ctx.translate(robotPosition.x, robotPosition.y);
    ctx.fillStyle = '#e74c3c';
    ctx.beginPath();
    ctx.arc(0, 0, 12, 0, Math.PI * 2);
    ctx.fill();

    // Robot direction indicator
    if (waypoints.length > currentWaypoint) {
      const target = waypoints[currentWaypoint];
      const angle = Math.atan2(target.y - robotPosition.y, target.x - robotPosition.x);
      ctx.rotate(angle);
      ctx.fillStyle = '#fff';
      ctx.beginPath();
      ctx.moveTo(12, 0);
      ctx.lineTo(6, -4);
      ctx.lineTo(6, 4);
      ctx.closePath();
      ctx.fill();
    }
    ctx.restore();

  }, [waypoints, robotPosition, currentWaypoint]);

  const connectionStatus = getConnectionStatusDisplay();
  const StatusIcon = connectionStatus.icon;

  // Show loading state
  if (isLoading) {
    return <DashboardLoader />;
  }

  return (
    <div className="min-h-screen bg-gradient-to-br from-nightfall-primary via-nightfall-secondary to-nightfall-primary text-white">
      {/* Settings Panel */}
      <SettingsPanel
        isOpen={showSettings}
        onClose={() => setShowSettings(false)}
        settings={settings}
        onSettingsChange={(newSettings) => {
          setSettings(newSettings);
          localStorage.setItem('dashboard-settings', JSON.stringify(newSettings));
        }}
      />

      {/* Fullscreen Video */}
      <FullscreenVideo
        isOpen={showFullscreenVideo}
        onClose={() => setShowFullscreenVideo(false)}
        cameraIp={telemetry.cam_ip}
        cameraStatus={telemetry.camera_status}
      />

      {/* Header */}
      <header className="card p-6 m-6 animate-slide-up">
        <div className="flex items-center justify-between flex-wrap gap-4">
          <div className="flex items-center gap-4">
            <div className="bg-nightfall-accent p-3 rounded-lg glow">
              <Radio className="w-8 h-8 text-white" />
            </div>
            <div>
              <h1 className="text-3xl font-bold text-shadow">Project Nightfall — Mission Control</h1>
              <p className="text-gray-400 text-sm">Enhanced rescue robot control dashboard v2.0</p>
            </div>
          </div>

          <div className="flex items-center gap-4 flex-wrap">
            <div className={`status-indicator ${connectionStatus.className}`}>
              <StatusIcon className="w-5 h-5 mr-2" />
              {connectionStatus.text}
            </div>
            
            <div className="bg-gray-700 px-4 py-2 rounded-lg flex items-center gap-2">
              <Activity className="w-5 h-5 text-blue-400" />
              <span className="font-medium">{robotState}</span>
            </div>
            
            {connectionStats.averageLatency > 0 && (
              <Tooltip content="Average network latency">
                <div className="bg-gray-700 px-3 py-2 rounded-lg text-sm">
                  <div className="text-gray-400">Latency</div>
                  <div className="font-mono">{connectionStats.averageLatency}ms</div>
                </div>
              </Tooltip>
            )}

            <Tooltip content="Dashboard Settings">
              <button
                onClick={() => setShowSettings(true)}
                className="p-2 bg-gray-700 hover:bg-gray-600 rounded-lg transition-colors"
                aria-label="Open settings"
              >
                <Settings className="w-5 h-5" />
              </button>
            </Tooltip>

            <Tooltip content={showAnalytics ? "Hide Analytics" : "Show Analytics"}>
              <button
                onClick={() => setShowAnalytics(!showAnalytics)}
                className={`p-2 rounded-lg transition-colors ${
                  showAnalytics 
                    ? 'bg-nightfall-accent text-white' 
                    : 'bg-gray-700 hover:bg-gray-600'
                }`}
                aria-label="Toggle analytics"
              >
                <BarChart3 className="w-5 h-5" />
              </button>
            </Tooltip>
          </div>
        </div>
      </header>
                <div className="text-gray-400">Latency</div>
                <div className="font-mono">{connectionStats.averageLatency}ms</div>
              </div>
            )}
          </div>
        </div>
      </header>

      {/* Three-Zone Layout */}
      <div className="px-6 pb-6">
        {/* Analytics Section (Optional) */}
        {showAnalytics && (
          <div className="mb-6 animate-slide-up">
            <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
              <TelemetryChart
                data={batteryHistory}
                title="Battery Voltage"
                color="#10b981"
                icon={Battery}
                unit="V"
              />
              <TelemetryChart
                data={gasHistory}
                title="Gas Level"
                color="#f59e0b"
                icon={Wind}
                unit=""
              />
              <TelemetryChart
                data={distanceHistory}
                title="Distance"
                color="#3b82f6"
                icon={Gauge}
                unit="cm"
              />
              <TelemetryChart
                data={signalHistory}
                title="WiFi Signal"
                color="#8b5cf6"
                icon={Signal}
                unit="%"
              />
            </div>
          </div>
        )}

        <div className="grid grid-cols-1 xl:grid-cols-12 gap-6">
          
          {/* Zone A: Video & Vision Processing (60% width) */}
          <div className="xl:col-span-7 space-y-6 animate-slide-up">
            <div className="card p-6 h-full card-hover">
              <div className="flex items-center justify-between mb-4">
                <h2 className="text-xl font-semibold flex items-center gap-3">
                  <Eye className="w-6 h-6 text-robot-vision" />
                  Live Vision Feed
                </h2>
                <div className="flex items-center gap-4">
                  <div className={`flex items-center gap-2 ${telemetry.camera_status ? 'text-green-400' : 'text-red-400'}`}>
                    <Circle className="w-3 h-3 fill-current animate-pulse" />
                    <span className="text-sm">{telemetry.camera_status ? 'Streaming' : 'Offline'}</span>
                  </div>
                  <div className="text-sm text-gray-400">
                    FPS: {videoFps}
                  </div>
                  <Tooltip content="Fullscreen Mode">
                    <button
                      onClick={() => setShowFullscreenVideo(true)}
                      className="p-2 bg-gray-700 hover:bg-gray-600 rounded-lg transition-colors"
                      aria-label="Fullscreen"
                    >
                      <Maximize2 className="w-4 h-4" />
                    </button>
                  </Tooltip>
                </div>
              </div>

              <div className="bg-black rounded-lg overflow-hidden aspect-video border-2 border-gray-700 relative group">
                {telemetry.camera_status ? (
                  <>
                    <img 
                      src={`http://${telemetry.cam_ip}:81/stream`}
                      alt="Robot camera feed"
                      className="w-full h-full object-cover"
                      onLoad={() => fpsCounterRef.current.frames++}
                      onError={() => addAlert('Camera stream error', 'error')}
                    />
                    
                    {/* HUD Overlay */}
                    <div className="hud-overlay">
                      <div className="space-y-2 text-sm">
                        <div className="flex justify-between items-center">
                          <span className="text-gray-300">Distance:</span>
                          <span className={`font-mono font-bold ${getDistanceColor(telemetry.dist)}`}>
                            {Math.round(telemetry.dist)}cm
                          </span>
                        </div>
                        <div className="flex justify-between items-center">
                          <span className="text-gray-300">Gas Level:</span>
                          <span className={`font-mono font-bold ${getGasColor(telemetry.gas)}`}>
                            {Math.round(telemetry.gas)}
                          </span>
                        </div>
                        <div className="flex justify-between items-center">
                          <span className="text-gray-300">Camera IP:</span>
                          <span className="font-mono text-blue-400">{telemetry.cam_ip}</span>
                        </div>
                        <div className="flex justify-between items-center">
                          <span className="text-gray-300">Time:</span>
                          <span className="font-mono text-gray-300">
                            {new Date().toLocaleTimeString()}
                          </span>
                        </div>
                      </div>
                    </div>

                    {/* Recording indicator */}
                    <div className="absolute top-4 left-4 bg-red-600 px-3 py-1 rounded-full flex items-center gap-2">
                      <div className="w-2 h-2 bg-white rounded-full animate-pulse"></div>
                      <span className="text-xs font-medium">REC</span>
                    </div>
                  </>
                ) : (
                  <div className="w-full h-full flex items-center justify-center">
                    <div className="text-center">
                      <Camera className="w-16 h-16 text-gray-600 mx-auto mb-4" />
                      <div className="text-gray-400 text-lg">Waiting for Camera...</div>
                      <div className="text-gray-500 text-sm mt-2">Ensure ESP32-CAM is connected</div>
                    </div>
                  </div>
                )}
              </div>
            </div>
          </div>

          {/* Zone B: System Health Dashboard (25% width) */}
          <div className="xl:col-span-3 space-y-6 animate-slide-in-right">
            <div className="card p-6 card-hover">
              <h2 className="text-xl font-semibold mb-4 flex items-center gap-3">
                <Cpu className="w-6 h-6 text-robot-brain" />
                System Health
              </h2>

              {/* Connection Status Indicators */}
              <div className="space-y-4">
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

              {/* Sensor Metrics */}
              <div className="mt-6 space-y-4">
                <h3 className="text-sm font-semibold text-gray-300">Real-time Sensors</h3>
                
                {/* Battery Level */}
                <div className="space-y-2">
                  <div className="flex justify-between items-center">
                    <span className="text-sm text-gray-400 flex items-center gap-2">
                      <Battery className={`w-4 h-4 ${getBatteryColor(telemetry.battery)}`} />
                      Battery
                    </span>
                    <span className={`font-mono font-bold ${getBatteryColor(telemetry.battery)}`}>
                      {telemetry.battery.toFixed(1)}V
                    </span>
                  </div>
                  <div className="metric-bar">
                    <div 
                      className={`metric-fill ${getBatteryColor(telemetry.battery).replace('text-', 'bg-')}`}
                      style={{ width: `${Math.max(0, Math.min((telemetry.battery - 11) / (14.8 - 11) * 100, 100))}%` }}
                    ></div>
                  </div>
                </div>

                {/* Signal Strength */}
                <div className="space-y-2">
                  <div className="flex justify-between items-center">
                    <span className="text-sm text-gray-400 flex items-center gap-2">
                      <Signal className="w-4 h-4" />
                      WiFi Signal
                    </span>
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

                {/* Gas Level */}
                <div className="space-y-2">
                  <div className="flex justify-between items-center">
                    <span className="text-sm text-gray-400 flex items-center gap-2">
                      <Wind className="w-4 h-4" />
                      Gas Level
                    </span>
                    <span className={`font-mono font-bold ${getGasColor(telemetry.gas)}`}>
                      {Math.round(telemetry.gas)}
                    </span>
                  </div>
                  <div className="metric-bar">
                    <div 
                      className={`metric-fill ${getGasColor(telemetry.gas).replace('text-', 'bg-')}`}
                      style={{ width: `${Math.min(telemetry.gas / 4095 * 100, 100)}%` }}
                    ></div>
                  </div>
                </div>

                {/* System Uptime */}
                <div className="space-y-2">
                  <div className="flex justify-between items-center">
                    <span className="text-sm text-gray-400 flex items-center gap-2">
                      <Clock className="w-4 h-4" />
                      Uptime
                    </span>
                    <span className="font-mono font-bold text-green-400">
                      {Math.floor(telemetry.uptime / 3600)}h {Math.floor((telemetry.uptime % 3600) / 60)}m
                    </span>
                  </div>
                </div>
              </div>

              {/* Performance Metrics */}
              <div className="mt-6 pt-4 border-t border-gray-700">
                <h3 className="text-sm font-semibold text-gray-300 mb-3">Performance</h3>
                <div className="grid grid-cols-2 gap-3 text-xs">
                  <div className="bg-gray-800 p-2 rounded">
                    <div className="text-gray-400">Data Rate</div>
                    <div className="font-mono text-green-400">{performanceMetrics.dataRate} KB/s</div>
                  </div>
                  <div className="bg-gray-800 p-2 rounded">
                    <div className="text-gray-400">Success Rate</div>
                    <div className="font-mono text-blue-400">{performanceMetrics.commandSuccessRate}%</div>
                  </div>
                  <div className="bg-gray-800 p-2 rounded">
                    <div className="text-gray-400">Messages</div>
                    <div className="font-mono text-purple-400">{connectionStats.messagesReceived}</div>
                  </div>
                  <div className="bg-gray-800 p-2 rounded">
                    <div className="text-gray-400">Latency</div>
                    <div className="font-mono text-yellow-400">{connectionStats.averageLatency}ms</div>
                  </div>
                </div>
              </div>

              {/* Data Export */}
              <div className="mt-6 pt-4 border-t border-gray-700">
                <DataExportPanel
                  telemetry={telemetry}
                  waypoints={waypoints}
                  alerts={alerts}
                  connectionStats={connectionStats}
                />
              </div>
            </div>
          </div>

          {/* Zone C: Command & Control Interface (40% width) */}
          <div className="xl:col-span-2 space-y-6 animate-slide-in-right" style={{ animationDelay: '0.1s' }}>
            <div className="card p-6 card-hover">
              <h2 className="text-xl font-semibold mb-4 flex items-center gap-3">
                <Zap className="w-6 h-6 text-nightfall-accent" />
                Control Interface
              </h2>

              {/* Mode Toggle */}
              <div className="mb-6">
                <Tooltip content={mode === 'autonomous' ? 'Switch to manual control' : 'Switch to autonomous mode'}>
                  <button 
                    onClick={toggleMode}
                    className={`w-full p-4 rounded-lg font-semibold transition-all duration-200 ${
                      mode === 'autonomous' 
                        ? 'bg-nightfall-success hover:bg-nightfall-success/90 text-white glow-green' 
                        : 'bg-gray-700 hover:bg-gray-600 text-white'
                    }`}
                  >
                    {mode === 'autonomous' ? '🤖 Autonomous Mode' : '🎮 Manual Mode'}
                  </button>
                </Tooltip>
              </div>

              {/* Movement Controls */}
              <div className="mb-6">
                <h3 className="text-sm font-semibold text-gray-300 mb-3">Movement Controls</h3>
                <div className="control-grid">
                  <div></div>
                  <button 
                    onClick={moveForward}
                    disabled={mode === 'autonomous'}
                    className="control-btn-forward disabled:opacity-50 disabled:cursor-not-allowed"
                    aria-label="Move forward"
                  >
                    <ArrowUp className="w-6 h-6 mx-auto" />
                  </button>
                  <div></div>

                  <button 
                    onClick={turnLeft}
                    disabled={mode === 'autonomous'}
                    className="control-btn-left disabled:opacity-50 disabled:cursor-not-allowed"
                    aria-label="Turn left"
                  >
                    <ArrowLeft className="w-6 h-6 mx-auto" />
                  </button>

                  <button 
                    onClick={() => sendMotorCommand(0, 0)}
                    className="control-btn-stop"
                    aria-label="Stop"
                  >
                    <StopCircle className="w-6 h-6 mx-auto" />
                  </button>

                  <button 
                    onClick={turnRight}
                    disabled={mode === 'autonomous'}
                    className="control-btn-right disabled:opacity-50 disabled:cursor-not-allowed"
                    aria-label="Turn right"
                  >
                    <ArrowRight className="w-6 h-6 mx-auto" />
                  </button>

                  <div></div>
                  <button 
                    onClick={moveBackward}
                    disabled={mode === 'autonomous'}
                    className="control-btn-back disabled:opacity-50 disabled:cursor-not-allowed"
                    aria-label="Move backward"
                  >
                    <ArrowDown className="w-6 h-6 mx-auto" />
                  </button>

                  <div></div>
                </div>

                <div className="mt-4">
                  <button 
                    onClick={rotateClockwise}
                    disabled={mode === 'autonomous'}
                    className="control-btn-rotate w-full disabled:opacity-50 disabled:cursor-not-allowed"
                    aria-label="Rotate clockwise"
                  >
                    <RotateCw className="w-5 h-5 mx-auto mr-2" />
                    Rotate
                  </button>
                </div>
              </div>

              {/* Emergency Stop */}
              <div className="mb-6">
                <Tooltip content="Immediately stop all robot movement (Space/Esc)">
                  <button 
                    onClick={emergencyStop}
                    className="emergency-stop w-full flex flex-col items-center justify-center"
                    aria-label="Emergency stop"
                  >
                    <Power className="w-8 h-8 mb-1" />
                    <span className="text-xs font-bold">EMERGENCY</span>
                  </button>
                </Tooltip>
              </div>

              {/* Current Motor State */}
              <div className="bg-gray-800 p-4 rounded-lg">
                <h3 className="text-sm font-semibold text-gray-300 mb-3">Motor Status</h3>
                <div className="space-y-2 text-sm">
                  <div className="flex justify-between">
                    <span className="text-gray-400">Left Motor:</span>
                    <span className="font-mono font-bold text-blue-400">{motorState.left}</span>
                  </div>
                  <div className="w-full bg-gray-700 rounded-full h-2">
                    <div 
                      className="h-2 rounded-full bg-blue-500 transition-all"
                      style={{ width: `${Math.min(Math.abs(motorState.left) / 255 * 100, 100)}%` }}
                    ></div>
                  </div>
                  
                  <div className="flex justify-between">
                    <span className="text-gray-400">Right Motor:</span>
                    <span className="font-mono font-bold text-blue-400">{motorState.right}</span>
                  </div>
                  <div className="w-full bg-gray-700 rounded-full h-2">
                    <div 
                      className="h-2 rounded-full bg-blue-500 transition-all"
                      style={{ width: `${Math.min(Math.abs(motorState.right) / 255 * 100, 100)}%` }}
                    ></div>
                  </div>
                </div>
              </div>

              {/* Keyboard Shortcuts */}
              <div className="mt-6 p-4 bg-gray-800 rounded-lg">
                <h3 className="text-sm font-semibold text-gray-300 mb-3">Keyboard Shortcuts</h3>
                <div className="space-y-1 text-xs text-gray-400">
                  <div className="flex justify-between">
                    <span>W / ↑</span>
                    <span>Forward</span>
                  </div>
                  <div className="flex justify-between">
                    <span>S / ↓</span>
                    <span>Backward</span>
                  </div>
                  <div className="flex justify-between">
                    <span>A / ←</span>
                    <span>Turn Left</span>
                  </div>
                  <div className="flex justify-between">
                    <span>D / →</span>
                    <span>Turn Right</span>
                  </div>
                  <div className="flex justify-between">
                    <span>Space / Esc</span>
                    <span>Emergency Stop</span>
                  </div>
                  <div className="flex justify-between">
                    <span>Ctrl + R</span>
                    <span>Rotate</span>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>

        {/* Mission Planning Section */}
        <div className="mt-6 grid grid-cols-1 xl:grid-cols-12 gap-6 animate-slide-up" style={{ animationDelay: '0.2s' }}>
          <div className="xl:col-span-7">
            <div className="card p-6 card-hover">
              <div className="flex items-center justify-between mb-4">
                <h2 className="text-xl font-semibold flex items-center gap-3">
                  <MapPin className="w-6 h-6 text-blue-400" />
                  Mission Map & Waypoints
                </h2>
                <div className={`px-3 py-1 rounded-full text-sm font-medium ${
                  isNavigating ? 'bg-nightfall-success text-white animate-pulse-glow' : 'bg-gray-700 text-gray-300'
                }`}>
                  {isNavigating ? '🟢 Mission Active' : '⚫ Mission Idle'}
                </div>
              </div>

              <div className="bg-[#071322] rounded-lg border border-gray-700 overflow-hidden">
                <canvas 
                  ref={canvasRef} 
                  width={800} 
                  height={360} 
                  className="w-full cursor-crosshair" 
                  onClick={handleCanvasClick}
                />
              </div>

              <p className="text-sm text-gray-400 mt-3 flex items-center gap-2">
                <Info className="w-4 h-4" />
                Click on the map to add waypoints. The robot will navigate them in sequence during autonomous mode.
              </p>

              {/* Mission Controls */}
              <div className="grid grid-cols-2 md:grid-cols-4 gap-3 mt-4">
                <button 
                  onClick={startMission}
                  disabled={waypoints.length === 0 || isNavigating}
                  className="btn-primary disabled:opacity-50 disabled:cursor-not-allowed flex items-center justify-center gap-2"
                >
                  <Play className="w-4 h-4" /> Start
                </button>

                <button 
                  onClick={pauseMission}
                  disabled={!isNavigating}
                  className="btn-secondary disabled:opacity-50 disabled:cursor-not-allowed flex items-center justify-center gap-2"
                >
                  <Pause className="w-4 h-4" /> {isPaused ? 'Resume' : 'Pause'}
                </button>

                <button 
                  onClick={stopMission}
                  disabled={!isNavigating}
                  className="btn-danger disabled:opacity-50 disabled:cursor-not-allowed flex items-center justify-center gap-2"
                >
                  <Target className="w-4 h-4" /> Stop
                </button>

                <button 
                  onClick={clearWaypoints}
                  className="btn-secondary flex items-center justify-center gap-2"
                >
                  <Trash2 className="w-4 h-4" /> Clear
                </button>
              </div>
            </div>
          </div>

          <div className="xl:col-spa card-hovern-5">
            <div className="card p-6">
              <h2 className="text-xl font-semibold mb-4 flex items-center gap-3">
                <Navigation className="w-6 h-6 text-purple-400" />
                Waypoint Management ({waypoints.length})
              </h2>

              <div className="max-h-80 overflow-y-auto space-y-2">
                {waypoints.length === 0 ? (
                  <div className="text-center text-gray-500 py-8">
                    <MapPin className="w-12 h-12 mx-auto mb-3 opacity-50" />
                    <div className="text-lg">No waypoints defined</div>
                    <div className="text-sm text-gray-400">Click on the mission map to add waypoints</div>
                  </div>
                ) : (
                  waypoints.map((wp, idx) => (
                    <div 
                      key={wp.id} 
                      className={`p-3 rounded-lg border-l-4 transition-all ${
                        idx < currentWaypoint 
                          ? 'bg-nightfall-success/20 border-nightfall-success' 
                          : idx === currentWaypoint 
                          ? 'bg-nightfall-warning/20 border-nightfall-warning' 
                          : 'bg-gray-700 border-blue-500'
                      }`}
                    >
                      <div className="flex items-center justify-between">
                        <div className="flex items-center gap-3">
                          {idx < currentWaypoint ? (
                            <CheckCircle className="w-5 h-5 text-nightfall-success" />
                          ) : idx === currentWaypoint ? (
                            <Navigation className="w-5 h-5 text-nightfall-warning animate-pulse" />
                          ) : (
                            <Circle className="w-5 h-5 text-blue-400" />
                          )}
                          <div>
                            <div className="font-medium">{wp.name}</div>
                            <div className="text-xs text-gray-400">
                              X: {Math.round(wp.x)}, Y: {Math.round(wp.y)}
                            </div>
                          </div>
                        </div>
                        <button 
                          onClick={() => removeWaypoint(wp.id)}
                          disabled={isNavigating}
                          className="text-red-400 hover:text-red-300 disabled:opacity-50 disabled:cursor-not-allowed p-1"
                        >
                          <Trash2 className="w-4 h-4" />
                        </button>
                      </div>
                    </div>
                  ))
                )}
              </div>

              {/* Mission Actions */}
              <div className="mt-6 space-y-3">
                <div className="grid grid-cols-2 gap-3">
                  <button 
                    onClick={() => {
                      // Quick mission patterns
                      const centerX = 400, centerY = 180, size = 100;
                      clearWaypoints();
                      addWaypoint(centerX - size, centerY - size, 'WP1');
                      addWaypoint(centerX + size, centerY - size, 'WP2');
                      addWaypoint(centerX + size, centerY + size, 'WP3');
                      addWaypoint(centerX - size, centerY + size, 'WP4');
                    }}
                    className="btn-secondary text-sm"
                  >
                    Square Pattern
                  </button>
                  
                  <button 
                    onClick={() => {
                      // Circular pattern
                      const centerX = 400, centerY = 180, radius = 120;
                      clearWaypoints();
                      for (let i = 0; i < 6; i++) {
                        const angle = (i / 6) * Math.PI * 2;
                        addWaypoint(
                          centerX + Math.cos(angle) * radius,
                          centerY + Math.sin(angle) * radius,
                          `WP${i + 1}`
                        );
                      }
                    }}
                    className="btn-secondary text-sm"
                  >
                    Circle Pattern
                  </button>
                </div>
              </div>
            </div>

            {/* Alerts Panel */}
            <div className="card p-6 mt-6 card-hover">
              <h3 className="text-lg font-semibold mb-4 flex items-center gap-2">
                <AlertTriangle className="w-5 h-5 text-nightfall-warning" />
                System Alerts ({alerts.length})
              </h3>
              
              <div className="max-h-48 overflow-y-auto space-y-2">
                {alerts.length === 0 ? (
                  <div className="text-center text-gray-500 py-4">
                    <CheckCircle className="w-8 h-8 mx-auto mb-2 opacity-50" />
                    <div className="text-sm">No active alerts</div>
                  </div>
                ) : (
                  alerts.map(alert => (
                    <div 
                      key={alert.id} 
                      className={`p-3 rounded-lg text-sm ${
                        alert.type === 'error' 
                          ? 'bg-nightfall-error/20 border border-nightfall-error/30 text-nightfall-error' 
                          : alert.type === 'warning' 
                          ? 'bg-nightfall-warning/20 border border-nightfall-warning/30 text-nightfall-warning'
                          : 'bg-nightfall-accent/20 border border-nightfall-accent/30 text-nightfall-accent'
                      }`}
                    >
                      <div className="flex items-start gap-2">
                        <AlertCircle className="w-4 h-4 mt-0.5 flex-shrink-0" />
                        <div className="flex-1">
                          <div>{alert.message}</div>
                          <div className="text-xs opacity-75 mt-1">
                            {alert.timestamp.toLocaleTimeString()}
                          </div>
                        </div>
                      </div>
                    </div>
                  ))
                )}
              </div>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}