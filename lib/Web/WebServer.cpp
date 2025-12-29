#include "WebServer.h"

WebServerHandler::WebServerHandler()
    : _httpServer(nullptr), _webSocketServer(nullptr),
      _httpPort(HTTP_PORT), _webSocketPort(WEBSOCKET_PORT), _running(false) {}

void WebServerHandler::begin(const char *ssid, const char *password)
{
    _ssid = String(ssid);
    _password = String(password);

    DEBUG_PRINTLN("Setting up WiFi access point...");

    // Create access point
    WiFi.softAP(ssid, password);
    IPAddress IP = WiFi.softAPIP();

    DEBUG_PRINT("Access Point IP: ");
    DEBUG_PRINTLN(IP);

    // Create HTTP server
    _httpServer = new WebServer(_httpPort);

    // Add basic endpoints
    _httpServer->on("/", HTTP_GET, handleRoot);
    _httpServer->on("/api/status", HTTP_GET, handleStatus);
    _httpServer->on("/api/control", HTTP_POST, handleControl);
    _httpServer->on("/api/<endpoint>", HTTP_GET, handleAPI);

    // Start HTTP server
    _httpServer->begin();

    // Create WebSocket server
    _webSocketServer = new WebSocketsServer(_webSocketPort);
    _webSocketServer->begin();
    _webSocketServer->onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                              { onWebSocketEvent(num, type, payload, length); });

    _running = true;

    DEBUG_PRINTLN("Web server started");
    DEBUG_PRINT("HTTP Server: http://");
    DEBUG_PRINTLN(IP);
    DEBUG_PRINT("WebSocket Port: ");
    DEBUG_PRINTLN(_webSocketPort);
}

void WebServerHandler::update()
{
    if (!_running)
    {
        return;
    }

    // Handle HTTP requests
    if (_httpServer)
    {
        _httpServer->handleClient();
    }

    // Handle WebSocket connections
    if (_webSocketServer)
    {
        _webSocketServer->loop();
    }
}

void WebServerHandler::stop()
{
    if (_running)
    {
        if (_httpServer)
        {
            _httpServer->stop();
            delete _httpServer;
            _httpServer = nullptr;
        }

        if (_webSocketServer)
        {
            _webSocketServer->close();
            delete _webSocketServer;
            _webSocketServer = nullptr;
        }

        _running = false;
        DEBUG_PRINTLN("Web server stopped");
    }
}

void WebServerHandler::broadcastMessage(const String &message)
{
    if (_webSocketServer && _running)
    {
        _webSocketServer->broadcastTXT(message);
    }
}

void WebServerHandler::sendToClient(uint8_t clientNum, const String &message)
{
    if (_webSocketServer && _running)
    {
        _webSocketServer->sendTXT(clientNum, message);
    }
}

int WebServerHandler::getConnectedClients()
{
    if (_webSocketServer && _running)
    {
        return _webSocketServer->connectedClients();
    }
    return 0;
}

void WebServerHandler::addEndpoint(const String &path, HTTPMethod method,
                                   void (*handler)(WebServer &))
{
    if (_httpServer && _running)
    {
        _httpServer->on(path, method, [handler, this]()
                        { handler(*_httpServer); });
    }
}

void WebServerHandler::setPort(uint16_t port)
{
    _httpPort = port;
}

void WebServerHandler::setWebSocketPort(uint16_t port)
{
    _webSocketPort = port;
}

bool WebServerHandler::isConnected()
{
    return WiFi.status() == WL_CONNECTED || WiFi.getMode() == WIFI_MODE_AP;
}

bool WebServerHandler::isServerRunning()
{
    return _running;
}

void WebServerHandler::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_CONNECTED:
        DEBUG_PRINT("WebSocket client ");
        DEBUG_PRINT(num);
        DEBUG_PRINTLN(" connected");
        break;
    case WStype_DISCONNECTED:
        DEBUG_PRINT("WebSocket client ");
        DEBUG_PRINT(num);
        DEBUG_PRINTLN(" disconnected");
        break;
    case WStype_TEXT:
        String message = String((char *)payload);
        DEBUG_PRINT("WebSocket message from client ");
        DEBUG_PRINT(num);
        DEBUG_PRINT(": ");
        DEBUG_PRINTLN(message);
        break;
    case WStype_ERROR:
        DEBUG_PRINTLN("WebSocket error");
        break;
    default:
        break;
    }
}

void WebServerHandler::handleRoot(WebServer &server)
{
    server.send(200, "text/html",
                "<!DOCTYPE html>"
                "<html><head><title>Project Nightfall Dashboard</title>"
                "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>"
                "<style>"
                "body{font-family:Arial;margin:20px;background:#1a1a1a;color:#fff}"
                ".container{max-width:1200px;margin:0 auto}"
                ".card{background:#2d2d2d;border-radius:10px;padding:20px;margin:10px 0}"
                ".status{display:inline-block;padding:5px 15px;border-radius:20px;margin:5px}"
                ".normal{background:#28a745}.warning{background:#ffc107;color:#000}.error{background:#dc3545}"
                ".button{background:#007bff;color:white;border:none;padding:10px 20px;margin:5px;border-radius:5px;cursor:pointer}"
                ".button:hover{background:#0056b3}"
                ".grid{display:grid;grid-template-columns:1fr 1fr;gap:20px}"
                "</style></head>"
                "<body><div class='container'>"
                "<h1>🤖 Project Nightfall - Autonomous Rescue Robot</h1>"
                "<div class='grid'>"
                "<div class='card'><h2>System Status</h2>"
                "<div>State: <span id='state' class='status normal'>IDLE</span></div>"
                "<div>Uptime: <span id='uptime'>0s</span></div>"
                "<div>Emergency: <span id='emergency' class='status normal'>NO</span></div>"
                "<div>Connected Clients: <span id='clients'>0</span></div></div>"
                "<div class='card'><h2>Sensor Data</h2>"
                "<div>Front Distance: <span id='frontDistance'>0</span> cm</div>"
                "<div>Rear Distance: <span id='rearDistance'>0</span> cm</div>"
                "<div>Gas Level: <span id='gasLevel'>0</span></div>"
                "<div>Battery: <span id='battery'>0</span> V</div></div>"
                "<div class='card'><h2>Motor Control</h2>"
                "<div>Left Speed: <span id='leftSpeed'>0</span></div>"
                "<div>Right Speed: <span id='rightSpeed'>0</span></div>"
                "<div>Moving: <span id='moving'>NO</span></div></div>"
                "<div class='card'><h2>Control Panel</h2>"
                "<button class='button' onclick='sendCommand(\"autonomous\")'>Start Autonomous</button>"
                "<button class='button' onclick='sendCommand(\"stop\")'>Stop</button>"
                "<button class='button' onclick='sendCommand(\"emergency\")'>Emergency Stop</button>"
                "<button class='button' onclick='sendCommand(\"forward\")'>Forward</button>"
                "<button class='button' onclick='sendCommand(\"backward\")'>Backward</button>"
                "<button class='button' onclick='sendCommand(\"left\")'>Turn Left</button>"
                "<button class='button' onclick='sendCommand(\"right\")'>Turn Right</button></div>"
                "</div></div>"
                "<script>"
                "var ws = new WebSocket('ws://' + window.location.hostname + ':81');"
                "ws.onopen = function(){"
                "console.log('Connected to robot');"
                "document.getElementById('clients').textContent = '1';"
                "};"
                "ws.onclose = function(){"
                "console.log('Disconnected from robot');"
                "document.getElementById('clients').textContent = '0';"
                "};"
                "ws.onmessage = function(event){"
                "try {"
                "var data = JSON.parse(event.data);"
                "updateDashboard(data);"
                "} catch(e) {"
                "console.log('Invalid JSON:', event.data);"
                "}"
                "};"
                "function updateDashboard(data){"
                "document.getElementById('state').textContent = data.state || 'UNKNOWN';"
                "document.getElementById('uptime').textContent = Math.floor((data.uptime || 0)/1000) + 's';"
                "document.getElementById('emergency').textContent = data.emergency ? 'YES' : 'NO';"
                "document.getElementById('frontDistance').textContent = data.frontDistance || '0';"
                "document.getElementById('rearDistance').textContent = data.rearDistance || '0';"
                "document.getElementById('gasLevel').textContent = data.gasLevel || '0';"
                "document.getElementById('battery').textContent = data.batteryVoltage || '0';"
                "document.getElementById('leftSpeed').textContent = data.leftMotorSpeed || '0';"
                "document.getElementById('rightSpeed').textContent = data.rightMotorSpeed || '0';"
                "document.getElementById('moving').textContent = (data.leftMotorSpeed || 0) != 0 || (data.rightMotorSpeed || 0) != 0 ? 'YES' : 'NO';"
                "}"
                "function sendCommand(cmd){"
                "ws.send(JSON.stringify({command: cmd}));"
                "console.log('Command sent:', cmd);"
                "}"
                "</script></body></html>");
}

void WebServerHandler::handleStatus(WebServer &server)
{
    DynamicJsonDocument doc(1024);
    doc["status"] = "online";
    doc["timestamp"] = millis();
    doc["version"] = VERSION_STRING;

    String response;
    serializeJson(doc, response);
    server.send(200, "application/json", response);
}

void WebServerHandler::handleControl(WebServer &server)
{
    if (server.hasArg("plain"))
    {
        DynamicJsonDocument doc(512);
        deserializeJson(doc, server.arg("plain"));

        String command = doc["command"];

        DEBUG_PRINT("Control command received: ");
        DEBUG_PRINTLN(command);

        server.send(200, "application/json", "{\"status\":\"ok\",\"command\":\"" + command + "\"}");
    }
    else
    {
        server.send(400, "application/json", "{\"error\":\"No command provided\"}");
    }
}

void WebServerHandler::handleAPI(WebServer &server)
{
    server.send(200, "application/json", "{\"message\":\"API endpoint\"}");
}