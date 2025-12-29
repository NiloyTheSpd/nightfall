#ifndef UART_COMM_H
#define UART_COMM_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

class UARTComm
{
public:
    UARTComm(HardwareSerial &serial, uint32_t baudRate);

    void begin();
    bool available();
    void sendMessage(const JsonDocument &doc);
    StaticJsonDocument<MAX_JSON_DOCUMENT_SIZE> receiveMessage();
    unsigned long getLastReceived();

private:
    HardwareSerial &_serial;
    uint32_t _baudRate;
    unsigned long _lastReceived;

    String readLine();
};

#endif // UART_COMM_H