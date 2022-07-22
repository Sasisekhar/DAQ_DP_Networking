#ifndef BOOST_SIMULATION_PUBLISHER_HPP
#define BOOST_SIMULATION_PUBLISHER_HPP

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

using namespace cadmium;
using namespace std;

#ifdef RT_ARM_MBED
//Begin RT_Cadmium

#include "MQTTDriver.h"
#include "mbed.h"

struct Publisher_defs {
    struct in : public in_port<string> {};
};

template<typename TIME>
class Publisher {

    using defs=Publisher_defs;

    public:

        Publisher(string topic, MQTTDriver *client) noexcept {

            state.topic = topic;
            state.message = "";
            state._client = client;

            state.UID = rand()%50;
            char buffer[2];
            sprintf(buffer, (state.UID < 10)? "0%d" : "%d", state.UID);

            state._client -> publish("ARSLAB/Register", buffer);
        }

        struct state_type {
            string topic;
            string message;
            int UID;
            MQTTDriver* _client;
        }; state_type state;

        using input_ports=std::tuple<typename defs::in>;
        using output_ports=std::tuple<>;

        void internal_transition() {}

        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            for(const auto &x : get_messages<typename defs::in>(mbs)) {
                state.message = x;
            }

            char topic[32];
            sprintf(topic, "%d/%s", state.UID, state.topic.c_str());
            // printf("ET_DEBUG: %s\r\n", state.message);
            state._client -> publish((const char*) topic, (char*) state.message.c_str());
        }

        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            internal_transition();
            external_transition(TIME(), std::move(mbs));
        }

        typename make_message_bags<output_ports>::type output() const {
            typename make_message_bags<output_ports>::type bags;
            return bags;
        }

        TIME time_advance() const {
        return std::numeric_limits<TIME>::infinity();
        }

        friend std::ostringstream& operator<<(std::ostringstream& os, const typename Publisher<TIME>::state_type& i) {
        os << "";
        return os;
        }
};



#else
#include <cadmium/io/oestream.hpp>

//Port definition
struct Publisher_defs{
    struct in : public in_port<string> {};
};


template<typename TIME>
class Publisher : public oestream_output<string,TIME, Publisher_defs> {
    public:
        Publisher() = default;
        Publisher(const char* file_path) : oestream_output<string,TIME, Publisher_defs>(file_path) {}
};
#endif
#endif