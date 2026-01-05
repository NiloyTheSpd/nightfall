import React, { useEffect, useRef, useState, useCallback } from 'react';
import {
  Camera, Radio, Gauge, Battery, AlertTriangle, Wind,
  Wifi, WifiOff, ArrowUp, ArrowDown, ArrowLeft, ArrowRight,
  StopCircle, Eye, Zap, Signal, Settings, Maximize2, AlertCircle,
  Bot, Clock, Volume2, VolumeX, Keyboard
} from 'lucide-react';

// --- FIXED IMPORTS (Relative to src/components/) ---
import { DashboardLoader } from './components/LoadingSkeleton';
import TelemetryChart, { useTelemetryHistory } from './components/TelemetryChart';
import SettingsPanel from './components/SettingsPanel';
import SystemHealthPanel from './components/SystemHealthPanel';
import FullscreenVideo from './components/FullscreenVideo';
import DataExportPanel from './components/DataExportPanel';

const WEBSOCKET_URL = 'ws://192.168.4.1:8888';
const ConnectionStates = { DISCONNECTED: 'disconnected', CONNECTING: 'connecting', CONNECTED: 'connected', ERROR: 'error' };
const SystemStatus = { HEALTHY: 'healthy', WARNING: 'warning', CRITICAL: 'critical', OFFLINE: 'offline' };

// --- LOCAL UTILITIES ---
function Tooltip({ children, content, position = 'bottom' }) {
  const [show, setShow] = useState(false);
  return (
    <div className="relative" onMouseEnter={() => setShow(true)} onMouseLeave={() => setShow(false)}>
      {children}
      {show && (
        <div className={`absolute ${position === 'bottom' ? 'top-full mt-2' : 'bottom-full mb-2'} left-1/2 -translate-x-1/2 px-2 py-1 bg-gray-900 text-white text-xs rounded whitespace-nowrap z-50 animate-fade-in shadow-lg border border-gray-700`}>{content}</div>
      )}
    </div>
  );
}

function KeyboardShortcuts({ onClose }) {
  return (
    <div className="fixed inset-0 bg-black/70 flex items-center justify-center z-50 animate-fade-in" onClick={onClose}>
      <div className="bg-gray-800 p-6 rounded-2xl border border-gray-700 max-w-md w-full mx-4 animate-slide-up shadow-2xl" onClick={e => e.stopPropagation()}>
        <div className="flex justify-between items-center mb-4">
          <h3 className="text-lg font-bold flex items-center gap-2"><Keyboard className="w-5 h-5 text-purple-400" /> Keyboard Shortcuts</h3>
          <button onClick={onClose} className="p-1 hover:bg-gray-700 rounded-lg transition-colors text-gray-400 hover:text-white">✕</button>
        </div>
        <div className="space-y-2 text-sm">
          {[{ key: 'W', action: 'Move Forward' }, { key: 'S', action: 'Move Backward' }, { key: 'A', action: 'Turn Left' }, { key: 'D', action: 'Turn Right' }, { key: 'Space', action: 'Stop' }, { key: 'Esc', action: 'Emergency Stop' }, { key: '?', action: 'Show Help' }].map(({ key, action }) => (
            <div key={key} className="flex justify-between items-center py-2 border-b border-gray-700/50 last:border-0"><span className="text-gray-300">{action}</span><kbd className="px-2 py-1 bg-gray-700 rounded text-white font-mono text-xs border border-gray-600 shadow-sm">{key}</kbd></div>
          ))}
        </div>
      </div>
    </div>
  );
}

// --- MAIN DASHBOARD ---
export default function DashboardEnhanced() {
  const [isLoading, setIsLoading] = useState(true);
  const [showSettings, setShowSettings] = useState(false);
  const [showFullscreenVideo, setShowFullscreenVideo] = useState(false);
  const [showAnalytics, setShowAnalytics] = useState(false);
  const [showShortcuts, setShowShortcuts] = useState(false);
  const [soundEnabled, setSoundEnabled] = useState(true);
  
  const [settings, setSettings] = useState(() => {
    const saved = localStorage.getItem('dashboard-settings');
    return saved ? JSON.parse(saved) : { theme: 'dark', maxTelemetryHistory: 50 };
  });

  const { batteryHistory, gasHistory, distanceHistory, signalHistory, addTelemetryPoint } = useTelemetryHistory(settings.maxTelemetryHistory);
  const [connectionState, setConnectionState] = useState(ConnectionStates.DISCONNECTED);
  const [connectionStats, setConnectionStats] = useState({ messagesReceived: 0, messagesSent: 0 });

  const [telemetry, setTelemetry] = useState({
    back_status: 'offline', front_status: false, camera_status: false,
    dist: 0, gas: 0, cam_ip: null, battery: 11.1, signal_strength: 0,
    uptime: 0, emergency: false, auto_mode: false
  });

  const [systemHealth, setSystemHealth] = useState({ brain: SystemStatus.OFFLINE, motors: SystemStatus.OFFLINE, vision: SystemStatus.OFFLINE });
  const [performanceMetrics, setPerformanceMetrics] = useState({ dataRate: 0, commandSuccessRate: 100, fps: 0 });
  const [alerts, setAlerts] = useState([]);
  const [videoFps, setVideoFps] = useState(0);

  const wsRef = useRef(null);
  const fpsCounterRef = useRef({ frames: 0, lastTime: Date.now() });
  const lastCommandTimeRef = useRef(0);
  const commandFeedbackRef = useRef(null);
  
  const playSound = useCallback((type) => {
    if (!soundEnabled) return;
    try {
      const AudioContext = window.AudioContext || window.webkitAudioContext;
      if (!AudioContext) return;
      const ctx = new AudioContext();
      const osc = ctx.createOscillator();
      const gain = ctx.createGain();
      osc.connect(gain); gain.connect(ctx.destination);
      const now = ctx.currentTime;
      if (type === 'move') { osc.frequency.setValueAtTime(600, now); osc.frequency.exponentialRampToValueAtTime(300, now + 0.1); gain.gain.setValueAtTime(0.05, now); gain.gain.exponentialRampToValueAtTime(0.01, now + 0.1); osc.start(now); osc.stop(now + 0.1); }
      else if (type === 'stop') { osc.frequency.setValueAtTime(200, now); osc.frequency.exponentialRampToValueAtTime(100, now + 0.15); gain.gain.setValueAtTime(0.1, now); gain.gain.exponentialRampToValueAtTime(0.01, now + 0.15); osc.start(now); osc.stop(now + 0.15); }
      else if (type === 'alert') { osc.type = 'square'; osc.frequency.setValueAtTime(800, now); osc.frequency.setValueAtTime(0, now + 0.1); osc.frequency.setValueAtTime(800, now + 0.2); gain.gain.setValueAtTime(0.1, now); osc.start(now); osc.stop(now + 0.3); }
    } catch (e) { console.warn("Audio error", e); }
  }, [soundEnabled]);

  const addAlert = useCallback((message, type = 'info') => {
    const alert = { id: Date.now() + Math.random(), message, type, timestamp: new Date() };
    setAlerts(prev => [alert, ...prev].slice(0, 10));
    if (type === 'error') playSound('alert');
    setTimeout(() => setAlerts(prev => prev.filter(a => a.id !== alert.id)), 5000);
  }, [playSound]);

  const showCommandFeedback = useCallback((command) => {
    if (commandFeedbackRef.current) {
      commandFeedbackRef.current.textContent = command.replace('_', ' ').toUpperCase();
      commandFeedbackRef.current.style.opacity = '1';
      setTimeout(() => { if(commandFeedbackRef.current) commandFeedbackRef.current.style.opacity = '0'; }, 500);
    }
  }, []);

  useEffect(() => {
    setTimeout(() => setIsLoading(false), 1500);
    const connect = () => {
      if (wsRef.current?.readyState === WebSocket.OPEN) return;
      setConnectionState(ConnectionStates.CONNECTING);
      const ws = new WebSocket(WEBSOCKET_URL);
      wsRef.current = ws;
      ws.onopen = () => { setConnectionState(ConnectionStates.CONNECTED); addAlert('Connected to Robot Brain', 'success'); };
      ws.onmessage = (event) => {
        try {
          const data = JSON.parse(event.data);
          setConnectionStats(prev => ({ ...prev, messagesReceived: prev.messagesReceived + 1 }));
          if (data.type === 'cam_telemetry') { setTelemetry(prev => ({ ...prev, cam_ip: data.ip, signal_strength: data.rssi, camera_status: true })); setSystemHealth(prev => ({...prev, vision: SystemStatus.HEALTHY})); } 
          else if (data.d !== undefined) {
            setTelemetry(prev => {
              const newData = { ...prev, dist: data.d, gas: data.g, battery: data.v, emergency: data.e, front_status: data.fo, back_status: 'ok', auto_mode: data.auto, uptime: prev.uptime + 0.5 };
              addTelemetryPoint(newData); return newData;
            });
            setSystemHealth(prev => ({ ...prev, brain: SystemStatus.HEALTHY, motors: data.fo ? SystemStatus.HEALTHY : SystemStatus.CRITICAL }));
            if (data.e) addAlert("EMERGENCY STOP ACTIVE", 'error');
          }
        } catch (e) { console.error("Parse Error", e); }
      };
      ws.onclose = () => { setConnectionState(ConnectionStates.DISCONNECTED); setSystemHealth({ brain: SystemStatus.OFFLINE, motors: SystemStatus.OFFLINE, vision: SystemStatus.OFFLINE }); setTimeout(connect, 3000); };
      ws.onerror = () => { setConnectionState(ConnectionStates.ERROR); ws.close(); };
    };
    connect();
    return () => { if (wsRef.current) wsRef.current.close(); };
  }, [addAlert, addTelemetryPoint]);

  const sendCommand = (cmdString) => {
    if (!wsRef.current || wsRef.current.readyState !== WebSocket.OPEN) return;
    const now = Date.now();
    if (now - lastCommandTimeRef.current < 50) return;
    wsRef.current.send(JSON.stringify({ command: cmdString }));
    lastCommandTimeRef.current = now;
    setConnectionStats(prev => ({ ...prev, messagesSent: prev.messagesSent + 1 }));
    showCommandFeedback(cmdString);
    if(cmdString !== 'auto_toggle') playSound(cmdString === 'stop' ? 'stop' : 'move');
  };

  const toggleAuto = () => sendCommand('auto_toggle');
  const moveForward = () => sendCommand('forward');
  const moveBackward = () => sendCommand('backward');
  const turnLeft = () => sendCommand('left');
  const turnRight = () => sendCommand('right');
  const stopRobot = () => sendCommand('stop');
  const emergencyStop = () => sendCommand('emergency');

  useEffect(() => {
    const handleKeyDown = (e) => {
      if(e.repeat) return;
      switch(e.key.toLowerCase()) {
        case 'w': moveForward(); break; case 's': moveBackward(); break; case 'a': turnLeft(); break; case 'd': turnRight(); break; case ' ': stopRobot(); break; case 'escape': emergencyStop(); break; case '?': setShowShortcuts(true); break; default: break;
      }
    };
    const handleKeyUp = (e) => { if(['w','a','s','d'].includes(e.key.toLowerCase())) stopRobot(); };
    window.addEventListener('keydown', handleKeyDown); window.addEventListener('keyup', handleKeyUp);
    return () => { window.removeEventListener('keydown', handleKeyDown); window.removeEventListener('keyup', handleKeyUp); };
  }, []);

  useEffect(() => {
    const interval = setInterval(() => {
      const now = Date.now();
      if (now - fpsCounterRef.current.lastTime >= 1000) { setVideoFps(fpsCounterRef.current.frames); fpsCounterRef.current.frames = 0; fpsCounterRef.current.lastTime = now; }
    }, 1000);
    return () => clearInterval(interval);
  }, []);

  const formatUptime = (seconds) => { const hrs = Math.floor(seconds / 3600); const mins = Math.floor((seconds % 3600) / 60); return `${hrs}h ${mins}m`; };
  const getConnectionQuality = (signal) => { if (signal >= -50) return 4; if (signal >= -60) return 3; if (signal >= -70) return 2; return 1; };

  if (isLoading) return <DashboardLoader />;

  return (
    <div className="min-h-screen bg-gradient-to-br from-gray-900 via-slate-900 to-black text-white p-4 sm:p-6 font-sans animate-fade-in">
      {showShortcuts && <KeyboardShortcuts onClose={() => setShowShortcuts(false)} />}
      <SettingsPanel isOpen={showSettings} onClose={() => setShowSettings(false)} settings={settings} onSettingsChange={setSettings} />
      <FullscreenVideo isOpen={showFullscreenVideo} onClose={() => setShowFullscreenVideo(false)} cameraIp={telemetry.cam_ip || '192.168.4.3'} cameraStatus={telemetry.camera_status} />

      {/* HEADER */}
      <header className="mb-6 flex flex-wrap justify-between items-center bg-gray-800/50 p-4 rounded-2xl backdrop-blur-sm border border-gray-700/50 shadow-xl relative overflow-hidden">
        <div className="absolute inset-0 rounded-2xl opacity-10 bg-gradient-to-r from-blue-500 via-purple-500 to-pink-500 animate-pulse" />
        <div className="flex items-center gap-4 relative z-10">
            <div className="bg-blue-600 p-2 rounded-xl shadow-lg shadow-blue-900/40 relative">
              <Radio className="w-6 h-6 text-white" />
              {connectionState === ConnectionStates.CONNECTED && <span className="absolute -top-1 -right-1 w-3 h-3 bg-green-400 rounded-full animate-ping" />}
            </div>
            <div>
              <h1 className="text-2xl font-bold bg-clip-text text-transparent bg-gradient-to-r from-blue-400 via-purple-400 to-pink-400">Project Nightfall</h1>
              <div className="flex items-center gap-2 text-gray-400 text-xs font-medium tracking-wide"><span>Autonomous Rescue System</span><span className="text-gray-600">//</span><Tooltip content="Version 3.3.0"><span className="text-purple-400 cursor-help">V3.3.0</span></Tooltip></div>
            </div>
        </div>
        <div className="flex gap-2 items-center relative z-10 mt-4 sm:mt-0">
            <Tooltip content="Keyboard Shortcuts (?)"><button onClick={() => setShowShortcuts(true)} className="p-2 bg-gray-700/50 hover:bg-gray-600 rounded-lg transition-all"><Keyboard className="w-5 h-5" /></button></Tooltip>
            <Tooltip content={soundEnabled ? 'Sound On' : 'Sound Off'}><button onClick={() => setSoundEnabled(!soundEnabled)} className="p-2 bg-gray-700/50 hover:bg-gray-600 rounded-lg transition-all">{soundEnabled ? <Volume2 className="w-5 h-5" /> : <VolumeX className="w-5 h-5" />}</button></Tooltip>
            {telemetry.auto_mode && (<div className="flex items-center gap-2 px-3 py-1.5 rounded-lg border border-purple-500/50 text-purple-400 bg-purple-500/10 animate-pulse"><Bot className="w-4 h-4" /><span className="font-semibold text-sm hidden sm:inline">AUTO PILOT</span></div>)}
            <div className={`flex items-center gap-2 px-3 py-1.5 rounded-lg border transition-all ${connectionState === ConnectionStates.CONNECTED ? 'text-green-400 bg-green-400/10 border-green-500/30' : 'text-red-400 bg-red-400/10 border-red-500/30'}`}><Wifi className="w-4 h-4" /><div className="flex flex-col"><span className="font-semibold text-sm capitalize hidden sm:inline">{connectionState}</span></div></div>
            <div className="hidden md:flex items-center gap-1 px-3 py-1.5 bg-gray-700/30 rounded-lg border border-gray-700/50"><Clock className="w-4 h-4 text-gray-400" /><span className="font-mono text-sm text-gray-300">{formatUptime(telemetry.uptime)}</span></div>
            <button onClick={() => setShowSettings(true)} className="p-2 bg-gray-700/50 hover:bg-gray-600 rounded-lg transition-all"><Settings className="w-5 h-5" /></button>
        </div>
      </header>

      {/* MAIN LAYOUT */}
      <div className="grid grid-cols-1 xl:grid-cols-12 gap-6">
        {/* ZONE A: VIDEO */}
        <div className="xl:col-span-7 bg-black rounded-2xl overflow-hidden border border-gray-800 shadow-2xl relative group h-[400px] sm:h-[500px]">
             <div className="absolute inset-0 pointer-events-none z-10 bg-[radial-gradient(circle,transparent_60%,rgba(0,0,0,0.8)_100%)]" />
             {telemetry.camera_status && (<div className="absolute top-4 left-4 flex items-center gap-2 z-20"><span className="relative flex h-3 w-3"><span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-red-400 opacity-75"></span><span className="relative inline-flex rounded-full h-3 w-3 bg-red-500"></span></span><span className="text-red-400 font-bold text-xs tracking-wider uppercase">REC</span></div>)}
             {telemetry.cam_ip ? (<img src={`http://${telemetry.cam_ip}:81/stream`} className="w-full h-full object-contain" alt="Live Feed" onLoad={() => fpsCounterRef.current.frames++} onError={(e) => { e.target.style.display='none'; }} />) : (<div className="flex flex-col items-center justify-center h-full text-gray-500 animate-pulse"><Camera className="w-16 h-16 mb-4 opacity-50" /><p className="text-sm">Searching for Camera Signal...</p></div>)}
             <div className="absolute bottom-4 left-4 right-4 z-20 flex justify-between items-end">
               <div className="bg-black/60 backdrop-blur px-3 py-1.5 rounded-lg border border-white/10 text-xs">
                  <div className="flex items-center gap-2 mb-1"><Eye className="w-3 h-3 text-blue-400" /> <span>FPS: <span className="font-mono text-white">{videoFps}</span></span></div>
                  <div className="flex items-center gap-2"><Signal className="w-3 h-3 text-purple-400" /> <span>Signal: <span className="font-mono text-white">{telemetry.signal_strength}dB</span></span></div>
               </div>
               <div ref={commandFeedbackRef} className="bg-black/60 backdrop-blur px-4 py-2 rounded-lg border border-white/10 font-bold text-lg text-white tracking-widest uppercase opacity-0 transition-opacity duration-300"></div>
             </div>
             <button onClick={() => setShowFullscreenVideo(true)} className="absolute top-4 right-4 p-2 bg-black/60 hover:bg-black/80 rounded-lg backdrop-blur text-white z-20 opacity-0 group-hover:opacity-100 transition-all duration-300 hover:scale-110"><Maximize2 className="w-5 h-5" /></button>
        </div>

        {/* ZONE B: CONTROLS */}
        <div className="xl:col-span-5 flex flex-col gap-6">
            <div className="bg-gray-800/40 p-6 rounded-2xl border border-gray-700/50 shadow-xl flex-1 flex flex-col justify-center relative overflow-hidden">
                <div className="absolute inset-0 bg-gradient-to-br from-blue-500/5 to-purple-500/5 rounded-2xl pointer-events-none" />
                <div className="flex justify-between items-center mb-6 relative z-10">
                    <h2 className="text-lg font-semibold flex items-center gap-2"><Zap className="w-5 h-5 text-yellow-400" /> Controls</h2>
                    <button onClick={toggleAuto} className={`px-4 py-2 rounded-lg font-bold text-xs tracking-wider transition-all flex items-center gap-2 ${telemetry.auto_mode ? 'bg-purple-600 hover:bg-purple-500 text-white shadow-lg shadow-purple-500/30' : 'bg-gray-700 hover:bg-gray-600 text-gray-300 border border-gray-600'}`}><Bot className="w-4 h-4" />{telemetry.auto_mode ? 'DISABLE AUTO' : 'ENABLE AUTO'}</button>
                </div>
                <div className="grid grid-cols-3 gap-3 max-w-[220px] mx-auto mb-6 relative z-10">
                    <div></div>
                    <button onMouseDown={moveForward} onMouseUp={stopRobot} onTouchStart={moveForward} onTouchEnd={stopRobot} className="h-14 bg-blue-600 hover:bg-blue-500 rounded-xl shadow-lg shadow-blue-900/40 flex items-center justify-center active:scale-95 transition-all"><ArrowUp className="w-6 h-6"/></button>
                    <div></div>
                    <button onMouseDown={turnLeft} onMouseUp={stopRobot} onTouchStart={turnLeft} onTouchEnd={stopRobot} className="h-14 bg-gray-700 hover:bg-gray-600 rounded-xl flex items-center justify-center active:scale-95 transition-all"><ArrowLeft className="w-6 h-6"/></button>
                    <button onClick={stopRobot} className="h-14 bg-red-900/20 border border-red-500/50 text-red-500 hover:bg-red-900/40 rounded-xl flex items-center justify-center active:scale-95 transition-all"><StopCircle className="w-6 h-6"/></button>
                    <button onMouseDown={turnRight} onMouseUp={stopRobot} onTouchStart={turnRight} onTouchEnd={stopRobot} className="h-14 bg-gray-700 hover:bg-gray-600 rounded-xl flex items-center justify-center active:scale-95 transition-all"><ArrowRight className="w-6 h-6"/></button>
                    <div></div>
                    <button onMouseDown={moveBackward} onMouseUp={stopRobot} onTouchStart={moveBackward} onTouchEnd={stopRobot} className="h-14 bg-gray-700 hover:bg-gray-600 rounded-xl flex items-center justify-center active:scale-95 transition-all"><ArrowDown className="w-6 h-6"/></button>
                    <div></div>
                </div>
                <button onClick={emergencyStop} className={`w-full py-3 rounded-xl font-bold tracking-widest transition-all relative z-10 ${telemetry.emergency ? 'bg-yellow-500 text-black animate-pulse' : 'bg-red-600 hover:bg-red-500 text-white shadow-lg shadow-red-900/30'}`}>{telemetry.emergency ? 'RESET SYSTEM' : 'EMERGENCY STOP'}</button>
            </div>
            <div className="bg-gray-800/40 rounded-2xl border border-gray-700/50 shadow-xl overflow-hidden">
                <SystemHealthPanel systemHealth={systemHealth} telemetry={telemetry} performanceMetrics={performanceMetrics} connectionStats={connectionStats} />
            </div>
        </div>

        {/* ZONE C: ANALYTICS */}
        {showAnalytics && (
             <div className="xl:col-span-12 grid grid-cols-1 md:grid-cols-4 gap-6 animate-slide-up">
                 <TelemetryChart data={batteryHistory} title="Voltage" color="#10b981" icon={Battery} unit="V" />
                 <TelemetryChart data={gasHistory} title="Gas Level" color="#f59e0b" icon={Wind} unit="" />
                 <TelemetryChart data={distanceHistory} title="Distance" color="#3b82f6" icon={Gauge} unit="cm" />
                 <div className="bg-gray-800 p-4 rounded-xl border border-gray-700"><DataExportPanel telemetry={telemetry} waypoints={[]} alerts={alerts} connectionStats={connectionStats} /></div>
             </div>
        )}
        
        {/* ALERTS */}
        {alerts.length > 0 && (
          <div className="fixed bottom-6 right-6 flex flex-col gap-2 pointer-events-none z-50">
            {alerts.map(alert => (
              <div key={alert.id} className={`p-4 rounded-xl shadow-xl flex items-center gap-3 animate-slide-up ${alert.type === 'error' ? 'bg-red-600/90 text-white' : alert.type === 'success' ? 'bg-green-600/90 text-white' : 'bg-gray-800/90 text-white'}`}>
                {alert.type === 'error' ? <AlertCircle className="w-5 h-5"/> : <div className="w-2 h-2 rounded-full bg-white"/>}
                <span className="font-medium text-sm">{alert.message}</span>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
