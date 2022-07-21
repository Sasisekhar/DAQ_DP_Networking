#ifndef BOOST_SIMULATION_DP_PACKETIZER_HPP
#define BOOST_SIMULATION_DP_PACKETIZER_HPP

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


struct DP_Packetizer_defs
{
  struct T : public in_port<double>{};
  struct H : public in_port<double>{};

  struct StJSONout : public out_port<string> {};
};

template<typename TIME>
class DP_Packetizer
{
  using defs=DP_Packetizer_defs;
  	public:
        DP_Packetizer() noexcept {
            for(int i = 0; i < 2; i++) {
                state.values[i] = 0;
            }
            state.active = false;
        }

        struct state_type {
            double values[2];
            string buffer;
            bool active;
        }; state_type state;

        using input_ports=std::tuple<typename defs::T, typename defs::H>;
      	using output_ports=std::tuple<typename defs::StJSONout>;


        void internal_transition (){
          state.active = false;
        }

        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
            for(const auto &x : get_messages<typename defs::T>(mbs)) {
            state.values[0] = x;
            } for(const auto &x : get_messages<typename defs::H>(mbs)) {
            state.values[1] = x;
            }

            // printf("ET_DEBUG(DPP): {");
            // for(int i = 0; i < 2; i++) {
            //   printf("%.2f, ", state.values[i]);
            // }
            // printf("}\n\r");

            char tempBuff[128];

            int h = 70;
          
            sprintf(tempBuff, "{\"Temp\":%.2f, \"Hum\":%.2f}", 
                                                            state.values[0], 
                                                            state.values[1]
            );

            printf("ET_DEBUG(DPP): %s\r\n", tempBuff);

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

            get_messages<typename defs::StJSONout>(bags).push_back(state.buffer);

            return bags;
        }

      TIME time_advance() const {
        if(state.active) {
          return TIME("00:00:00");
        }
        return std::numeric_limits<TIME>::infinity();
       // return TIME("01:00:00");

      }

      friend std::ostringstream& operator<<(std::ostringstream& os, const typename DP_Packetizer<TIME>::state_type& i) {
                 os << "Sent Data by DP_Packetizer: " << i.buffer ;
                 return os;
        }
    };
    #endif