# Project Nightfall Enhanced Builds - Smoke Test Checklist

**Date:** January 5, 2026  
**Status:** Ready for firmware validation  
**Firmware:** main_rear_enhanced.cpp + main_front_enhanced.cpp + main_camera.cpp

---

## Pre-Build Validation

- [ ] **Config check:** `SAFE_DISTANCE` (30cm) defined in [include/config.h](include/config.h#L23-L33)
- [ ] **Pin consistency:** UART Serial2 pins 16/17 hardcoded in both enhanced sources
- [ ] **Safety logic:** `checkSafetyConditions()` properly scoped and only triggers on real hazards
- [ ] **Motor PWM:** Rear controller uses `analogWrite()` on EN pins (13/27) per pin.md
- [ ] **Heartbeat tracking:** Rear controller updates `lastFrontHeartbeat` timestamp on front messages

---

## Build & Flash Steps

### 1. Rear ESP32 (Master)

```bash
# Build enhanced rear controller
pio run -e rear_esp32

# Expected output:
# - No compile errors (SAFE_DISTANCE, SafetyMonitor should link)
# - "ENHANCED REAR ESP32 Master Controller Ready!"

# Flash to COM8 (adjust port as needed)
pio run -e rear_esp32 -t upload --upload-port COM8
```

### 2. Front ESP32 (Slave)

```bash
# Build enhanced front controller
pio run -e front_esp32

# Expected output:
# - No compile errors
# - "ENHANCED FRONT ESP32 Motor Slave Ready!"

# Flash to COM8
pio run -e front_esp32 -t upload --upload-port COM8
```

### 3. Camera ESP32 (Vision)

```bash
# Build camera module
pio run -e camera_esp32

# Flash (adjust port for your camera board)
pio run -e camera_esp32 -t upload --upload-port /dev/ttyUSB2
```

---

## Runtime Validation

### Rear ESP32 Serial Monitor (115200 baud)

```
✅ ENHANCED REAR ESP32 Master Controller Ready!
WiFi AP: ProjectNightfall
WebSocket Server: Port 8888
Dashboard URL: http://192.168.4.1
Six-Motor Architecture: 4 Front + 2 Rear

Telemetry: Front=400cm, Rear=400cm, Gas=250, Smoke=0, ...
```

### Front ESP32 Serial Monitor (115200 baud)

```
✅ ENHANCED FRONT ESP32 Motor Slave Ready!
Listening for UART commands on Serial2
Controlling 4 motors: Front Pair + Center Pair

Heartbeat sent - Emergency: NO, Front L: 0, Front R: 0, ...
```

---

## UART Communication Test

### Expected behavior (Rear → Front)

1. **Motor Command Sent:**

   ```json
   { "L": 100, "R": 100, "CL": 50, "CR": 50 }
   ```

   - Rear ESP32 transmits on GPIO17 (TX2)
   - Front ESP32 receives on GPIO16 (RX2)

2. **Heartbeat Response (Front → Rear)**

   ```json
   {"type": "heartbeat", "source": "front", "leftSpeed": 100, "rightSpeed": 100, ...}
   ```

   - Front ESP32 transmits on GPIO17 (TX2)
   - Rear ESP32 receives on GPIO16 (RX2)
   - Rear controller logs: `Heartbeat received - connectionStatus=2`

3. **Monitor for errors:**
   - [ ] No "JSON parse error" messages
   - [ ] No UART timeout logs (→ would trigger emergency stop)
   - [ ] Heartbeat arriving every ~1000ms

---

## Motor Control Validation

### Rear Motors (Direct PWM Control)

- **Pin mapping** (per pin.md):

  - **Left:** ENA=GPIO13 (PWM), IN1=GPIO14, IN2=GPIO18
  - **Right:** ENB=GPIO27 (PWM), IN3=GPIO19, IN4=GPIO23

- **Test sequence:**
  1. Send forward command via dashboard: `{"command": "forward"}`
  2. Rear ESP32 should:
     - Set `targetRearLeftSpeed = 150`, `targetRearRightSpeed = 150`
     - Call `analogWrite(13, 150)` and `analogWrite(27, 150)`
     - Set GPIO14=HIGH, GPIO18=LOW (forward)
     - Set GPIO19=HIGH, GPIO23=LOW (forward)
  3. Listen for motor hum/rotation
  4. Reverse test: `{"command": "backward"}` → GPIO14=LOW, GPIO18=HIGH, etc.

### Front Motors (UART-driven)

- **Via Rear Command:**
  1. Dashboard sends: `{"command": "forward"}`
  2. Rear transmits: `{"L": 150, "R": 150, "CL": 150, "CR": 150}`
  3. Front receives and executes:
     - Motor 1 (Front Left): GPIO13=PWM(150), GPIO23=HIGH, GPIO22=LOW
     - Motor 2 (Front Right): GPIO25=PWM(150), GPIO26=HIGH, GPIO27=LOW
     - Motor 3 (Center Left): GPIO14=PWM(150), GPIO32=HIGH, GPIO33=LOW
     - Motor 4 (Center Right): GPIO18=PWM(150), GPIO19=HIGH, GPIO21=LOW

---

## Safety System Test

### Emergency Stop Trigger

- **Obstacle detection:** Place object <20cm from ultrasonic sensor

  - Expected: Immediate emergency stop, buzzer activates
  - Check log: `"🚨 SIX-MOTOR EMERGENCY STOP ACTIVATED! Reason: Obstacle detected: 15.3cm"`

- **Gas detection:** Expose MQ-2 to cigarette smoke

  - Expected: Emergency stop triggered
  - Check log: `"Gas critical: 425"` (if > GAS_THRESHOLD_ANALOG=300)

- **Communication loss:** Disconnect front ESP32 UART
  - Expected: After 3 seconds, rear logs `"Front ESP32 heartbeat timeout"`
  - Motors should continue operating (only collision/gas stops them)

### Emergency Reset

- Send: `{"command": "emergency_reset"}`
- Expected: All emergency flags cleared, system resumes normal operation

---

## Dashboard Connectivity Test

### WiFi Connection

1. Device connects to AP **"ProjectNightfall"** (password: `rescue2025`)
2. Open browser → `http://192.168.4.1`
3. Dashboard loads with:
   - Real-time telemetry (distance, gas, battery)
   - Motor status indicators
   - Command buttons (forward/backward/left/right/stop/emergency)

### WebSocket Communication

- [ ] Dashboard receives `telemetry` messages every 500ms
- [ ] Motor commands sent via WebSocket successfully reach rear controller
- [ ] Front ESP32 online status displayed correctly

---

## Known Limitations & Future Work

| Issue                                                   | Status         | Workaround                                     |
| ------------------------------------------------------- | -------------- | ---------------------------------------------- |
| Non-enhanced rear/front (main_rear.cpp, main_front.cpp) | Deprecated     | Use only enhanced versions                     |
| UART echo buffer                                        | Not tested     | Monitor for JSON corruption under high traffic |
| Motor current sensing                                   | Stub only      | Optional: add ACS712 for real-time monitoring  |
| ML inference                                            | Not integrated | Camera → ML → detection path TBD               |
| Battery BMS                                             | No protection  | Use external LiPo alarm (mandatory)            |

---

## Sign-Off

- [ ] All builds compile without errors
- [ ] Rear ESP32 boots and creates WiFi AP
- [ ] Front ESP32 boots and listens on UART
- [ ] UART heartbeats exchanged successfully
- [ ] Motors respond to PWM commands
- [ ] Safety system triggers on collision/gas
- [ ] Dashboard loads and receives telemetry
- [ ] WebSocket commands control robot movement

**Ready for field testing? ⬜ Not started ⬜ In progress ✅ Complete**

---

**Next phase:** Integration testing with physical 6WD platform + battery pack.
