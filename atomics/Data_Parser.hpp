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
class Data_Parser
{
  using defs=Data_Parser_defs;
  public:
    Data_Parser() noexcept {
      for(int i = 0; i < 9; i++) {
        state.values[i] = 0;
      }
      state.active = false;
    }

    struct state_type {
      double values[9];  //Number of inputs
      string buffer[3];
      bool active;
    }; state_type state;

    using input_ports=std::tuple<typename defs::T, typename defs::H, typename defs::C>;
    using output_ports=std::tuple<typename defs::T1, typename defs::T2, typename defs::T3, typename defs::H1, typename defs::H2, typename defs::H3, typename defs::C1, typename defs::C2, typename defs::C3>;


    void internal_transition (){
      state.active = false;
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
      for(const auto &x : get_messages<typename defs::T>(mbs)) {
        state.buffer[0] = x;
      }
      for(const auto &x : get_messages<typename defs::H>(mbs)) {
        state.buffer[1] = x;
      }
      for(const auto &x : get_messages<typename defs::C>(mbs)) {
        state.buffer[2] = x;
      }

      int count = 0;
      string tmp = "";

      //temp csv
      for(int i = 0; i < state.buffer[0].length(); i++) {

        if((state.buffer[0])[i] != ',') {
          tmp += (state.buffer[0])[i];
        } else if((state.buffer[0])[i] == ',') {
          state.values[count++] = stoi(tmp);
          tmp = "";
        }

      }

      //hum csv
      for(int i = 0; i < state.buffer[1].length(); i++) {

        if((state.buffer[1])[i] != ',') {
          tmp += (state.buffer[1])[i];
        } else if((state.buffer[1])[i] == ',') {
          state.values[count++] = stoi(tmp);
          tmp = "";
        }

      }

      //co csv
      for(int i = 0; i < state.buffer[2].length(); i++) {

        if((state.buffer[2])[i] != ',') {
          tmp += (state.buffer[2])[i];
        } else if((state.buffer[2])[i] == ',') {
          state.values[count++] = stoi(tmp);
          tmp = "";
        }

      }

      state.active = true;
    }

    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      internal_transition();
      external_transition(TIME(), std::move(mbs));
    }

    typename make_message_bags<output_ports>::type output() const {
      typename make_message_bags<output_ports>::type bags;
      get_messages<typename defs::T1>(bags).push_back(state.values[0]);
      get_messages<typename defs::T2>(bags).push_back(state.values[1]);
      get_messages<typename defs::T3>(bags).push_back(state.values[2]);

      get_messages<typename defs::H1>(bags).push_back(state.values[3]);
      get_messages<typename defs::H2>(bags).push_back(state.values[4]);
      get_messages<typename defs::H3>(bags).push_back(state.values[5]);

      get_messages<typename defs::C1>(bags).push_back(state.values[6]);
      get_messages<typename defs::C2>(bags).push_back(state.values[7]);
      get_messages<typename defs::C3>(bags).push_back(state.values[8]);
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