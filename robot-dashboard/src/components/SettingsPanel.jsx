import React, { useState } from 'react';
import { 
  Settings, 
  X, 
  Monitor, 
  Volume2, 
  VolumeX, 
  Download,
  Upload,
  Trash2,
  Save,
  RotateCcw,
  Sliders
} from 'lucide-react';

const SettingsPanel = ({ isOpen, onClose, settings, onSettingsChange }) => {
  const [localSettings, setLocalSettings] = useState(settings);

  const handleChange = (key, value) => {
    const newSettings = { ...localSettings, [key]: value };
    setLocalSettings(newSettings);
    onSettingsChange(newSettings);
  };

  const handleSave = () => {
    localStorage.setItem('dashboard-settings', JSON.stringify(localSettings));
    onClose();
  };

  const handleReset = () => {
    const defaultSettings = {
      theme: 'dark',
      enableSounds: false,
      autoReconnect: true,
      showNotifications: true,
      videoQuality: 'high',
      dataRefreshRate: 100,
      commandThrottle: 50,
      maxTelemetryHistory: 50
    };
    setLocalSettings(defaultSettings);
    onSettingsChange(defaultSettings);
  };

  const exportSettings = () => {
    const dataStr = JSON.stringify(localSettings, null, 2);
    const dataBlob = new Blob([dataStr], { type: 'application/json' });
    const url = URL.createObjectURL(dataBlob);
    const link = document.createElement('a');
    link.href = url;
    link.download = 'dashboard-settings.json';
    link.click();
    URL.revokeObjectURL(url);
  };

  const importSettings = (event) => {
    const file = event.target.files[0];
    if (!file) return;

    const reader = new FileReader();
    reader.onload = (e) => {
      try {
        const imported = JSON.parse(e.target.result);
        setLocalSettings(imported);
        onSettingsChange(imported);
      } catch (error) {
        console.error('Failed to import settings:', error);
      }
    };
    reader.readAsText(file);
  };

  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4 bg-black/70 backdrop-blur-sm animate-fade-in">
      <div className="card p-6 max-w-2xl w-full max-h-[90vh] overflow-y-auto">
        <div className="flex items-center justify-between mb-6">
          <div className="flex items-center gap-3">
            <div className="bg-nightfall-accent p-2 rounded-lg">
              <Settings className="w-6 h-6 text-white" />
            </div>
            <h2 className="text-2xl font-bold text-white">Dashboard Settings</h2>
          </div>
          <button
            onClick={onClose}
            className="text-gray-400 hover:text-white transition-colors p-2 hover:bg-gray-700 rounded-lg"
            aria-label="Close settings"
          >
            <X className="w-6 h-6" />
          </button>
        </div>

        <div className="space-y-6">
          {/* Appearance */}
          <div>
            <h3 className="text-lg font-semibold text-white mb-3 flex items-center gap-2">
              <Monitor className="w-5 h-5 text-nightfall-accent" />
              Appearance
            </h3>
            <div className="space-y-3 bg-gray-800 p-4 rounded-lg">
              <div className="flex items-center justify-between">
                <label className="text-gray-300">Theme</label>
                <select
                  value={localSettings.theme}
                  onChange={(e) => handleChange('theme', e.target.value)}
                  className="bg-gray-700 text-white px-3 py-2 rounded-lg border border-gray-600 focus:border-nightfall-accent focus:outline-none"
                >
                  <option value="dark">Dark</option>
                  <option value="light">Light (Coming Soon)</option>
                  <option value="auto">Auto (Coming Soon)</option>
                </select>
              </div>
            </div>
          </div>

          {/* Audio */}
          <div>
            <h3 className="text-lg font-semibold text-white mb-3 flex items-center gap-2">
              {localSettings.enableSounds ? (
                <Volume2 className="w-5 h-5 text-nightfall-accent" />
              ) : (
                <VolumeX className="w-5 h-5 text-gray-500" />
              )}
              Audio
            </h3>
            <div className="space-y-3 bg-gray-800 p-4 rounded-lg">
              <div className="flex items-center justify-between">
                <label className="text-gray-300">Enable Sounds</label>
                <button
                  onClick={() => handleChange('enableSounds', !localSettings.enableSounds)}
                  className={`relative inline-flex h-6 w-11 items-center rounded-full transition-colors ${
                    localSettings.enableSounds ? 'bg-nightfall-success' : 'bg-gray-600'
                  }`}
                >
                  <span
                    className={`inline-block h-4 w-4 transform rounded-full bg-white transition-transform ${
                      localSettings.enableSounds ? 'translate-x-6' : 'translate-x-1'
                    }`}
                  />
                </button>
              </div>
            </div>
          </div>

          {/* Connection */}
          <div>
            <h3 className="text-lg font-semibold text-white mb-3 flex items-center gap-2">
              <Sliders className="w-5 h-5 text-nightfall-accent" />
              Connection
            </h3>
            <div className="space-y-3 bg-gray-800 p-4 rounded-lg">
              <div className="flex items-center justify-between">
                <label className="text-gray-300">Auto Reconnect</label>
                <button
                  onClick={() => handleChange('autoReconnect', !localSettings.autoReconnect)}
                  className={`relative inline-flex h-6 w-11 items-center rounded-full transition-colors ${
                    localSettings.autoReconnect ? 'bg-nightfall-success' : 'bg-gray-600'
                  }`}
                >
                  <span
                    className={`inline-block h-4 w-4 transform rounded-full bg-white transition-transform ${
                      localSettings.autoReconnect ? 'translate-x-6' : 'translate-x-1'
                    }`}
                  />
                </button>
              </div>

              <div className="flex items-center justify-between">
                <label className="text-gray-300">Show Notifications</label>
                <button
                  onClick={() => handleChange('showNotifications', !localSettings.showNotifications)}
                  className={`relative inline-flex h-6 w-11 items-center rounded-full transition-colors ${
                    localSettings.showNotifications ? 'bg-nightfall-success' : 'bg-gray-600'
                  }`}
                >
                  <span
                    className={`inline-block h-4 w-4 transform rounded-full bg-white transition-transform ${
                      localSettings.showNotifications ? 'translate-x-6' : 'translate-x-1'
                    }`}
                  />
                </button>
              </div>

              <div>
                <div className="flex items-center justify-between mb-2">
                  <label className="text-gray-300">Command Throttle (ms)</label>
                  <span className="font-mono text-nightfall-accent">{localSettings.commandThrottle}</span>
                </div>
                <input
                  type="range"
                  min="10"
                  max="200"
                  step="10"
                  value={localSettings.commandThrottle}
                  onChange={(e) => handleChange('commandThrottle', parseInt(e.target.value))}
                  className="w-full accent-nightfall-accent"
                />
              </div>
            </div>
          </div>

          {/* Video */}
          <div>
            <h3 className="text-lg font-semibold text-white mb-3 flex items-center gap-2">
              <Monitor className="w-5 h-5 text-nightfall-accent" />
              Video
            </h3>
            <div className="space-y-3 bg-gray-800 p-4 rounded-lg">
              <div className="flex items-center justify-between">
                <label className="text-gray-300">Video Quality</label>
                <select
                  value={localSettings.videoQuality}
                  onChange={(e) => handleChange('videoQuality', e.target.value)}
                  className="bg-gray-700 text-white px-3 py-2 rounded-lg border border-gray-600 focus:border-nightfall-accent focus:outline-none"
                >
                  <option value="low">Low</option>
                  <option value="medium">Medium</option>
                  <option value="high">High</option>
                </select>
              </div>
            </div>
          </div>

          {/* Data Management */}
          <div>
            <h3 className="text-lg font-semibold text-white mb-3">Data Management</h3>
            <div className="grid grid-cols-2 gap-3">
              <button
                onClick={exportSettings}
                className="btn-secondary flex items-center justify-center gap-2"
              >
                <Download className="w-4 h-4" />
                Export Settings
              </button>

              <label className="btn-secondary flex items-center justify-center gap-2 cursor-pointer">
                <Upload className="w-4 h-4" />
                Import Settings
                <input
                  type="file"
                  accept=".json"
                  onChange={importSettings}
                  className="hidden"
                />
              </label>
            </div>
          </div>

          {/* Actions */}
          <div className="flex gap-3 pt-4 border-t border-gray-700">
            <button
              onClick={handleReset}
              className="btn-danger flex-1 flex items-center justify-center gap-2"
            >
              <RotateCcw className="w-4 h-4" />
              Reset to Defaults
            </button>

            <button
              onClick={handleSave}
              className="btn-primary flex-1 flex items-center justify-center gap-2"
            >
              <Save className="w-4 h-4" />
              Save & Close
            </button>
          </div>
        </div>
      </div>
    </div>
  );
};

export default SettingsPanel;
