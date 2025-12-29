# Project Debug Rules (Non-Obvious Only)

**Serial Monitor Commands:** Use single letter commands from serial monitor: 'f' (forward), 'b' (backward), 'l' (left), 'r' (right), 's' (stop), 't' (360° rotate), 'u' (ultrasonic readings), 'g' (gas sensor)

**Multi-Board Debugging:** Each ESP32 requires separate `pio device monitor` session with different environment flags (-e front_esp32, -e rear_esp32, -e camera_esp32)

**ESP32-CAM Debugging:** Camera stream accessible at http://192.168.4.1:81 after connecting to "ProjectNightfall" WiFi - not obvious from file structure

**Communication Timeout:** Watchdog timeout set to 5000ms - ESP32s will emergency stop if communication lost for >5 seconds

**MQ-2 Gas Sensor:** False positives common during first 24-48 hours - sensor requires extensive preheating period

**Brownout Detection:** ESP32s may reset under high motor load - add 1000µF capacitor near VIN pins if experiencing random resets

**ESP32 GPIO Voltage Warning:** ESP32 GPIO pins accept ONLY 3.3V signals - connecting 5V sensors directly will damage the chip. Use voltage dividers (1kΩ + 2kΩ) or logic level shifters for 5V sensors.

**UART Communication:** Master ESP32 #1 sends heartbeat every 1000ms - rear controller will emergency stop if heartbeat timeout exceeded

**LED Status Codes:** Onboard LED patterns indicate system state - slow blink (autonomous), fast blink (initializing), triple blink (sensor error), SOS pattern (critical failure)