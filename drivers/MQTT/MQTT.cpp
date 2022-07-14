#include "MQTT.h"
#include "Callback.h"
#include "ThisThread.h"
#include "mbed.h"
#include "nsapi_types.h"
#include "thread_network_data_lib.h"
#include "us_ticker_api.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <ctime>

MQTTclient::MQTTclient(NetworkInterface *interface, SocketAddress address) {
    _interface = interface;
    _address =  address;
    _connect_status = false;
}

MQTTclient::MQTTclient() {
    _connect_status = false;
}

void MQTTclient::initializeClass(NetworkInterface *interface, SocketAddress address) {
    _interface = interface;
    _address = address;
}

bool MQTTclient::connected() {
    return _connect_status;
}

bool MQTTclient::MQTTinit() {

    _result = _socket.open(_interface);
    if(_result < 0) {
        printf("Socket didn't open. Error: %d\n", _result);
        return false;
    }

    _result = _socket.connect(_address);
    if(_result < 0) {
        printf("Socket didn't connect. Error: %d\n", _result);
        return false;
    }

    _socket.set_blocking(false);

    return true;
}

bool MQTTclient::connect(const char* clientID) {
    return connect(clientID, NULL, NULL);
}

bool MQTTclient::connect(const char* clientID, const char* username) {
    return connect(clientID, username, NULL);
}

bool MQTTclient::connect( const char* clientID, const char* username, const char* password) { //implement usrname and password
    
    //uint8_t buffer[] = {0x10, 0x12, 0x00, 0x04, 'M', 'Q', 'T', 'T', 0x04, 0x02, 0x00, 0x3C, 0x00, 0x06, 'A', 'R', 'S', 'L', 'A', 'B'};

    if(!_connect_status) {
        if(!MQTTinit()) {
            printf("TCP socket initialization failed!\n");
        }
        
        uint8_t CONNECTFLAG = 0;

        if(username) {
            if(password) {
                CONNECTFLAG = 0xC2;
            } else {
                CONNECTFLAG = 0x82;
            }
        } else {
            CONNECTFLAG = 0x02;
        }
        
        uint8_t variable[10] = {0x00, 0x04, 'M', 'Q', 'T', 'T', 0x04, CONNECTFLAG, 0x00, 0x3C};

        uint8_t payload[128];
        unsigned int index = 0;

        payload[index++] = (uint8_t) (((uint16_t) strlen(clientID) & 0xFF00) >> 8);
        payload[index++] = (uint8_t) ((uint16_t) strlen(clientID) & 0x00FF);

        for(int i = 0; i < strlen(clientID); i++) {
            payload[index++] = clientID[i];
        }

        if(username) {
            payload[index++] = (uint8_t) (((uint16_t) strlen(username) & 0xFF00) >> 8);
            payload[index++] = (uint8_t) ((uint16_t) strlen(username) & 0x00FF);

            for(int i = 0; i < strlen(username); i++) {
                payload[index++] = username[i];
            }
        }

        if(password) {
            payload[index++] = (uint8_t) (((uint16_t) strlen(password) & 0xFF00) >> 8);
            payload[index++] = (uint8_t) ((uint16_t) strlen(password) & 0x00FF);

            for(int i = 0; i < strlen(password); i++) {
                payload[index++] = password[i];
            }
        }
        //Fixed header

        uint8_t fixed[2] = {(uint8_t)MQTTCONNECT, (uint8_t) (sizeof(variable) + index)};

        nsapi_size_t bytes_to_send = sizeof(fixed) + sizeof(variable) + index;
        // printf("Size of packet is: %d\n", index);

        uint8_t buffer[128];

        for(int i = 0; i < bytes_to_send; i++) {
            if(i < 2) {
                buffer[i] = fixed[i];
            } else if(i >= 2 && i < 12) {
                buffer[i] = variable[i - 2];
            } else if(i >= 12) {
                buffer[i] = payload[i - 12];
            }
        }
        
        _result = _socket.send(buffer, bytes_to_send);
        if(_result < 0) {
            printf("Send failed! Error: %d", _result);
        } else {
            // printf("sent %d bytes\r\n", _result);
        }

        uint32_t time = us_ticker_read()/1000;
        
        while(((us_ticker_read()/1000) - time) < 5000){
            if(receive_response(MQTTCONNACK)) {
                _connect_status = true;
                return true;
            }
        }

        return false;
    } else {
        return true;
    }
}

bool MQTTclient::receive_response() {
    return receive_response(NULL);
}

bool MQTTclient::receive_response(uint8_t check) {

    return receive_response(check, NULL, NULL);
}

bool MQTTclient::receive_response(char* topic, char* payload) {
    return receive_response(NULL, topic, payload);
}

bool MQTTclient::receive_response(uint8_t check, char* topic, char* payload) {
    uint8_t buffer[128];

    _result = this->_socket.recv(buffer, 128);
    if (_result < 0) {
        if(_result == -3001) {
            return false;
        } else {
            printf("Error! _socket.recv() returned: %d\n", _result);
            _connect_status = false;

            return false;
        }
        
    }

    if(buffer[0] == 0x30) { //The received packet is a publish packet (Assuming QoS is 0)

        //Deconstructing the header
        uint8_t msgLen = buffer[1];
        uint16_t topicLen = (buffer[2] << 8) | buffer[3];
        uint16_t payloadLen = msgLen - topicLen - 2;
        uint8_t topicHead = 4;
        uint8_t payloadHead = topicHead + topicLen;

        char tempBuff[64];
        int index = 0;

        for(int i = topicHead; i < (topicHead + topicLen); i++) {
            tempBuff[index++] = (char) buffer[i];
        }
        tempBuff[index] = '\0';
        strcpy(this->_global._topic,  (const char*) tempBuff);

        index = 0;
        for(int i = payloadHead; i < (payloadHead + payloadLen); i++) {
            tempBuff[index++] = buffer[i];
        }
        tempBuff[index] = '\0';
        strcpy(this->_global._payload,  (const char*) tempBuff);

        // printf("Message: %s received on topic: %s\n", this->_global._payload, this->_global._topic);

        strcpy(payload, _global._payload);
        strcpy(topic, _global._topic);

        return true;
    } else if(check) {
        if(buffer[0] == check) {
            return true;
            printf("Match");
        } else {
            return false;
            printf("No Match");
        }
    } else {
        printf("received %d bytes: ", _result);
        for(int i = 0; i < _result; i++) {
            printf("0x%02X, ", (unsigned int) buffer[i]);
        }
        printf("\n");
        return true;
    }
    return false;
}

bool MQTTclient::publish(const char* topic, const char* message) {
    
    //uint8_t buffer[] = {0x30, 0x12, 0x00, 0x04, 'T', 'E', 'S', 'T', 0x00, 0x0A, 'S', 'A', 'S', 'I', 'S', 'E', 'K', 'H', 'A', 'R'};

    if(_connect_status) {
        uint8_t variable[256];

        uint8_t index = 0;

        variable[index++] = (uint8_t) (((uint16_t) strlen(topic) & 0xFF00) >> 8);
        variable[index++] = (uint8_t) ((uint16_t) strlen(topic) & 0x00FF);

        for(int i = 0; i < strlen(topic); i++) {
            variable[index++] = topic[i];
        }

        for(int i = 0; i < strlen(message); i++) {
            variable[index++] = message[i];
        }

        uint8_t fixed[] = {MQTTPUBLISH, index};

        uint8_t buffer[512];

        for(int i = 0; i < index + 2; i++) {
            if(i < 2) {
                buffer[i] = fixed[i];
            } else {
                buffer[i] = variable[i - 2];
            }
        }

        nsapi_size_t bytes_to_send = index + 2;
        // printf("Size of packet is: %d\r\n", bytes_to_send);

        // for(int i = 0; i < bytes_to_send; i++) {
        //     printf("0x%02X, ", (unsigned int) buffer[i]);
        // }
        // printf("\n");
        
        _result = _socket.send(buffer, bytes_to_send);

        if(_result < 0) {
            printf("Publish failed! Error: %d", _result);
            return false;
        } else {
            // printf("sent %d bytes\r\n", _result);
            // printf("Published\n");
        }

        return true;
    } else {
        printf("Client not connected\n");
        return false;
    }
}

bool MQTTclient::subscribe(const char* topic) {
    
    // uint8_t buffer[] = {0x82, 0x09, 0x00, 0xFF, 0x00, 0x04, 'T', 'E', 'S', 'T', 0x00};
    // nsapi_size_t bytes_to_send = sizeof(buffer);

    uint8_t payload[128];
    uint8_t index = 0;

    payload[index++] = (uint8_t) (((uint16_t) strlen(topic) & 0xFF00) >> 8);
    payload[index++] = (uint8_t) ((uint16_t) strlen(topic) & 0x00FF);

    for(int i = 0; i < strlen(topic); i++) {
        payload[index++] = topic[i];
    }

    payload[index++] = 0x00; //QoS

    uint8_t fixed[] = {MQTTSUBSCRIBE, (uint8_t)(index + 2), 0x00, 0xFF};

    uint8_t buffer[128];

    for(int i = 0; i < (index + 4); i++) {
        if(i < 4) {
            buffer[i] = fixed[i];
        } else {
            buffer[i] = payload[i - 4];
        }
    }

    nsapi_size_t bytes_to_send = (index + 4);
    // printf("Size of packet is: %d\n", bytes_to_send);

    _result = _socket.send(buffer, bytes_to_send);
    if(_result < 0) {
        printf("Subscription unsuccessful! Error: %d\n", _result);;
        return false;
    }
    // printf("sent %d bytes\n", _result);

    uint32_t time = us_ticker_read()/1000;
    
    while(((us_ticker_read()/1000) - time) < 5000){
        if(receive_response(MQTTSUBACK)) {
            return true;
        }
    }

    return false;
}

bool MQTTclient::disconnect() {
    
    uint8_t buffer[] = {0xE0, 0x00};
    nsapi_size_t bytes_to_send = sizeof(buffer);
    // printf("Size of packet is: %d\n", bytes_to_send);

    _result = _socket.send(buffer, bytes_to_send);
    if(_result < 0) {
        printf("Broker Disconnect unsuccessful! Error:%d\n", _result);
    }

    _socket.close();
    _interface->disconnect();

    // printf("sent %d bytes\n", bytes_sent);
    printf("Network Disconnected\n");
    return true;

}

uint32_t MQTTclient::ping() {
    uint8_t buffer[] = {0xC0, 0x00};
    _socket.send(buffer, 2);

    int time = us_ticker_read()/1000;
    while((buffer[0] != MQTTPINGRESP) && (us_ticker_read()/1000 - time) <= 3000){
        _result = this->_socket.recv(buffer, 2);
    }

    uint32_t latency = us_ticker_read()/1000 - time;
    printf("Ping: %"PRIu32"ms\n", latency);
    return latency;
}

MQTTclient::~MQTTclient() {
    disconnect();
}