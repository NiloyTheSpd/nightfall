#!/usr/bin/env python3
"""
Motor Control via WiFi WebSocket
Sends motor control commands to rear ESP32 via WebSocket
Requires: pip install websocket-client
"""

import json
import time
import sys

try:
    from websocket import create_connection, WebSocketException
except ImportError:
    print("[!] websocket-client not installed")
    print("[!] Install with: pip install websocket-client")
    sys.exit(1)

# Configuration
ESP32_IP = "192.168.4.1"
WEBSOCKET_URL = f"ws://{ESP32_IP}:8888/"

def connect_websocket():
    """Establish WebSocket connection to ESP32"""
    try:
        print(f"[TEST] Connecting to WebSocket at {WEBSOCKET_URL}...")
        ws = create_connection(WEBSOCKET_URL, timeout=5)
        print("[✓] WebSocket connected!")
        return ws
    except Exception as e:
        print(f"[✗] WebSocket connection failed: {e}")
        return None

def send_motor_command(ws, left_speed, right_speed, center_left=0, center_right=0):
    """
    Send motor control command to ESP32
    
    Parameters:
    - left_speed: Rear left motor (-255 to 255)
    - right_speed: Rear right motor (-255 to 255)
    - center_left: Front left motors (0-255)
    - center_right: Front right motors (0-255)
    """
    cmd = {
        "L": left_speed,
        "R": right_speed,
        "CL": center_left,
        "CR": center_right
    }
    
    try:
        msg = json.dumps(cmd)
        ws.send(msg)
        print(f"[→] Sent: {msg}")
        return True
    except Exception as e:
        print(f"[✗] Send failed: {e}")
        return False

def test_motor_sequence(ws):
    """Run a simple motor test sequence"""
    print("\n[TEST] Starting motor control sequence...")
    
    test_cases = [
        ("Forward (50%)", 128, 128, 0, 0, 1.0),
        ("Backward (50%)", -128, -128, 0, 0, 1.0),
        ("Turn Left", 50, 200, 0, 0, 1.0),
        ("Turn Right", 200, 50, 0, 0, 1.0),
        ("Stop", 0, 0, 0, 0, 0.5),
    ]
    
    for test_name, l_speed, r_speed, cl_speed, cr_speed, duration in test_cases:
        print(f"\n[>] {test_name}")
        if send_motor_command(ws, l_speed, r_speed, cl_speed, cr_speed):
            time.sleep(duration)
        else:
            print(f"[!] Skipping {test_name}")
    
    # Final stop
    print("\n[>] Final Stop")
    send_motor_command(ws, 0, 0, 0, 0)
    print("[✓] Motor test sequence complete")

def read_telemetry(ws):
    """Read incoming telemetry from ESP32"""
    try:
        ws.settimeout(2)
        data = ws.recv(1024)
        if data:
            print(f"[←] Received: {data}")
            return json.loads(data)
    except Exception as e:
        print(f"[!] No telemetry received: {e}")
    return None

if __name__ == "__main__":
    print("=" * 50)
    print("PROJECT NIGHTFALL - Motor Control via WiFi")
    print("=" * 50)
    
    # Connect to WebSocket
    ws = connect_websocket()
    if not ws:
        print("[✗✗✗] Failed to connect - Test FAILED")
        sys.exit(1)
    
    try:
        # Read initial telemetry
        print("\n[TEST] Reading ESP32 telemetry...")
        telemetry = read_telemetry(ws)
        
        # Run motor test
        test_motor_sequence(ws)
        
        # Read final telemetry
        print("\n[TEST] Reading final telemetry...")
        telemetry = read_telemetry(ws)
        
        print("\n" + "=" * 50)
        print("[✓✓✓] Motor test PASSED!")
        print("=" * 50)
        
    except KeyboardInterrupt:
        print("\n[!] Test interrupted by user")
        send_motor_command(ws, 0, 0, 0, 0)
    finally:
        ws.close()
        print("[✓] WebSocket closed")
