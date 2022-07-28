#ifndef MQTT_h
#define MQTT_h

#include "DigitalOut.h"
#include "NetworkInterface.h"
#include "SocketAddress.h"
#include "TCPSocket.h"
#include "mbed.h"

#define MQTTCONNECT     0x10    // Client request to connect to Server
#define MQTTCONNACK     0x20    // Connect Acknowledgment
#define MQTTPUBLISH     0x30    // Publish message (QoS and DUP flags are set to 0)
#define MQTTPUBACK      0x40    // Publish Acknowledgment
#define MQTTPUBREC      0x50    // Publish Received (assured delivery part 1)
#define MQTTPUBREL      0x62    // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     0x70    // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   0x82    // Client Subscribe request
#define MQTTSUBACK      0x90    // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 0xA2    // Client Unsubscribe request
#define MQTTUNSUBACK    0xB0    // Unsubscribe Acknowledgment
#define MQTTPINGREQ     0xC0    // PING Request
#define MQTTPINGRESP    0xD0    // PING Response
#define MQTTDISCONNECT  0xE0    // Client is Disconnecting
#define MQTTReserved    0xF0     // Reserved

class MQTTclient {
    private:
    bool _connect_status;
    NetworkInterface *_interface;
    TCPSocket _socket;
    SocketAddress _address;
    nsapi_size_or_error_t _result;

    struct Message {
        char _clientID[64];
        char _topic[64];
        char _payload[64];
    } _global;

    public:
    MQTTclient(NetworkInterface*, SocketAddress);
    MQTTclient();
    ~MQTTclient();
    void initializeClass(NetworkInterface*, SocketAddress);
    bool connected();
    bool MQTTinit();
    bool connect(const char* , const char* , const char*);
    bool connect(const char* , const char*);
    bool connect(const char*);
    bool receive_response();
    bool receive_response(uint8_t);
    bool receive_response(char* topic, char* payload);
    bool receive_response(uint8_t, char*, char*);
    bool publish(const char* , const char*);
    bool subscribe(const char*);
    uint32_t ping(bool);
    bool disconnect();
};

#endif