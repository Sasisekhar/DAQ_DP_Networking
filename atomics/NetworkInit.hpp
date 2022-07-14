#ifndef BOOST_SIMULATION_NETWORKINIT_HPP
#define BOOST_SIMULATION_NETWORKINIT_HPP

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



#else

//Port definition
struct NetworkInit_defs{};

template<typename TIME>
class NetworkInit {
    using defs = NetworkInit_defs;
    public:
        NetworkInit() = default;

        struct state_type {
        }; state_type state;

        using input_ports=std::tuple<>;
        using output_ports=std::tuple<>;


        void internal_transition () {
        }

        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) { }

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

        friend std::ostringstream& operator<<(std::ostringstream& os, const typename NetworkInit<TIME>::state_type& i) {
          os << "Sent Data by Init: ";
          return os;
        }
};
#endif
#endif