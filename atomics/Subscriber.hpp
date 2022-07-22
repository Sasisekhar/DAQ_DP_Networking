#ifndef SUBSCRIBER_HPP
#define SUBSCRIBER_HPP

#include <stdio.h>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/message_bag.hpp>
#include <limits>
#include <math.h>
#include <assert.h>
#include <memory>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <chrono>
#include <algorithm>
#include <limits>
#include <random>

#ifdef RT_ARM_MBED
//Begin RT_Cadmium

#include "MQTTDriver.h"
#include "mbed.h"

MQTTDriver client;

struct subscriber_defs {
    struct out : public out_port<string> {};
};

template<typename TIME>
class Subscriber {

    using defs=subscriber_defs;
    public:

        Subscriber(string topic) noexcept {

            state.message   = "";
            state.topic     = topic;

            client.init();
            printf("Connecting to the broker...\n\r");

            char clientID[16];

            sprintf(clientID, "DP_SUBSCRIBER_%d", rand()%100);

            printf("Subscriber clientID: %s\r\n", clientID);

            if(client.connect((const char*) clientID)) {
                printf("Connected!\n\r");
            }

            client.subscribe((const char*) state.topic.c_str());

        }

        struct state_type {
            string topic;
            string message;
            bool valid;
        }; state_type state;

        using input_ports=std::tuple<>;
        using output_ports=std::tuple<typename defs::out>;

        void internal_transition() {
            char tempTopic[16], tempMessage[128];
            state.valid = client.receive_response(tempTopic, tempMessage);
            // printf("IT_DEBUG: %s\r\n", tempMessage);
            string tempM(tempMessage);
            state.message = tempMessage;
            client.keepalive((us_ticker_read()/1000));
        }

        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { }

        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            internal_transition();
            external_transition(TIME(), std::move(mbs));
        }

        typename make_message_bags<output_ports>::type output() const {
            typename make_message_bags<output_ports>::type bags;

            if(state.valid) {
                // printf("Lambda_debug: %s\r\n",state.message.c_str());
                get_messages<typename defs::out>(bags).push_back(state.message);
            }

            return bags;
        }

        TIME time_advance() const {
            return TIME("00:00:00:100");
        }

        friend std::ostringstream& operator<<(std::ostringstream& os, const typename Subscriber<TIME>::state_type& i) {
            os << "";
            return os;
        }
};

#else

#include <cadmium/io/iestream.hpp>
using namespace cadmium;
using namespace std;

//Port definition
struct Subscriber_defs{
    struct out : public out_port<string> {};
};


template<typename TIME>
class Subscriber : public iestream_input<string,TIME, Subscriber_defs>{
    public:
        Subscriber() = default;
        Subscriber(const char* filename) :   iestream_input<string, TIME, Subscriber_defs>(filename) {}
};

#endif
#endif