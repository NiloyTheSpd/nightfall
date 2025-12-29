#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "config.h"

class WebServerHandler
{
public:
    WebServerHandler();

    void begin(const char *ssid, const char *password);
    void update();
    void stop();

    // WebSocket communication
    void broadcastMessage(const String &message);
    void sendToClient(uint8_t clientNum, const String &message);
    int getConnectedClients();

    // HTTP endpoints
    void addEndpoint(const String &path, HTTPMethod method,
                     void (*handler)(WebServer &));

    // Configuration
    void setPort(uint16_t port);
    void setWebSocketPort(uint16_t port);

    // Status
    bool isConnected();
    bool isServerRunning();

private:
    WebServer *_httpServer;
    WebSocketsServer *_webSocketServer;
    uint16_t _httpPort;
    uint16_t _webSocketPort;
    bool _running;

    // WiFi configuration
    String _ssid;
    String _password;

    // Client management
    void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
    static void webSocketEventCallback(uint8_t num, WStype_t type, uint8_t *payload, size_t length);

    // HTTP request handlers
    static void handleRoot(WebServer &server);
    static void handleStatus(WebServer &server);
    static void handleControl(WebServer &server);
    static void handleAPI(WebServer &server);
};

#endif // WEB_SERVER_H