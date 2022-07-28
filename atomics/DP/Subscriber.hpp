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

using namespace std;
using namespace cadmium;

#ifdef RT_ARM_MBED
//Begin RT_Cadmium

#include "MQTTDriver.h"
#include "mbed.h"

struct subscriber_defs {
    struct out1 : public out_port<string> {};
    struct out2 : public out_port<string> {};
};

template<typename TIME>
class Subscriber {

    using defs=subscriber_defs;
    public:

        Subscriber(string* topics, MQTTDriver* client, bool debug) noexcept {

            state._client = client;
            state.message   = "";
            state.topic     = "";
            state.debug = debug;

            for(int i = 0; i < 2; i++){
                state.topics[i] = topics[i];
                state._client -> subscribe(topics[i].c_str());
            }
            state._client -> subscribe("ARSLAB/DATA/HUM");

        }

        struct state_type {
            string topic;
            string topics[2];
            string message;
            bool valid;
            MQTTDriver* _client;
            bool debug;
        }; state_type state;

        using input_ports=std::tuple<>;
        using output_ports=std::tuple<typename defs::out1, typename defs::out2>;

        void internal_transition() {
            char tempTopic[17], tempMessage[128];
            state.valid = state._client -> receive_response(tempTopic, tempMessage);

            if(state.valid) {
                printf("Received\n\r");
            }

            state.topic = tempTopic;
            state.message = tempMessage;
            state._client -> keepalive((us_ticker_read()/1000));
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
                if(state.topic == state.topics[0]){
                    get_messages<typename defs::out1>(bags).push_back(state.message);
                } else if (state.topic == state.topics[1]) {
                    get_messages<typename defs::out2>(bags).push_back(state.message);
                }
            }

            return bags;
        }

        TIME time_advance() const {
            return TIME("00:00:00:050");
        }

        friend std::ostringstream& operator<<(std::ostringstream& os, const typename Subscriber<TIME>::state_type& i) {
            os << "Subscriber data" << i.message;
            return os;
        }
};

#else

#include <cadmium/io/iestream.hpp>

//Port definition
struct subscriber_defs{
    struct out : public out_port<string> {};
};


template<typename TIME>
class Subscriber : public iestream_input<string,TIME, subscriber_defs>{
    public:
        Subscriber() = default;
        Subscriber(const char* filename) :   iestream_input<string, TIME, subscriber_defs>(filename) {}
};

#endif
#endif