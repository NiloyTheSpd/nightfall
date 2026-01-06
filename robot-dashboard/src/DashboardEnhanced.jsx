import React, { useEffect, useRef, useState, useCallback, memo } from 'react';
import {
  Camera, Radio, Gauge, Battery, AlertTriangle, Wind,
  Wifi, ArrowUp, ArrowDown, ArrowLeft, ArrowRight,
  StopCircle, Eye, Zap, Signal, Settings, Maximize2, AlertCircle,
  Bot, Volume2, VolumeX, Keyboard, X, RefreshCw
} from 'lucide-react';

import { DashboardLoader } from './components/LoadingSkeleton';
import TelemetryChart, { useTelemetryHistory } from './components/TelemetryChart';
import SettingsPanel from './components/SettingsPanel';
import SystemHealthPanel from './components/SystemHealthPanel';
import FullscreenVideo from './components/FullscreenVideo';
import DataExportPanel from './components/DataExportPanel';

const WEBSOCKET_URL = 'ws://192.168.4.1:8888';
const CAMERA_IP = '192.168.4.2';
const STREAM_URL = `http://${CAMERA_IP}/stream`;

const ConnectionStates = { DISCONNECTED: 'disconnected', CONNECTING: 'connecting', CONNECTED: 'connected', ERROR: 'error' };
const SystemStatus = { HEALTHY: 'healthy', WARNING: 'warning', CRITICAL: 'critical', OFFLINE: 'offline' };

// --- 1. MEMOIZED VIDEO PLAYER (CRITICAL: PREVENTS RE-RENDERS) ---
// This component ignores all prop changes except 'url'.
// This prevents the Dashboard from killing the camera connection when you press buttons.
const StableVideoPlayer = memo(({ url, onFps, onError }) => {
  return (
    <div className="relative w-full h-full bg-black rounded-2xl overflow-hidden border border-gray-800 shadow-2xl group">
      <img 
        src={url} 
        className="w-full h-full object-contain" 
        alt="Live Feed"
        onLoad={onFps}
        onError={onError}
        style={{ display: 'block' }} // Force display
      />
      {/* Overlay UI - Independent of Image */}
      <div className="absolute inset-0 pointer-events-none shadow-[inset_0_0_100px_rgba(0,0,0,0.5)]" />
    </div>
  );
}, (prevProps, nextProps) => prevProps.url === nextProps.url); // Only re-render if URL changes

// --- 2. ALERT COMPONENT ---
const AlertContainer = memo(({ alerts, onDismiss }) => {
  return (
    <div className="fixed bottom-6 right-6 flex flex-col gap-2 z-50 max-w-sm w-full pointer-events-auto">
      {alerts.map(alert => (
        <div key={alert.id} className={`p-4 rounded-xl shadow-2xl flex items-center justify-between gap-3 animate-slide-up ${
          alert.type === 'error' ? 'bg-red-600 text-white' : 
          alert.type === 'success' ? 'bg-emerald-600 text-white' : 'bg-gray-800 text-gray-200 border border-gray-700'
        }`}>
          <div className="flex items-center gap-3">
            {alert.type === 'error' ? <AlertTriangle className="w-5 h-5 animate-pulse"/> : <div className="w-2 h-2 rounded-full bg-white"/>}
            <div>
              <span className="font-bold text-sm block">{alert.message}</span>
              {alert.count > 1 && <span className="text-xs opacity-75">Occurred {alert.count} times</span>}
            </div>
          </div>
          <button onClick={() => onDismiss(alert.id)} className="p-1 hover:bg-white/20 rounded"><X className="w-4 h-4"/></button>
        </div>
      ))}
    </div>
  );
});

// --- 3. MAIN DASHBOARD ---
export default function DashboardEnhanced() {
  const [isLoading, setIsLoading] = useState(true);
  const [showSettings, setShowSettings] = useState(false);
  const [showFullscreenVideo, setShowFullscreenVideo] = useState(false);
  const [showAnalytics, setShowAnalytics] = useState(false);
  const [soundEnabled, setSoundEnabled] = useState(true);
  const [cameraError, setCameraError] = useState(false);
  
  const [settings, setSettings] = useState(() => JSON.parse(localStorage.getItem('dashboard-settings')) || { theme: 'dark', maxTelemetryHistory: 50 });
  const { batteryHistory, gasHistory, distanceHistory, addTelemetryPoint } = useTelemetryHistory(settings.maxTelemetryHistory);

  const [connectionState, setConnectionState] = useState(ConnectionStates.DISCONNECTED);
  const [connectionStats, setConnectionStats] = useState({ messagesReceived: 0, messagesSent: 0 });
  
  const [telemetry, setTelemetry] = useState({
    back_status: 'offline', front_status: false,
    dist: 0, gas: 0, battery: 11.1, signal_strength: 0,
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

  // --- SOUND ---
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
      osc.type = 'sawtooth'; osc.frequency.setValueAtTime(800, now); gain.gain.setValueAtTime(0.1, now); 
      osc.start(now); osc.stop(now + 0.2);
    } else if (type === 'click') {
      osc.frequency.setValueAtTime(600, now); gain.gain.setValueAtTime(0.05, now);
      osc.start(now); osc.stop(now + 0.05);
    }
  }, [soundEnabled]);

  // --- ALERTS ---
  const addAlert = useCallback((message, type = 'info') => {
    setAlerts(prev => {
      const existing = prev.find(a => a.message === message);
      if (existing) {
        if (Date.now() - existing.timestamp < 2000) return prev; 
        return prev.map(a => a.id === existing.id ? { ...a, count: a.count + 1, timestamp: Date.now() } : a);
      }
      if (type === 'error') playSound('alert');
      return [{ id: Date.now(), message, type, timestamp: Date.now(), count: 1 }, ...prev].slice(0, 3);
    });
  }, [playSound]);

  const dismissAlert = useCallback((id) => setAlerts(prev => prev.filter(a => a.id !== id)), []);

  // --- WEBSOCKET ---
  useEffect(() => {
    setTimeout(() => setIsLoading(false), 1000);
    const connect = () => {
      if (wsRef.current?.readyState === WebSocket.OPEN) return;
      setConnectionState(ConnectionStates.CONNECTING);
      
      const ws = new WebSocket(WEBSOCKET_URL);
      wsRef.current = ws;

      ws.onopen = () => {
        setConnectionState(ConnectionStates.CONNECTED);
        addAlert('Connected to Robot', 'success');
      };

      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          setConnectionStats(p => ({ ...p, messagesReceived: p.messagesReceived + 1 }));

          if (data.d !== undefined) {
            setTelemetry(prev => {
              if (data.e && !prev.emergency) addAlert("EMERGENCY STOP ACTIVATED", 'error');
              const newData = {
                ...prev, dist: data.d, gas: data.g, battery: data.v, emergency: data.e,
                front_status: data.fo, auto_mode: data.auto, uptime: prev.uptime + 0.5
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
        setTimeout(connect, 3000);
      };
      ws.onerror = () => { setConnectionState(ConnectionStates.ERROR); ws.close(); };
    };
    connect();
    return () => wsRef.current?.close();
  }, []);

  // --- CONTROLS ---
  const sendCommand = (cmd) => {
    if (wsRef.current?.readyState !== WebSocket.OPEN) return;
    if (Date.now() - lastCommandTimeRef.current < 80) return; // Throttle
    wsRef.current.send(JSON.stringify({ command: cmd }));
    lastCommandTimeRef.current = Date.now();
    playSound('click');
  };

  useEffect(() => {
    const handleKey = (e) => {
      if(e.repeat) return;
      const k = e.key.toLowerCase();
      const map = { w: 'forward', s: 'backward', a: 'left', d: 'right', ' ': 'stop', escape: 'emergency' };
      if (map[k]) sendCommand(map[k]);
    };
    const handleUp = (e) => { if(['w','a','s','d'].includes(e.key.toLowerCase())) sendCommand('stop'); };
    window.addEventListener('keydown', handleKey); window.addEventListener('keyup', handleUp);
    return () => { window.removeEventListener('keydown', handleKey); window.removeEventListener('keyup', handleUp); };
  }, []);

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

  if (isLoading) return <DashboardLoader />;

  return (
    <div className="min-h-screen bg-gradient-to-br from-gray-900 via-slate-900 to-black text-white p-4 sm:p-6 font-sans">
      <AlertContainer alerts={alerts} onDismiss={dismissAlert} />
      <SettingsPanel isOpen={showSettings} onClose={() => setShowSettings(false)} settings={settings} onSettingsChange={setSettings} />
      <FullscreenVideo isOpen={showFullscreenVideo} onClose={() => setShowFullscreenVideo(false)} cameraIp={CAMERA_IP} cameraStatus={!cameraError} />

      {/* HEADER */}
      <header className="mb-6 flex flex-wrap justify-between items-center bg-gray-800/50 p-4 rounded-2xl border border-gray-700/50 shadow-xl">
        <div className="flex items-center gap-4">
          <div className="bg-blue-600 p-2 rounded-xl"><Radio className="w-6 h-6 text-white" /></div>
          <div>
            <h1 className="text-2xl font-bold bg-clip-text text-transparent bg-gradient-to-r from-blue-400 via-purple-400 to-pink-400">Project Nightfall</h1>
            <p className="text-gray-400 text-xs font-medium">Autonomous Rescue System // V3.6.0 (Stable)</p>
          </div>
        </div>
        <div className="flex gap-2 items-center">
          {telemetry.auto_mode && <div className="flex items-center gap-2 px-3 py-1.5 rounded-lg border border-purple-500/50 text-purple-400 bg-purple-500/10 animate-pulse"><Bot className="w-4 h-4"/><span className="text-sm font-bold">AUTO</span></div>}
          <div className={`flex items-center gap-2 px-3 py-1.5 rounded-lg border ${connectionState === ConnectionStates.CONNECTED ? 'text-emerald-400 bg-emerald-400/10 border-emerald-500/30' : 'text-red-400 bg-red-400/10 border-red-500/30'}`}>
            <Wifi className="w-4 h-4" /><span className="text-sm font-semibold capitalize">{connectionState}</span>
          </div>
          <button onClick={() => setShowSettings(true)} className="p-2 bg-gray-700/50 rounded-lg"><Settings className="w-5 h-5"/></button>
        </div>
      </header>

      <div className="grid grid-cols-1 xl:grid-cols-12 gap-6">
        
        {/* VIDEO ZONE */}
        <div className="xl:col-span-7 h-[400px] sm:h-[500px] relative">
          {/* Using Memoized Player to Prevent Flicker */}
          <StableVideoPlayer 
            url={STREAM_URL} 
            onFps={() => { fpsCounterRef.current.frames++; setCameraError(false); }}
            onError={() => setCameraError(true)}
          />
          
          <div className="absolute top-4 left-4 bg-black/60 px-3 py-1.5 rounded text-xs text-white backdrop-blur flex items-center gap-2 pointer-events-none">
            <Eye className={`w-3 h-3 ${cameraError ? 'text-red-500' : 'text-blue-400'}`}/> 
            <span>{cameraError ? "NO SIGNAL" : `FPS: ${videoFps}`}</span>
          </div>
          
          <button onClick={() => setShowFullscreenVideo(true)} className="absolute top-4 right-4 p-2 bg-black/60 rounded text-white opacity-0 group-hover:opacity-100 transition-all"><Maximize2 className="w-5 h-5"/></button>
          
          {cameraError && (
            <div className="absolute inset-0 flex flex-col items-center justify-center bg-black/80 text-gray-400 gap-2">
              <Camera className="w-12 h-12 opacity-50"/>
              <p>Signal Lost</p>
              <button 
                onClick={() => {
                  setCameraError(false);
                  const img = document.querySelector('img[alt="Live Feed"]');
                  if(img) img.src = `${STREAM_URL}?t=${Date.now()}`;
                }}
                className="px-4 py-2 bg-blue-600 text-white rounded text-sm hover:bg-blue-500"
              >
                Retry Connection
              </button>
            </div>
          )}
        </div>

        {/* CONTROLS */}
        <div className="xl:col-span-5 flex flex-col gap-6">
          <div className="bg-gray-800/40 p-6 rounded-2xl border border-gray-700/50 shadow-xl flex-1 flex flex-col justify-center">
            <div className="flex justify-between items-center mb-6">
              <h2 className="text-lg font-semibold flex items-center gap-2"><Zap className="w-5 h-5 text-yellow-400"/> Controls</h2>
              <button onClick={() => sendCommand('auto_toggle')} className={`px-4 py-2 rounded-lg font-bold text-xs flex gap-2 transition-all ${telemetry.auto_mode ? 'bg-purple-600 text-white animate-pulse' : 'bg-gray-700 text-gray-300 hover:bg-gray-600'}`}><Bot className="w-4 h-4"/> {telemetry.auto_mode ? 'DISABLE AUTO' : 'ENABLE AUTO'}</button>
            </div>
            
            <div className="grid grid-cols-3 gap-3 max-w-[220px] mx-auto mb-6">
              <div></div><button onMouseDown={() => sendCommand('forward')} onMouseUp={() => sendCommand('stop')} className="h-14 bg-blue-600 rounded-xl flex items-center justify-center text-white shadow-lg active:scale-95 transition-all"><ArrowUp className="w-6 h-6"/></button><div></div>
              <button onMouseDown={() => sendCommand('left')} onMouseUp={() => sendCommand('stop')} className="h-14 bg-gray-700 rounded-xl flex items-center justify-center text-white active:scale-95 transition-all"><ArrowLeft className="w-6 h-6"/></button>
              <button onClick={() => sendCommand('stop')} className="h-14 bg-red-900/20 border border-red-500/50 text-red-500 rounded-xl flex items-center justify-center active:scale-95 transition-all"><StopCircle className="w-6 h-6"/></button>
              <button onMouseDown={() => sendCommand('right')} onMouseUp={() => sendCommand('stop')} className="h-14 bg-gray-700 rounded-xl flex items-center justify-center text-white active:scale-95 transition-all"><ArrowRight className="w-6 h-6"/></button>
              <div></div><button onMouseDown={() => sendCommand('backward')} onMouseUp={() => sendCommand('stop')} className="h-14 bg-gray-700 rounded-xl flex items-center justify-center text-white active:scale-95 transition-all"><ArrowDown className="w-6 h-6"/></button><div></div>
            </div>

            <button onClick={() => sendCommand('emergency')} className={`w-full py-3 rounded-xl font-bold tracking-widest transition-all ${telemetry.emergency ? 'bg-yellow-500 text-black animate-pulse' : 'bg-red-600 hover:bg-red-500 text-white shadow-lg shadow-red-900/30'}`}>
              {telemetry.emergency ? 'RESET SYSTEM' : 'EMERGENCY STOP'}
            </button>
          </div>

          <div className="bg-gray-800/40 rounded-2xl border border-gray-700/50 overflow-hidden">
            <SystemHealthPanel systemHealth={systemHealth} telemetry={telemetry} performanceMetrics={performanceMetrics} connectionStats={connectionStats} />
          </div>
        </div>

        {/* CHARTS */}
        {showAnalytics && (
          <div className="xl:col-span-12 grid grid-cols-1 md:grid-cols-4 gap-6 animate-slide-up">
            <TelemetryChart data={batteryHistory} title="Voltage" color="#10b981" icon={Battery} unit="V" />
            <TelemetryChart data={gasHistory} title="Gas Level" color="#f59e0b" icon={Wind} unit="" />
            <TelemetryChart data={distanceHistory} title="Distance" color="#3b82f6" icon={Gauge} unit="cm" />
            <div className="bg-gray-800 p-4 rounded-xl border border-gray-700"><DataExportPanel telemetry={telemetry} waypoints={[]} alerts={alerts} connectionStats={connectionStats} /></div>
          </div>
        )}
      </div>
    </div>
  );
}