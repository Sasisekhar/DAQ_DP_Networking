#ifndef BOOST_SIMULATION_DATA_PARSER_HPP
#define BOOST_SIMULATION_DATA_PARSER_HPP

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


struct Data_Parser_defs {
  struct in : public in_port<string>{};


  struct out1 : public out_port<double> {};
  struct out2 : public out_port<double> {};
};

template<typename TIME>
class Data_Parser
{
  using defs=Data_Parser_defs;
  public:
    Data_Parser() noexcept {
      for(int i = 0; i < 2; i++) {
        state.values[i] = 0;
      }
      state.active = false;
    }

    struct state_type {
      double values[2];  //Number of inputs
      string buffer;
      bool active;
    }; state_type state;

    using input_ports=std::tuple<typename defs::in>;
    using output_ports=std::tuple<typename defs::out1, typename defs::out2>;


    void internal_transition (){
      state.active = false;
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
      for(const auto &x : get_messages<typename defs::in>(mbs)) {
        state.buffer = x;
      }

      int count = 0;
      string tmp = "";

      // printf("ET_DEBUG(DPRSR): %s\n\r", state.buffer.c_str());

      for(int i = 0; i < state.buffer.length(); i++) {

        if(state.buffer[i] != ',') {
          tmp += state.buffer[i];
        } else if(state.buffer[i] == ',') {
          // printf("LOOP: %d, %s, %d\r\n", i, tmp.c_str(), stoi(tmp));
          state.values[count++] = stof(tmp);
          tmp = "";
        }

      }

      printf("ET_DEBUG(DPRSR): {");
      for(int i = 0; i < 2; i++) {
        printf("%.2f, ", state.values[i]);
      }
      printf("}\n\r");

      state.active = true;
    }

    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      internal_transition();
      external_transition(TIME(), std::move(mbs));
    }

    typename make_message_bags<output_ports>::type output() const {
      typename make_message_bags<output_ports>::type bags;
      get_messages<typename defs::out1>(bags).push_back(state.values[0]);
      get_messages<typename defs::out2>(bags).push_back(state.values[1]);
      return bags;
    }

    TIME time_advance() const {
      if(state.active) {
        return TIME("00:00:00");
      }
      return std::numeric_limits<TIME>::infinity();
      // return TIME("01:00:00");

    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename Data_Parser<TIME>::state_type& i) {
      os << "Sent Data by Parser: " << i.buffer ;
      return os;
    }
};
#endif