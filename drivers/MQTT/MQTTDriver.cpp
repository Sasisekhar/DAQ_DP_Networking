#include "MQTTDriver.h"
#include "ESP8266Interface.h"
#include "mbed.h"

namespace global {
    #ifdef DAQ_ETH
        NetworkInterface *interface;
    #else
        ESP8266Interface _ESPclient(D1, D0);
    #endif

}

namespace arduino{
    uint64_t millis() {
        return (us_ticker_read()/1000);
    }
}

bool MQTTDriver::init() {

    #ifdef DAQ_ETH
        global::interface = NetworkInterface::get_default_instance(); 
        global::interface->connect();
        printf("\r\nConnected!\r\n");
    #else
    global::_ESPclient.set_credentials((const char*)"Sx3K", (const char*)"golikuttan7577", NSAPI_SECURITY_WPA_WPA2);
    // global::_ESPclient.set_credentials((const char*)"ARS-LAB", (const char*)"3928DC6C25", NSAPI_SECURITY_WEP);
    global::_ESPclient.connect();
    printf("Connecting");
        while(global::_ESPclient.get_connection_status() == NSAPI_STATUS_CONNECTING){
            printf(".");
            ThisThread::sleep_for(500ms);
        }
        if(global::_ESPclient.get_connection_status() == NSAPI_STATUS_GLOBAL_UP) {
            printf("\r\nConnected!\r\n");
        }
    #endif

    SocketAddress address;

    #ifdef DAQ_ETH
        global::interface->gethostbyname("broker.hivemq.com", &address);
    #else
        global::_ESPclient.gethostbyname("broker.hivemq.com", &address);
    #endif

    // global::_ESPclient.gethostbyname("mqtt.flespi.io", &address);
    // address.set_ip_address("192.168.173.46\0");  //My laptop
    // address.set_ip_address("192.168.137.1\0");  //My laptop
    // address.set_ip_address("172.17.23.254\0");  //My laptop
    // address.set_ip_address("134.117.52.231\0");     //My workstation
    address.set_port(1883);

    #ifdef DAQ_ETH
        _client.initializeClass(global::interface, address);    
    #else
        _client.initializeClass(&global::_ESPclient, address);
    #endif

    return true;
}

bool MQTTDriver::connect(const char* clientID) {
    _timeout = arduino::millis();
    return _client.connect(clientID);
}

bool MQTTDriver::publish(const char* topic, char* message) {
    _timeout = (us_ticker_read()/1000);
    return _client.publish(topic, message);
}

bool MQTTDriver::receive_response(char* topic, char* message) {
    return _client.receive_response(topic, message);
}

bool MQTTDriver::subscribe(const char* topic) {
    return _client.subscribe(topic);
}

bool MQTTDriver::ping() {
    return _client.ping(true);
}

void MQTTDriver::disconnect() {
    #ifdef DAQ_ETH
        global::interface->disconnect();
    #else
        global::_ESPclient.disconnect();
    #endif
}

bool MQTTDriver::connected() {
    return _client.connected();
}

void MQTTDriver::keepalive(uint64_t currTime) {
    if((currTime - _timeout) > 30000) {
        _client.ping(true);
        _timeout = arduino::millis();
    }
}

string MQTTDriver::networkInfo() {
    SocketAddress a;

    #ifdef DAQ_ETH
        global::interface->get_ip_address(&a);
    #else
        global::_ESPclient.get_ip_address(&a);
    #endif

    return a.get_ip_address() ? a.get_ip_address() : "None";
}