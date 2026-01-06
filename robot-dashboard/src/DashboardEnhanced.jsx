import React, { useEffect, useRef, useState, useCallback, useMemo } from 'react';
import {
  Camera, Radio, Gauge, Battery, AlertTriangle, Wind,
  Wifi, ArrowUp, ArrowDown, ArrowLeft, ArrowRight,
  StopCircle, Eye, Zap, Signal, Settings, Maximize2, AlertCircle,
  Bot, Clock, Volume2, VolumeX, Keyboard, X, WifiOff
} from 'lucide-react';

import { DashboardLoader } from './components/LoadingSkeleton';
import TelemetryChart, { useTelemetryHistory } from './components/TelemetryChart';
import SettingsPanel from './components/SettingsPanel';
import SystemHealthPanel from './components/SystemHealthPanel';
import FullscreenVideo from './components/FullscreenVideo';
import DataExportPanel from './components/DataExportPanel';
import MjpegVideo from './components/MjpegVideo';

const WEBSOCKET_URL = 'ws://192.168.4.1:8888';
const DEFAULT_CAM_IP = '192.168.4.2'; // Fallback IP

const ConnectionStates = { DISCONNECTED: 'disconnected', CONNECTING: 'connecting', CONNECTED: 'connected', ERROR: 'error' };
const SystemStatus = { HEALTHY: 'healthy', WARNING: 'warning', CRITICAL: 'critical', OFFLINE: 'offline' };

// --- MEMOIZED ALERT COMPONENT ---
const AlertContainer = React.memo(function AlertContainer({ alerts, onDismiss }) {
  return (
    <div className="fixed bottom-6 right-6 flex flex-col gap-2 z-50 max-w-sm w-full pointer-events-auto">
      {alerts.map(alert => (
        <div key={alert.id} className={`p-4 rounded-xl shadow-2xl flex items-center justify-between gap-3 transform transition-all duration-300 ${
          alert.type === 'error' ? 'bg-red-600 text-white' : 
          alert.type === 'success' ? 'bg-emerald-600 text-white' : 'bg-gray-800 text-gray-200 border border-gray-700'
        }`}>
          <div className="flex items-center gap-3">
            {alert.type === 'error' ? <AlertTriangle className="w-5 h-5 animate-pulse"/> : <div className="w-2 h-2 rounded-full bg-white"/>}
            <div>
              <span className="font-bold text-sm block">{alert.message}</span>
              <span className="text-xs opacity-75">{new Date(alert.timestamp).toLocaleTimeString()}</span>
            </div>
          </div>
          {alert.count > 1 && <span className="bg-white/20 px-2 py-0.5 rounded text-xs font-mono">x{alert.count}</span>}
          <button onClick={() => onDismiss(alert.id)} className="p-1 hover:bg-white/20 rounded"><X className="w-4 h-4"/></button>
        </div>
      ))}
    </div>
  );
});

// --- MEMOIZED HEADER COMPONENT ---
const DashboardHeader = React.memo(function DashboardHeader({ 
  connectionState, 
  autoMode, 
  soundEnabled, 
  onToggleSound,
  onOpenSettings 
}) {
  return (
    <header className="mb-6 flex flex-wrap justify-between items-center bg-gray-800/50 p-4 rounded-2xl border border-gray-700/50 shadow-xl">
      <div className="flex items-center gap-4">
        <div className="bg-blue-600 p-2 rounded-xl shadow-lg shadow-blue-500/20"><Radio className="w-6 h-6 text-white" /></div>
        <div>
          <h1 className="text-2xl font-bold bg-clip-text text-transparent bg-gradient-to-r from-blue-400 via-purple-400 to-pink-400">Project Nightfall</h1>
          <p className="text-gray-400 text-xs font-medium">Autonomous Rescue System // V3.4.0 (Stable)</p>
        </div>
      </div>
      <div className="flex gap-2 items-center mt-4 sm:mt-0">
        <button 
          onClick={onToggleSound} 
          className="p-2 bg-gray-700/50 rounded-lg transition-all hover:bg-gray-600"
          aria-label={soundEnabled ? 'Mute sound' : 'Enable sound'}
        >
          {soundEnabled ? <Volume2 className="w-5 h-5"/> : <VolumeX className="w-5 h-5"/>}
        </button>
        
        {autoMode && (
          <div className="flex items-center gap-2 px-3 py-1.5 rounded-lg border border-purple-500/50 text-purple-400 bg-purple-500/10 animate-pulse">
            <Bot className="w-4 h-4"/>
            <span className="text-sm font-bold">AUTO</span>
          </div>
        )}
        
        <div className={`flex items-center gap-2 px-3 py-1.5 rounded-lg border transition-colors ${
          connectionState === ConnectionStates.CONNECTED 
            ? 'text-emerald-400 bg-emerald-400/10 border-emerald-500/30' 
            : 'text-red-400 bg-red-400/10 border-red-500/30'
        }`}>
          <Wifi className="w-4 h-4" />
          <span className="text-sm font-semibold capitalize">{connectionState}</span>
        </div>
        
        <button 
          onClick={onOpenSettings} 
          className="p-2 bg-gray-700/50 rounded-lg hover:bg-gray-600 transition-colors"
          aria-label="Open settings"
        >
          <Settings className="w-5 h-5"/>
        </button>
      </div>
    </header>
  );
});

// --- MEMOIZED CONTROL PANEL COMPONENT ---
const ControlPanel = React.memo(function ControlPanel({ 
  autoMode, 
  emergency, 
  sendCommand 
}) {
  return (
    <div className="bg-gray-800/40 p-6 rounded-2xl border border-gray-700/50 shadow-xl flex-1 flex flex-col justify-center">
      <div className="flex justify-between items-center mb-6">
        <h2 className="text-lg font-semibold flex items-center gap-2">
          <Zap className="w-5 h-5 text-yellow-400"/>
          Controls
        </h2>
        <button 
          onClick={() => sendCommand('auto_toggle')} 
          className={`px-4 py-2 rounded-lg font-bold text-xs flex gap-2 transition-all ${
            autoMode 
              ? 'bg-purple-600 text-white animate-pulse' 
              : 'bg-gray-700 text-gray-300 hover:bg-gray-600'
          }`}
        >
          <Bot className="w-4 h-4"/> 
          {autoMode ? 'DISABLE AUTO' : 'ENABLE AUTO'}
        </button>
      </div>
      
      <div className="grid grid-cols-3 gap-3 max-w-[220px] mx-auto mb-6 will-change-transform">
        <div></div>
        <button 
          onMouseDown={() => sendCommand('forward')} 
          onMouseUp={() => sendCommand('stop')} 
          onMouseLeave={() => sendCommand('stop')}
          className="h-14 bg-blue-600 rounded-xl flex items-center justify-center text-white shadow-lg active:scale-95 transition-all hover:bg-blue-500"
        >
          <ArrowUp className="w-6 h-6"/>
        </button>
        <div></div>
        
        <button 
          onMouseDown={() => sendCommand('left')} 
          onMouseUp={() => sendCommand('stop')}
          onMouseLeave={() => sendCommand('stop')}
          className="h-14 bg-gray-700 rounded-xl flex items-center justify-center text-white active:scale-95 transition-all hover:bg-gray-600"
        >
          <ArrowLeft className="w-6 h-6"/>
        </button>
        <button 
          onClick={() => sendCommand('stop')} 
          className="h-14 bg-red-900/20 border border-red-500/50 text-red-500 rounded-xl flex items-center justify-center active:scale-95 transition-all hover:bg-red-900/30"
        >
          <StopCircle className="w-6 h-6"/>
        </button>
        <button 
          onMouseDown={() => sendCommand('right')} 
          onMouseUp={() => sendCommand('stop')}
          onMouseLeave={() => sendCommand('stop')}
          className="h-14 bg-gray-700 rounded-xl flex items-center justify-center text-white active:scale-95 transition-all hover:bg-gray-600"
        >
          <ArrowRight className="w-6 h-6"/>
        </button>
        
        <div></div>
        <button 
          onMouseDown={() => sendCommand('backward')} 
          onMouseUp={() => sendCommand('stop')}
          onMouseLeave={() => sendCommand('stop')}
          className="h-14 bg-gray-700 rounded-xl flex items-center justify-center text-white active:scale-95 transition-all hover:bg-gray-600"
        >
          <ArrowDown className="w-6 h-6"/>
        </button>
        <div></div>
      </div>

      <button 
        onClick={() => sendCommand('emergency')} 
        className={`w-full py-3 rounded-xl font-bold tracking-widest transition-all ${
          emergency 
            ? 'bg-yellow-500 text-black animate-pulse' 
            : 'bg-red-600 hover:bg-red-500 text-white shadow-lg shadow-red-900/30'
        }`}
      >
        {emergency ? 'RESET SYSTEM' : 'EMERGENCY STOP'}
      </button>
    </div>
  );
});

// --- MAIN DASHBOARD ---
export default function DashboardEnhanced() {
  const [isLoading, setIsLoading] = useState(true);
  const [showSettings, setShowSettings] = useState(false);
  const [showFullscreenVideo, setShowFullscreenVideo] = useState(false);
  const [showAnalytics, setShowAnalytics] = useState(false);
  const [showShortcuts, setShowShortcuts] = useState(false);
  const [soundEnabled, setSoundEnabled] = useState(true);
  const [isUsingFallbackIp, setIsUsingFallbackIp] = useState(false);
  
  const [settings, setSettings] = useState(() => {
    try {
      return JSON.parse(localStorage.getItem('dashboard-settings')) || { theme: 'dark', maxTelemetryHistory: 50 };
    } catch {
      return { theme: 'dark', maxTelemetryHistory: 50 };
    }
  });
  
  const { batteryHistory, gasHistory, distanceHistory, addTelemetryPoint } = useTelemetryHistory(settings.maxTelemetryHistory);

  const [connectionState, setConnectionState] = useState(ConnectionStates.DISCONNECTED);
  const [connectionStats, setConnectionStats] = useState({ messagesReceived: 0, messagesSent: 0 });
  
  const [telemetry, setTelemetry] = useState({
    back_status: 'offline', front_status: false, camera_status: false,
    dist: 0, gas: 0, cam_ip: null, battery: 11.1, signal_strength: 0,
    uptime: 0, emergency: false, auto_mode: false
  });

  const [systemHealth, setSystemHealth] = useState({ brain: SystemStatus.OFFLINE, motors: SystemStatus.OFFLINE, vision: SystemStatus.OFFLINE });
  const [performanceMetrics, setPerformanceMetrics] = useState({ dataRate: 0, fps: 0 });
  const [alerts, setAlerts] = useState([]);
  const [videoFps, setVideoFps] = useState(0);

  const wsRef = useRef(null);
  const fpsCounterRef = useRef({ frames: 0, lastTime: Date.now() });
  const lastCommandTimeRef = useRef(0);
  const audioContextRef = useRef(null);

  // Memoized camera URL
  const cameraStreamUrl = useMemo(() => {
    const ip = isUsingFallbackIp ? DEFAULT_CAM_IP : (telemetry.cam_ip || DEFAULT_CAM_IP);
    return `http://${ip}/stream`;
  }, [telemetry.cam_ip, isUsingFallbackIp]);

  // --- SOUND ENGINE (Lazy Load) ---
  const playSound = useCallback((type) => {
    if (!soundEnabled) return;
    if (!audioContextRef.current) audioContextRef.current = new (window.AudioContext || window.webkitAudioContext)();
    
    const ctx = audioContextRef.current;
    if (ctx.state === 'suspended') ctx.resume();

    const osc = ctx.createOscillator();
    const gain = ctx.createGain();
    osc.connect(gain); gain.connect(ctx.destination);
    
    const now = ctx.currentTime;
    if (type === 'alert') {
      osc.type = 'sawtooth'; osc.frequency.setValueAtTime(800, now); osc.frequency.exponentialRampToValueAtTime(100, now + 0.3);
      gain.gain.setValueAtTime(0.1, now); gain.gain.linearRampToValueAtTime(0, now + 0.3);
      osc.start(now); osc.stop(now + 0.3);
    } else if (type === 'click') {
      osc.frequency.setValueAtTime(600, now); gain.gain.setValueAtTime(0.05, now);
      osc.start(now); osc.stop(now + 0.05);
    }
  }, [soundEnabled]);

  // --- SMART ALERT SYSTEM ---
  const addAlert = useCallback((message, type = 'info') => {
    setAlerts(prev => {
      const existing = prev.find(a => a.message === message);
      if (existing) {
        if (Date.now() - existing.timestamp < 1000) return prev;
        return prev.map(a => a.id === existing.id ? { ...a, count: a.count + 1, timestamp: Date.now(), type } : a);
      }
      if (type === 'error') playSound('alert');
      return [{ id: Date.now(), message, type, timestamp: Date.now(), count: 1 }, ...prev].slice(0, 3);
    });
  }, [playSound]);

  const dismissAlert = useCallback((id) => {
    setAlerts(prev => prev.filter(a => a.id !== id));
  }, []);

  // --- AUTO DISCOVERY FALLBACK ---
  useEffect(() => {
    if (connectionState === ConnectionStates.CONNECTED && !telemetry.cam_ip) {
      const timer = setTimeout(() => {
        setTelemetry(t => ({ ...t, cam_ip: DEFAULT_CAM_IP, camera_status: true }));
        setSystemHealth(s => ({ ...s, vision: SystemStatus.WARNING }));
        setIsUsingFallbackIp(true);
        addAlert("Camera Auto-Linked (Fallback IP: " + DEFAULT_CAM_IP + ")", "info");
      }, 4000);
      return () => clearTimeout(timer);
    }
  }, [connectionState, telemetry.cam_ip, addAlert]);

  // --- WEBSOCKET CONNECTION ---
  useEffect(() => {
    setTimeout(() => setIsLoading(false), 1000);

    const connect = () => {
      if (wsRef.current?.readyState === WebSocket.OPEN) return;
      setConnectionState(ConnectionStates.CONNECTING);
      
      const ws = new WebSocket(WEBSOCKET_URL);
      wsRef.current = ws;

      ws.onopen = () => {
        setConnectionState(ConnectionStates.CONNECTED);
        addAlert('Connected to Robot Brain', 'success');
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          setConnectionStats(p => ({ ...p, messagesReceived: p.messagesReceived + 1 }));

          if (data.type === 'cam_telemetry') {
            setIsUsingFallbackIp(false);
            setTelemetry(prev => ({ ...prev, cam_ip: data.ip, signal_strength: data.rssi, camera_status: true }));
            setSystemHealth(p => ({ ...p, vision: SystemStatus.HEALTHY }));
          } 
          else if (data.d !== undefined) {
            setTelemetry(prev => {
              if (data.e && !prev.emergency) addAlert("EMERGENCY STOP ACTIVATED", 'error');
              const newData = {
                ...prev,
                dist: data.d, gas: data.g, battery: data.v, emergency: data.e,
                front_status: data.fo, auto_mode: data.auto,
                uptime: prev.uptime + 0.5
              };
              addTelemetryPoint(newData);
              return newData;
            });
            setSystemHealth(p => ({ ...p, brain: SystemStatus.HEALTHY, motors: data.fo ? SystemStatus.HEALTHY : SystemStatus.CRITICAL }));
          }
        } catch (e) {}
      };

      ws.onclose = () => {
        setConnectionState(ConnectionStates.DISCONNECTED);
        setSystemHealth({ brain: SystemStatus.OFFLINE, motors: SystemStatus.OFFLINE, vision: SystemStatus.OFFLINE });
        setTimeout(connect, 3000);
      };
      
      ws.onerror = () => { setConnectionState(ConnectionStates.ERROR); ws.close(); };
    };

    connect();
    return () => wsRef.current?.close();
  }, []);

  // --- COMMANDS ---
  const sendCommand = useCallback((cmd) => {
    if (wsRef.current?.readyState !== WebSocket.OPEN) return;
    if (Date.now() - lastCommandTimeRef.current < 100) return;
    wsRef.current.send(JSON.stringify({ command: cmd }));
    lastCommandTimeRef.current = Date.now();
    playSound('click');
  }, [playSound]);

  useEffect(() => {
    const down = (e) => {
      if(e.repeat) return;
      const k = e.key.toLowerCase();
      if(k==='w') sendCommand('forward');
      if(k==='s') sendCommand('backward');
      if(k==='a') sendCommand('left');
      if(k==='d') sendCommand('right');
      if(k===' ') sendCommand('stop');
      if(k==='escape') sendCommand('emergency');
      if(e.key==='?') setShowShortcuts(true);
    };
    const up = (e) => { if(['w','a','s','d'].includes(e.key.toLowerCase())) sendCommand('stop'); };
    window.addEventListener('keydown', down); window.addEventListener('keyup', up);
    return () => { window.removeEventListener('keydown', down); window.removeEventListener('keyup', up); };
  }, [sendCommand]);

  // --- FPS ---
  useEffect(() => {
    const i = setInterval(() => {
      if (Date.now() - fpsCounterRef.current.lastTime >= 1000) {
        setVideoFps(fpsCounterRef.current.frames);
        fpsCounterRef.current.frames = 0;
        fpsCounterRef.current.lastTime = Date.now();
      }
    }, 1000);
    return () => clearInterval(i);
  }, []);

  // --- KEYBOARD SHORTCUT ---
  useEffect(() => {
    const handleKeyDown = (e) => {
      if (e.key === '?' || (e.key === '/' && e.shiftKey)) {
        e.preventDefault();
        setShowShortcuts(true);
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, []);

  // Handle FPS update from video component
  const handleFpsUpdate = useCallback((fps) => {
    setVideoFps(fps);
    fpsCounterRef.current.frames = fps;
  }, []);

  // Toggle sound with proper handler
  const toggleSound = useCallback(() => {
    setSoundEnabled(prev => !prev);
  }, []);

  // Handle fallback mode change
  const handleFallbackModeChange = useCallback((isFallback) => {
    setIsUsingFallbackIp(isFallback);
  }, []);

  if (isLoading) return <DashboardLoader />;

  return (
    <div className="min-h-screen bg-gradient-to-br from-gray-900 via-slate-900 to-black text-white p-4 sm:p-6 font-sans">
      <AlertContainer alerts={alerts} onDismiss={dismissAlert} />
      
      <SettingsPanel isOpen={showSettings} onClose={() => setShowSettings(false)} settings={settings} onSettingsChange={setSettings} />
      
      <FullscreenVideo 
        isOpen={showFullscreenVideo} 
        onClose={() => setShowFullscreenVideo(false)} 
        cameraIp={telemetry.cam_ip || DEFAULT_CAM_IP} 
        cameraStatus={telemetry.camera_status}
        fallbackIp={DEFAULT_CAM_IP}
      />

      <DashboardHeader 
        connectionState={connectionState}
        autoMode={telemetry.auto_mode}
        soundEnabled={soundEnabled}
        onToggleSound={toggleSound}
        onOpenSettings={() => setShowSettings(true)}
      />

      <div className="grid grid-cols-1 xl:grid-cols-12 gap-6">
        {/* VIDEO SECTION - Using proper MJPEG component */}
        <div className="xl:col-span-7 bg-black rounded-2xl overflow-hidden border border-gray-800 shadow-2xl relative group h-[400px] sm:h-[500px]">
          <MjpegVideo
            streamUrl={cameraStreamUrl}
            isActive={telemetry.camera_status}
            onFpsUpdate={handleFpsUpdate}
            showFps={true}
            fallbackIp={DEFAULT_CAM_IP}
            onFallbackModeChange={handleFallbackModeChange}
            className="w-full h-full"
            placeholderClassName="rounded-xl"
          />
          
          {/* Camera IP indicator */}
          <div className="absolute bottom-4 left-4 bg-black/60 px-3 py-1.5 rounded text-xs text-white backdrop-blur flex items-center gap-2">
            <Camera className="w-3 h-3 text-blue-400"/>
            <span>{isUsingFallbackIp ? DEFAULT_CAM_IP : (telemetry.cam_ip || 'Unknown')}</span>
            {isUsingFallbackIp && (
              <span className="text-amber-400 flex items-center gap-1">
                <WifiOff className="w-3 h-3" />
                (Fallback)
              </span>
            )}
          </div>
          
          <button 
            onClick={() => setShowFullscreenVideo(true)} 
            className="absolute top-4 right-4 p-2 bg-black/60 rounded text-white opacity-0 group-hover:opacity-100 transition-all hover:bg-black/80"
          >
            <Maximize2 className="w-5 h-5"/>
          </button>
        </div>

        {/* CONTROLS */}
        <div className="xl:col-span-5 flex flex-col gap-6">
          <ControlPanel 
            autoMode={telemetry.auto_mode}
            emergency={telemetry.emergency}
            sendCommand={sendCommand}
          />

          <div className="bg-gray-800/40 rounded-2xl border border-gray-700/50 overflow-hidden">
            <SystemHealthPanel 
              systemHealth={systemHealth} 
              telemetry={telemetry} 
              performanceMetrics={performanceMetrics} 
              connectionStats={connectionStats} 
            />
          </div>
        </div>

        {/* CHARTS */}
        {showAnalytics && (
          <div className="xl:col-span-12 grid grid-cols-1 md:grid-cols-4 gap-6 animate-slide-up">
            <TelemetryChart data={batteryHistory} title="Voltage" color="#10b981" icon={Battery} unit="V" />
            <TelemetryChart data={gasHistory} title="Gas Level" color="#f59e0b" icon={Wind} unit="" />
            <TelemetryChart data={distanceHistory} title="Distance" color="#3b82f6" icon={Gauge} unit="cm" />
            <div className="bg-gray-800 p-4 rounded-xl border border-gray-700">
              <DataExportPanel telemetry={telemetry} waypoints={[]} alerts={alerts} connectionStats={connectionStats} />
            </div>
          </div>
        )}
      </div>

      {/* Keyboard Shortcuts Modal */}
      {showShortcuts && (
        <div className="fixed inset-0 z-50 bg-black/50 flex items-center justify-center" onClick={() => setShowShortcuts(false)}>
          <div className="bg-gray-800 p-6 rounded-xl border border-gray-700 max-w-md w-full mx-4" onClick={e => e.stopPropagation()}>
            <div className="flex justify-between items-center mb-4">
              <h3 className="text-lg font-bold">Keyboard Shortcuts</h3>
              <button onClick={() => setShowShortcuts(false)} className="p-1 hover:bg-gray-700 rounded">
                <X className="w-5 h-5"/>
              </button>
            </div>
            <div className="space-y-2 text-sm">
              <div className="flex justify-between"><kbd className="bg-gray-700 px-2 py-1 rounded">W</kbd><span>Move Forward</span></div>
              <div className="flex justify-between"><kbd className="bg-gray-700 px-2 py-1 rounded">S</kbd><span>Move Backward</span></div>
              <div className="flex justify-between"><kbd className="bg-gray-700 px-2 py-1 rounded">A</kbd><span>Turn Left</span></div>
              <div className="flex justify-between"><kbd className="bg-gray-700 px-2 py-1 rounded">D</kbd><span>Turn Right</span></div>
              <div className="flex justify-between"><kbd className="bg-gray-700 px-2 py-1 rounded">Space</kbd><span>Stop</span></div>
              <div className="flex justify-between"><kbd className="bg-gray-700 px-2 py-1 rounded">Esc</kbd><span>Emergency Stop</span></div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
