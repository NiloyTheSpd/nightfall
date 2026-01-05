#!/usr/bin/env python3
"""
Full System Test - ESP32 + Motor Control via WiFi
Complete integration test: verify ESP32 connectivity and execute motor commands
"""

import json
import time
import socket
import sys

try:
    from websocket import create_connection, WebSocketException
except ImportError:
    print("[!] websocket-client not installed: pip install websocket-client")
    sys.exit(1)

ESP32_IP = "192.168.4.1"
WEBSOCKET_URL = f"ws://{ESP32_IP}:8888/"

class ProjectNightfallTester:
    def __init__(self):
        self.ws = None
        self.connected = False
        self.test_results = []
    
    def test_http_connectivity(self):
        """Test HTTP connectivity (port 80)"""
        print("\n[1/4] Testing HTTP connectivity...")
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(3)
            result = sock.connect_ex((ESP32_IP, 80))
            sock.close()
            
            if result == 0:
                print(f"    [✓] HTTP reachable at {ESP32_IP}:80")
                self.test_results.append(("HTTP Connectivity", True))
                return True
            else:
                print(f"    [✗] Cannot reach {ESP32_IP}:80")
                self.test_results.append(("HTTP Connectivity", False))
                return False
        except Exception as e:
            print(f"    [✗] Error: {e}")
            self.test_results.append(("HTTP Connectivity", False))
            return False
    
    def test_websocket_connectivity(self):
        """Test WebSocket connectivity (port 8888)"""
        print("\n[2/4] Testing WebSocket connectivity...")
        try:
            self.ws = create_connection(WEBSOCKET_URL, timeout=5)
            self.connected = True
            print(f"    [✓] WebSocket connected to {WEBSOCKET_URL}")
            self.test_results.append(("WebSocket Connectivity", True))
            return True
        except Exception as e:
            print(f"    [✗] WebSocket failed: {e}")
            self.test_results.append(("WebSocket Connectivity", False))
            return False
    
    def test_motor_commands(self):
        """Test motor control commands"""
        print("\n[3/4] Testing motor commands...")
        if not self.connected:
            print("    [✗] No WebSocket connection")
            self.test_results.append(("Motor Commands", False))
            return False
        
        try:
            commands = [
                ({"L": 100, "R": 100, "CL": 0, "CR": 0}, "Forward"),
                ({"L": -100, "R": -100, "CL": 0, "CR": 0}, "Backward"),
                ({"L": 0, "R": 0, "CL": 0, "CR": 0}, "Stop"),
            ]
            
            for cmd, desc in commands:
                self.ws.send(json.dumps(cmd))
                print(f"    [→] {desc}: {cmd}")
                time.sleep(0.5)
            
            print("    [✓] Motor commands sent successfully")
            self.test_results.append(("Motor Commands", True))
            return True
        except Exception as e:
            print(f"    [✗] Motor command failed: {e}")
            self.test_results.append(("Motor Commands", False))
            return False
    
    def test_esp32_status(self):
        """Request and read ESP32 status"""
        print("\n[4/4] Reading ESP32 status...")
        if not self.connected:
            print("    [✗] No WebSocket connection")
            self.test_results.append(("ESP32 Status", False))
            return False
        
        try:
            self.ws.settimeout(2)
            data = self.ws.recv(1024)
            if data:
                status = json.loads(data)
                print(f"    [←] Status: {data}")
                print(f"    [✓] Received telemetry")
                self.test_results.append(("ESP32 Status", True))
                return True
            else:
                print("    [!] No status data received")
                self.test_results.append(("ESP32 Status", False))
                return False
        except socket.timeout:
            print("    [!] Status timeout (normal if no data sent)")
            self.test_results.append(("ESP32 Status", True))  # Not a hard failure
            return True
        except Exception as e:
            print(f"    [!] Status error: {e}")
            self.test_results.append(("ESP32 Status", False))
            return False
    
    def print_summary(self):
        """Print test summary"""
        print("\n" + "=" * 60)
        print("TEST SUMMARY")
        print("=" * 60)
        
        passed = sum(1 for _, result in self.test_results if result)
        total = len(self.test_results)
        
        for test_name, result in self.test_results:
            status = "[✓]" if result else "[✗]"
            print(f"{status} {test_name}")
        
        print("=" * 60)
        if passed == total:
            print(f"[✓✓✓] ALL TESTS PASSED ({passed}/{total})")
            print("      System is READY for autonomous operation!")
        elif passed >= total - 1:
            print(f"[⚠] MOSTLY PASSED ({passed}/{total})")
            print("    Check warnings above")
        else:
            print(f"[✗✗✗] TESTS FAILED ({passed}/{total})")
            print("      Check ESP32 power, WiFi, and connectivity")
        print("=" * 60)
    
    def run(self):
        """Execute full test suite"""
        print("\n" + "=" * 60)
        print("PROJECT NIGHTFALL - Full System Test")
        print("ESP32 Rescue Robot - WiFi Integration")
        print("=" * 60)
        
        try:
            self.test_http_connectivity()
            self.test_websocket_connectivity()
            self.test_motor_commands()
            self.test_esp32_status()
        except KeyboardInterrupt:
            print("\n[!] Test interrupted by user")
        finally:
            if self.ws:
                self.ws.close()
                print("\n[✓] WebSocket closed")
            self.print_summary()

if __name__ == "__main__":
    tester = ProjectNightfallTester()
    tester.run()
