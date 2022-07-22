#ifndef MQTTDRIVER_H
#define MQTTDRIVER_H
#include "DigitalIn.h"
#include "DigitalOut.h"
#include "PinNameAliases.h"
#include "SocketAddress.h"
#include "TCPSocket.h"
#include "ThisThread.h"
#include "USBSerial.h"
#include "WiFiInterface.h"
#include "mbed.h"
#include "ESP8266Interface.h"
#include "MQTT.h"
#include <cstring>

class MQTTDriver{
    private:
        MQTTclient _client;
        uint64_t _timeout;
    public:
        bool init();
        bool connected();
        bool connect(const char*);
        bool publish(const char*, char*);
        bool receive_response(char*, char*);
        bool subscribe(const char*);
        bool ping();
        void disconnect();
        void keepalive(uint64_t);
};
#endif