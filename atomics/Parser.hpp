#ifndef BOOST_SIMULATION_FUSION_HPP
#define BOOST_SIMULATION_FUSION_HPP

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

#include "../drivers/Algorithm.h"
#ifdef RT_ARM_MBED
#include "../drivers/DataToStorage.h"
#endif

using namespace cadmium;
using namespace std;


struct Parser_defs
{
  struct T : public in_port<string>{};
  struct H : public in_port<string>{};
  struct C : public in_port<string>{};


  struct T1 : public out_port<double> {};
  struct T2 : public out_port<double> {};
  struct T3 : public out_port<double> {};

  struct H1 : public out_port<double> {};
  struct H2 : public out_port<double> {};
  struct H3 : public out_port<double> {};

  struct C1 : public out_port<double> {};
  struct C2 : public out_port<double> {};
  struct C3 : public out_port<double> {};
};

template<typename TIME>
class Parser
{
  using defs=Parser_defs;
  	public:
        Parser() noexcept {
            for(int i = 0; i < 3; i++) {
                state.T[i] = 0;
            }
            state.active = false;
            // state.buffer = "";
        }

        struct state_type {
            double T[3];  //Number of inputs
            string buffer;
            bool active;
        }; state_type state;

        using input_ports=std::tuple<typename defs::T, typename defs::H, typename defs::C>;
      	using output_ports=std::tuple<typename defs::StJSONout>;


        void internal_transition (){
          state.active = false;
        }

        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
            for(const auto &x : get_messages<typename defs::T1>(mbs)) {
            state.T[0] = x;
            }
            for(const auto &x : get_messages<typename defs::T2>(mbs)) {
            state.T[1] = x;
            }
            for(const auto &x : get_messages<typename defs::T3>(mbs)) {
            state.T[2] = x;
            }

            char tempBuff[128];
          
            sprintf(tempBuff, "{\"Temp\":[%f, %f, %f]}", state.T[0], state.T[1], state.T[2]);

            string tempStr(tempBuff);

            state.buffer = tempStr;

            state.active = true;
      	}

        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
            internal_transition();
            external_transition(TIME(), std::move(mbs));
        }

        typename make_message_bags<output_ports>::type output() const {
            typename make_message_bags<output_ports>::type bags;

            string out;
            out = state.buffer;
            get_messages<typename defs::StJSONout>(bags).push_back(out);

            return bags;
        }

      TIME time_advance() const {
        if(state.active) {
          return TIME("00:00:01");
        }
        return std::numeric_limits<TIME>::infinity();
       // return TIME("01:00:00");

      }

      friend std::ostringstream& operator<<(std::ostringstream& os, const typename Packetizer<TIME>::state_type& i) {
                 os << "Sent Data by Fusion: " << i.buffer ;
                 return os;
        }
    };
    #endif