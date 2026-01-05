/**
 * @file    main_camera.cpp
 * @brief   Project Nightfall - Optimized Camera Node
 * @version 3.1.0
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h" 
#include "soc/rtc_cntl_reg.h"

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#define FLASH_LED_PIN      4
#define STATUS_LED_PIN    33 

const char *ssid = "ProjectNightfall";
const char *password = "rescue2025";
const char *master_host = "192.168.4.1";
const uint16_t master_port = 8888;

WebSocketsClient webSocket;
WiFiServer streamServer(80);

unsigned long lastHeartbeat = 0;
bool flashState = false;

void setupCamera();
void handleStream();
void sendHeartbeat();
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); 
  Serial.begin(115200);
  
  pinMode(FLASH_LED_PIN, OUTPUT);
  pinMode(STATUS_LED_PIN, OUTPUT);
  digitalWrite(FLASH_LED_PIN, LOW);
  digitalWrite(STATUS_LED_PIN, HIGH);

  setupCamera();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    digitalWrite(STATUS_LED_PIN, !digitalRead(STATUS_LED_PIN)); 
  }
  
  digitalWrite(STATUS_LED_PIN, LOW); 
  streamServer.begin();

  webSocket.begin(master_host, master_port, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
}

void loop() {
  webSocket.loop();
  handleStream();

  if (millis() - lastHeartbeat > 2000) {
    sendHeartbeat();
    lastHeartbeat = millis();
  }
}

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; 
  config.jpeg_quality = 15;           
  config.fb_count = 2;
  esp_camera_init(&config);
}

void handleStream() {
  WiFiClient client = streamServer.available();
  if (!client) return;

  String req = client.readStringUntil('\r');
  if (req.indexOf("GET /stream") != -1) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
    client.println();

    while (client.connected()) {
      camera_fb_t * fb = esp_camera_fb_get();
      if (!fb) break;
      client.printf("Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
      client.write(fb->buf, fb->len);
      client.println("\r\n--frame");
      esp_camera_fb_return(fb);
      webSocket.loop(); 
    }
  }
  client.stop();
}

void sendHeartbeat() {
  char buffer[128];
  snprintf(buffer, sizeof(buffer), 
    "{\"type\":\"cam_telemetry\",\"ip\":\"%s\",\"rssi\":%d}", 
    WiFi.localIP().toString().c_str(), WiFi.RSSI());
  webSocket.sendTXT(buffer);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String msg = (char*)payload;
    if (msg.indexOf("flash_on") >= 0) digitalWrite(FLASH_LED_PIN, HIGH);
    else if (msg.indexOf("flash_off") >= 0) digitalWrite(FLASH_LED_PIN, LOW);
  }
}
