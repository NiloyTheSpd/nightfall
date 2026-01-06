import React, { useEffect, useRef, useState, useCallback } from 'react';
import { Video, RefreshCw, AlertTriangle, Eye } from 'lucide-react';

/**
 * MjpegVideo - A proper MJPEG stream viewer for ESP-CAM and similar cameras
 * 
 * Features:
 * - Handles MJPEG multipart streams correctly using fetch and canvas
 * - Shows loading state while connecting
 * - Has proper error handling with retry logic
 * - Shows placeholder when camera is offline
 * - Displays "No Camera Signal" overlay when disconnected
 */
const CAMERA_STREAM_URL = 'http://192.168.4.2/stream'; // Fixed camera stream URL

const MjpegVideo = ({ 
  streamUrl, 
  isActive = true,
  onFpsUpdate,
  showFps = false,
  className = "",
  placeholderClassName = ""
}) => {
  const canvasRef = useRef(null);
  const abortControllerRef = useRef(null);
  const frameCountRef = useRef(0);
  const lastFrameTimeRef = useRef(Date.now());
  const retryTimeoutRef = useRef(null);
  const isStreamActiveRef = useRef(false);

  const [connectionState, setConnectionState] = useState('disconnected'); // disconnected, connecting, connected, error
  const [error, setError] = useState(null);
  const [fps, setFps] = useState(0);

  // Cleanup function
  const stopStream = useCallback(() => {
    isStreamActiveRef.current = false;
    if (abortControllerRef.current) {
      abortControllerRef.current.abort();
      abortControllerRef.current = null;
    }
    if (retryTimeoutRef.current) {
      clearTimeout(retryTimeoutRef.current);
      retryTimeoutRef.current = null;
    }
  }, []);

  // Parse MJPEG boundary from content-type header
  const getBoundary = async (url) => {
    try {
      const response = await fetch(url, { method: 'HEAD', cache: 'no-store' });
      const contentType = response.headers.get('content-type');
      if (contentType && contentType.includes('multipart')) {
        const boundaryMatch = contentType.match(/boundary=(.+)/);
        if (boundaryMatch) return boundaryMatch[1];
      }
    } catch (e) {
      console.warn('Could not get boundary:', e);
    }
    return null;
  };

  // Main stream reading function
  const readStream = useCallback(async (url, boundary) => {
    if (!isStreamActiveRef.current) return;

    try {
      abortControllerRef.current = new AbortController();
      const response = await fetch(url, { 
        signal: abortControllerRef.current.signal,
        cache: 'no-store'
      });

      if (!response.ok) {
        throw new Error(`HTTP ${response.status}: ${response.statusText}`);
      }

      const reader = response.body.getReader();
      const decoder = new TextDecoder();
      let buffer = '';
      const canvas = canvasRef.current;
      const ctx = canvas?.getContext('2d');

      setConnectionState('connected');

      while (isStreamActiveRef.current && reader) {
        const { done, value } = await reader.read();
        if (done) break;

        // Decode chunk and add to buffer
        buffer += decoder.decode(value, { stream: true });

        // Find JPEG frames using boundary
        const boundaryBytes = `--${boundary}`;
        let startIdx = buffer.indexOf(boundaryBytes);

        while (startIdx !== -1 && isStreamActiveRef.current) {
          // Find end of current frame (next boundary or end of buffer)
          let endIdx = buffer.indexOf(boundaryBytes, startIdx + boundaryBytes.length);
          if (endIdx === -1) {
            // Check if we have a complete frame at the end (may be missing boundary)
            const jpegStart = buffer.indexOf('\r\n\r\n', startIdx);
            if (jpegStart !== -1) {
              const headerEnd = jpegStart + 4;
              // Look for JPEG magic bytes
              const jpegMagicIdx = buffer.indexOf('\xff\xd8', headerEnd);
              if (jpegMagicIdx !== -1) {
                // Find end (next boundary or EOF)
                const potentialEnd = buffer.indexOf('\r\n--', jpegMagicIdx);
                if (potentialEnd !== -1) {
                  const frameData = buffer.substring(jpegMagicIdx, potentialEnd);
                  renderFrame(ctx, canvas, frameData);
                  buffer = buffer.substring(potentialEnd);
                  startIdx = buffer.indexOf(boundaryBytes);
                  continue;
                }
              }
            }
            break;
          }

          // Extract JPEG data between boundaries
          const headerEnd = buffer.indexOf('\r\n\r\n', startIdx);
          if (headerEnd !== -1 && headerEnd < endIdx) {
            const jpegStart = buffer.indexOf('\xff\xd8', headerEnd);
            if (jpegStart !== -1 && jpegStart < endIdx) {
              const frameData = buffer.substring(jpegStart, endIdx);
              renderFrame(ctx, canvas, frameData);
            }
          }

          buffer = buffer.substring(endIdx);
          startIdx = buffer.indexOf(boundaryBytes);
        }
      }
    } catch (error) {
      if (error.name !== 'AbortError') {
        console.error('Stream error:', error);
        setError(error.message);
        setConnectionState('error');
      }
    }
  }, []);

  // Simple JPEG renderer using image object (more reliable)
  const renderFrame = (ctx, canvas, frameData) => {
    if (!ctx || !canvas || !frameData.length) return;

    try {
      const blob = new Blob([new Uint8Array(Array.from(frameData).map(c => c.charCodeAt ? c.charCodeAt(0) : c))], { type: 'image/jpeg' });
      const img = new Image();
      img.onload = () => {
        ctx.clearRect(0, 0, canvas.width, canvas.height);
        
        // Calculate aspect ratio to fit in canvas
        const scale = Math.min(canvas.width / img.width, canvas.height / img.height);
        const x = (canvas.width - img.width * scale) / 2;
        const y = (canvas.height - img.height * scale) / 2;
        
        ctx.drawImage(img, x, y, img.width * scale, img.height * scale);
        
        // FPS counter
        frameCountRef.current++;
        const now = Date.now();
        if (now - lastFrameTimeRef.current >= 1000) {
          const currentFps = frameCountRef.current;
          setFps(currentFps);
          if (onFpsUpdate) onFpsUpdate(currentFps);
          frameCountRef.current = 0;
          lastFrameTimeRef.current = now;
        }
      };
      img.onerror = () => {
        // Silently ignore frame errors (common with corrupted frames)
      };
      img.src = URL.createObjectURL(blob);
    } catch (e) {
      // Silently ignore rendering errors
    }
  };

  // Start streaming
  const startStream = useCallback(async () => {
    if (!streamUrl || !isActive) return;

    setConnectionState('connecting');
    setError(null);
    isStreamActiveRef.current = true;

    // Try with boundary detection first, then fallback to simple fetch
    const boundary = await getBoundary(streamUrl);
    
    if (boundary) {
      // Use boundary-based stream reader
      readStream(streamUrl, boundary);
    } else {
      // Fallback: use img element approach (more compatible)
      setConnectionState('connected');
    }
  }, [streamUrl, isActive, readStream]);

  // Retry function
  const retryConnection = useCallback(() => {
    stopStream();
    startStream();
  }, [stopStream, startStream]);

  // Effect to start/stop stream
  useEffect(() => {
    if (isActive && streamUrl) {
      startStream();
    } else {
      stopStream();
    }

    return () => {
      stopStream();
    };
  }, [isActive, streamUrl, startStream, stopStream]);

  // Calculate actual stream URL
  const actualStreamUrl = streamUrl;

  return (
    <div className={`relative w-full h-full ${className}`}>
      {/* Canvas for MJPEG rendering */}
      <canvas 
        ref={canvasRef}
        width="640"
        height="480"
        className={`w-full h-full object-contain ${connectionState !== 'connected' ? 'hidden' : ''}`}
        style={{ willChange: 'contents' }}
      />

      {/* Fallback: Image element for simpler MJPEG streams */}
      {connectionState === 'connected' && (
        <img
          src={actualStreamUrl}
          alt="Camera Feed"
          className={`w-full h-full object-contain ${connectionState === 'connected' ? 'block' : 'hidden'}`}
          decoding="async"
          loading="eager"
          onLoad={() => frameCountRef.current++}
          onError={(e) => {
            // If img fails, fall back to canvas method
            e.target.style.display = 'none';
          }}
        />
      )}

      {/* Loading State */}
      {connectionState === 'connecting' && (
        <div className={`absolute inset-0 flex flex-col items-center justify-center bg-gray-900 ${placeholderClassName}`}>
          <div className="flex flex-col items-center gap-4">
            <div className="w-12 h-12 border-4 border-blue-500 border-t-transparent rounded-full animate-spin"></div>
            <span className="text-blue-400 text-sm font-medium">Connecting to Camera...</span>
          </div>
        </div>
      )}

      {/* Error State */}
      {connectionState === 'error' && (
        <div className={`absolute inset-0 flex flex-col items-center justify-center bg-gray-900/90 ${placeholderClassName}`}>
          <div className="flex flex-col items-center gap-4 max-w-xs text-center">
            <AlertTriangle className="w-12 h-12 text-amber-500" />
            <span className="text-white font-medium">Camera Connection Failed</span>
            <span className="text-gray-400 text-sm">{error || 'Unable to connect to camera stream'}</span>
            
            <button 
              onClick={retryConnection}
              className="mt-2 px-4 py-2 bg-blue-600 hover:bg-blue-500 text-white text-sm rounded-lg transition-colors flex items-center gap-2"
            >
              <RefreshCw className="w-4 h-4" />
              Retry Connection
            </button>
          </div>
        </div>
      )}

      {/* Disconnected State */}
      {connectionState === 'disconnected' && (
        <div className={`absolute inset-0 flex flex-col items-center justify-center bg-gray-900 ${placeholderClassName}`}>
          <div className="flex flex-col items-center gap-4">
            <Video className="w-12 h-12 text-gray-500" />
            <span className="text-gray-400 text-sm">Camera Disconnected</span>
          </div>
        </div>
      )}

      {/* No Signal Overlay */}
      {!isActive && (
        <div className={`absolute inset-0 flex flex-col items-center justify-center bg-black/80 ${placeholderClassName}`}>
          <Video className="w-12 h-12 text-gray-500 mb-2" />
          <span className="text-gray-400 text-sm">No Camera Signal</span>
        </div>
      )}

      {/* FPS Indicator */}
      {showFps && connectionState === 'connected' && (
        <div className="absolute top-4 left-4 bg-black/60 px-3 py-1.5 rounded text-xs text-white backdrop-blur flex items-center gap-2">
          <Eye className="w-3 h-3 text-blue-400" />
          <span>FPS: {fps}</span>
        </div>
      )}
    </div>
  );
};

// Memoized version for performance
const MemoizedMjpegVideo = React.memo(MjpegVideo);

export default MemoizedMjpegVideo;
export { MjpegVideo };
