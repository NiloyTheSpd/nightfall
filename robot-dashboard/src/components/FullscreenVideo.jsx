import React from 'react';
import { Maximize2, Minimize2, Download, X } from 'lucide-react';

const FullscreenVideo = ({ isOpen, onClose, cameraIp, cameraStatus }) => {
  if (!isOpen) return null;

  return (
    <div className="fixed inset-0 z-50 bg-black flex flex-col">
      {/* Header */}
      <div className="bg-gradient-to-b from-black/80 to-transparent p-4 flex items-center justify-between">
        <div className="flex items-center gap-4">
          <div className="flex items-center gap-2">
            <div className="w-3 h-3 bg-red-500 rounded-full animate-pulse"></div>
            <span className="text-white font-medium">LIVE</span>
          </div>
          <div className="text-gray-400 text-sm">
            Camera: {cameraIp}
          </div>
        </div>

        <button
          onClick={onClose}
          className="text-white hover:bg-white/10 p-2 rounded-lg transition-colors"
          aria-label="Exit fullscreen"
        >
          <X className="w-6 h-6" />
        </button>
      </div>

      {/* Video */}
      <div className="flex-1 flex items-center justify-center p-4">
        {cameraStatus ? (
          <img 
            src={`http://${cameraIp}:81/stream`}
            alt="Robot camera feed"
            className="max-w-full max-h-full object-contain"
          />
        ) : (
          <div className="text-center text-white">
            <div className="text-2xl mb-4">Camera Offline</div>
            <div className="text-gray-400">Waiting for video stream...</div>
          </div>
        )}
      </div>

      {/* Footer with controls */}
      <div className="bg-gradient-to-t from-black/80 to-transparent p-4 flex items-center justify-between">
        <div className="text-gray-400 text-sm">
          Press ESC to exit fullscreen
        </div>
        
        <div className="flex items-center gap-3">
          <button
            onClick={() => {
              const link = document.createElement('a');
              link.href = `http://${cameraIp}:81/stream`;
              link.download = `snapshot-${Date.now()}.jpg`;
              link.click();
            }}
            className="btn-secondary text-sm flex items-center gap-2"
          >
            <Download className="w-4 h-4" />
            Save Snapshot
          </button>
        </div>
      </div>
    </div>
  );
};

export default FullscreenVideo;
