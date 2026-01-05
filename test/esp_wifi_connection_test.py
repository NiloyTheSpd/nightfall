#!/usr/bin/env python3
"""
Test ESP32 WiFi AP Connectivity
Connects to ProjectNightfall WiFi network and verifies ESP32 is reachable
"""

import socket
import time

# Configuration
WIFI_SSID = "ProjectNightfall"
ESP32_IP = "192.168.4.1"  # Default AP IP
TEST_PORT = 80  # HTTP port

def test_wifi_connection():
    """Test basic WiFi connectivity to ESP32 AP"""
    print(f"[TEST] Attempting to connect to {WIFI_SSID}...")
    print(f"[INFO] ESP32 AP IP: {ESP32_IP}")
    
    try:
        # Create socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        
        # Try to connect to HTTP port
        print(f"[TEST] Testing port {TEST_PORT}...")
        result = sock.connect_ex((ESP32_IP, TEST_PORT))
        
        if result == 0:
            print(f"[✓] Successfully connected to {ESP32_IP}:{TEST_PORT}")
            print("[✓] ESP32 is reachable via WiFi")
            sock.close()
            return True
        else:
            print(f"[✗] Failed to connect to {ESP32_IP}:{TEST_PORT}")
            print("[!] Check if ESP32 is powered and WiFi AP is running")
            sock.close()
            return False
            
    except socket.timeout:
        print("[✗] Connection timeout - ESP32 not responding")
        return False
    except Exception as e:
        print(f"[✗] Connection error: {e}")
        return False

def test_websocket_port():
    """Test WebSocket port availability"""
    print(f"\n[TEST] Testing WebSocket port 8888...")
    
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(5)
        result = sock.connect_ex((ESP32_IP, 8888))
        
        if result == 0:
            print("[✓] WebSocket port 8888 is open")
            sock.close()
            return True
        else:
            print("[!] WebSocket port 8888 not accessible")
            sock.close()
            return False
            
    except Exception as e:
        print(f"[✗] Error: {e}")
        return False

if __name__ == "__main__":
    print("=" * 50)
    print("PROJECT NIGHTFALL - ESP32 WiFi Connection Test")
    print("=" * 50)
    
    # Run tests
    http_ok = test_wifi_connection()
    ws_ok = test_websocket_port()
    
    print("\n" + "=" * 50)
    if http_ok and ws_ok:
        print("[✓✓✓] All tests PASSED - ESP32 is ready!")
    elif http_ok:
        print("[⚠] HTTP reachable but WebSocket needs check")
    else:
        print("[✗✗✗] Tests FAILED - Check WiFi and power")
    print("=" * 50)
