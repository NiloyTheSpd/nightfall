/**
 * @file    main_rear_enhanced.cpp
 * @brief   Project Nightfall - V4.1 Ultimate Dashboard with Sensors
 * @details Standalone AP Mode (192.168.4.1) | Sonar + Gas | Tank Steering
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// --- পিন কনফিগারেশন (আপনার কানেকশন অনুযায়ী) ---
// Rear Left Motor
#define ENA 13 // PWM Speed
#define IN1 14 // Direction 1
#define IN2 18 // Direction 2

// Rear Right Motor
#define IN3 23 // Direction 1
#define IN4 19 // Direction 2
#define ENB 27 // PWM Speed

// Sensors
#define TRIG_PIN 4
#define ECHO_PIN 36
#define GAS_PIN 32

// --- গ্লোবাল ভেরিয়েবল ---
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
int speed = 255; // ফুল স্পিড (Max Power)

// --- ড্যাশবোর্ড ডিজাইন (HTML/CSS/JS) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>NIGHTFALL COMMANDER</title>
  <style>
    /* Cyberpunk Theme */
    :root { --primary: #00f3ff; --danger: #ff003c; --bg: #050a14; --glass: rgba(16, 33, 60, 0.9); }
    body { background-color: var(--bg); color: var(--primary); font-family: 'Courier New', monospace; margin: 0; padding: 10px; text-align: center; overflow-x: hidden; touch-action: manipulation; }
    
    /* Header */
    h2 { letter-spacing: 4px; text-shadow: 0 0 10px var(--primary); margin: 10px 0; border-bottom: 1px solid var(--primary); display: inline-block; padding-bottom: 5px; }
    .status-badge { font-size: 10px; background: var(--primary); color: #000; padding: 2px 6px; border-radius: 2px; font-weight: bold; }

    /* Video Feed */
    .feed-box {
      position: relative; width: 100%; max-width: 640px; height: 360px; margin: 0 auto 15px;
      background: #000; border: 2px solid #333; border-radius: 8px; overflow: hidden;
      box-shadow: 0 0 20px rgba(0, 243, 255, 0.1);
    }
    img { width: 100%; height: 100%; object-fit: contain; } 
    .overlay { position: absolute; top: 10px; left: 10px; background: rgba(0,0,0,0.7); padding: 4px 8px; border-left: 3px solid var(--primary); font-size: 12px; }

    /* Sensor Dashboard */
    .sensor-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; max-width: 600px; margin: 0 auto 20px; }
    .card { background: var(--glass); padding: 10px; border-radius: 8px; border: 1px solid #333; }
    .label { font-size: 10px; color: #888; display: block; margin-bottom: 5px; text-transform: uppercase; }
    .value { font-size: 24px; font-weight: bold; }
    .unit { font-size: 12px; color: #666; }
    
    /* Progress Bars */
    .bar-bg { width: 100%; height: 6px; background: #333; margin-top: 5px; border-radius: 3px; overflow: hidden; }
    .bar-fill { height: 100%; background: var(--primary); width: 0%; transition: width 0.3s; }
    .danger .bar-fill { background: var(--danger); }
    .danger .value { color: var(--danger); text-shadow: 0 0 10px var(--danger); }

    /* Control Pad */
    .controls { display: grid; grid-template-columns: repeat(3, 1fr); gap: 10px; max-width: 320px; margin: 0 auto; user-select: none; }
    .btn {
      background: rgba(255,255,255,0.05); border: 1px solid var(--primary); color: var(--primary);
      padding: 25px 0; border-radius: 8px; font-size: 28px; cursor: pointer; transition: 0.1s;
      -webkit-tap-highlight-color: transparent;
    }
    .btn:active { background: var(--primary); color: #000; box-shadow: 0 0 15px var(--primary); transform: scale(0.95); }
    .stop { border-color: var(--danger); color: var(--danger); grid-column: 2; font-weight: bold; font-size: 18px; line-height: 2.5; }
    .stop:active { background: var(--danger); color: #fff; box-shadow: 0 0 15px var(--danger); }

  </style>
</head>
<body>

  <h2>NIGHTFALL <span class="status-badge">ONLINE</span></h2>

  <div class="feed-box">
    <div class="overlay">CAM: LIVE</div>
    <img src="http://192.168.4.2/stream" onerror="this.src=''; this.style.display='none'; document.querySelector('.overlay').innerText='CAM: OFFLINE';">
  </div>

  <div class="sensor-grid">
    <div class="card" id="sonar-card">
      <span class="label">Distance</span>
      <span class="value" id="dist">--</span> <span class="unit">CM</span>
      <div class="bar-bg"><div class="bar-fill" id="dist-bar"></div></div>
    </div>
    
    <div class="card" id="gas-card">
      <span class="label">Gas Level</span>
      <span class="value" id="gas">--</span>
      <div class="bar-bg"><div class="bar-fill" id="gas-bar"></div></div>
    </div>
  </div>

  <div class="controls">
    <div></div>
    <button class="btn" ontouchstart="s('F')" ontouchend="s('S')" onmousedown="s('F')" onmouseup="s('S')">▲</button>
    <div></div>
    
    <button class="btn" ontouchstart="s('L')" ontouchend="s('S')" onmousedown="s('L')" onmouseup="s('S')">◄</button>
    <button class="btn stop" onclick="s('S')">STOP</button>
    <button class="btn" ontouchstart="s('R')" ontouchend="s('S')" onmousedown="s('R')" onmouseup="s('S')">►</button>
    
    <div></div>
    <button class="btn" ontouchstart="s('B')" ontouchend="s('S')" onmousedown="s('B')" onmouseup="s('S')">▼</button>
    <div></div>
  </div>

  <script>
    var ws = new WebSocket(`ws://${window.location.hostname}/ws`);
    
    ws.onmessage = function(event) {
      var d = JSON.parse(event.data);
      updateSensors(d.d, d.g);
    };

    // Send Command
    function s(c) { if(ws.readyState === 1) ws.send(c); }

    function updateSensors(dist, gas) {
      // Distance Logic
      document.getElementById('dist').innerText = dist;
      var dBar = document.getElementById('dist-bar');
      var dCard = document.getElementById('sonar-card');
      
      var dPct = Math.min((dist / 100) * 100, 100);
      dBar.style.width = dPct + "%";

      if(dist < 20 && dist > 0) { // Danger Zone
        dCard.classList.add('danger');
      } else {
        dCard.classList.remove('danger');
      }

      // Gas Logic
      document.getElementById('gas').innerText = gas;
      var gBar = document.getElementById('gas-bar');
      var gCard = document.getElementById('gas-card');
      
      var gPct = Math.min((gas / 3000) * 100, 100);
      gBar.style.width = gPct + "%";

      if(gas > 2500) { // Smoke Detected
        gCard.classList.add('danger');
      } else {
        gCard.classList.remove('danger');
      }
    }

    // Keyboard Support
    document.addEventListener('keydown', e => {
      if(e.repeat) return;
      if(e.key=='w') s('F');
      if(e.key=='s') s('B');
      if(e.key=='a') s('L');
      if(e.key=='d') s('R');
      if(e.key==' ') s('S');
    });
    document.addEventListener('keyup', e => {
      if(['w','a','s','d'].includes(e.key)) s('S');
    });
  </script>
</body>
</html>
)rawliteral";

// --- মোটর লজিক (Steering Logic) ---
void stopMotors()
{
    // Rear Stop
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
    // Front Stop
    Serial2.println("{\"L\":0,\"R\":0}");
}

void moveForward()
{
    // Rear Forward
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, speed);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, speed);
    // Front Forward
    Serial2.println("{\"L\":255,\"R\":255}");
}

void moveBackward()
{
    // Rear Backward
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, speed);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, speed);
    // Front Backward
    Serial2.println("{\"L\":-255,\"R\":-255}");
}

void turnLeft()
{
    // TANK TURN: Left Backward, Right Forward
    // Rear
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    analogWrite(ENA, speed); // Left Back
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, speed); // Right Fwd
    // Front
    Serial2.println("{\"L\":-255,\"R\":255}");
}

void turnRight()
{
    // TANK TURN: Left Forward, Right Backward
    // Rear
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, speed); // Left Fwd
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    analogWrite(ENB, speed); // Right Back
    // Front
    Serial2.println("{\"L\":255,\"R\":-255}");
}

// --- WebSocket হ্যান্ডলার ---
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len)
        {
            data[len] = 0;
            char cmd = (char)data[0];
            if (cmd == 'F')
                moveForward();
            else if (cmd == 'B')
                moveBackward();
            else if (cmd == 'L')
                turnLeft();
            else if (cmd == 'R')
                turnRight();
            else if (cmd == 'S')
                stopMotors();
        }
    }
}

void setup()
{
    Serial.begin(115200);
    // Front ESP32 এর সাথে কথা বলার জন্য Serial2
    Serial2.begin(115200, SERIAL_8N1, 16, 17);

    // Motors Setup
    pinMode(ENA, OUTPUT);
    pinMode(ENB, OUTPUT);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // Sensors Setup
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(GAS_PIN, INPUT);

    // WiFi Access Point (হটস্পট)
    WiFi.softAP("ProjectNightfall", "12345678");
    Serial.println("Dashboard IP: http://192.168.4.1");

    // Server Routes
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });
    ws.onEvent(onEvent);
    server.addHandler(&ws);
    server.begin();
}

void loop()
{
    ws.cleanupClients();

    // সেন্সর আপডেট (প্রতি ১০০ মিলি সেকেন্ডে)
    static unsigned long lastTime = 0;
    if (millis() - lastTime > 100)
    {

        // 1. সোনার রিডিং (Sonar Reading)
        digitalWrite(TRIG_PIN, LOW);
        delayMicroseconds(2);
        digitalWrite(TRIG_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(TRIG_PIN, LOW);
        long duration = pulseIn(ECHO_PIN, HIGH, 25000); // 25ms Timeout
        int distance = (duration == 0) ? 0 : duration * 0.034 / 2;

        // 2. গ্যাস রিডিং (Gas Reading)
        int gas = analogRead(GAS_PIN);

        // 3. ড্যাশবোর্ডে পাঠানো
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "{\"d\":%d,\"g\":%d}", distance, gas);
        ws.textAll(buffer);

        lastTime = millis();
    }
}