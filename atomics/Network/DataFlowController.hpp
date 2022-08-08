#ifndef DATA_FLOW_CONTROLLER_HPP
#define DATA_FLOW_CONTROLLER_HPP

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

#include "nlohmann/json.hpp"

using namespace cadmium;
using namespace std;
using json = nlohmann::json;


struct DFC_defs {
  struct in : public in_port<string>{};

  struct outT : public out_port<string> {};
  struct outH : public out_port<string> {};
};

template<typename TIME>
class DFC {
  using defs=DFC_defs;
  public:
    DFC() noexcept {
        state.bufferT   = "";
        state.bufferH   = "";
        state.buffer    = "";
        state.active    = false;
    }

    struct state_type {
      string bufferT;
      string bufferH;
      string buffer;
      bool active;
    }; state_type state;

    using input_ports=std::tuple<typename defs::in>;
    using output_ports=std::tuple<typename defs::outT, typename defs::outH>;


    void internal_transition (){
      state.active = false;
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
      for(const auto &x : get_messages<typename defs::in>(mbs)) {
        state.buffer = x;
      }

      json tempObj = json::parse(state.buffer.c_str());

      state.bufferT = "";
      state.bufferH = "";
      
      for(int i = 0; i < 2; i++) {
        state.bufferT += to_string(tempObj["Temp"][i]);
        state.bufferT += ",";
      }

      for(int i = 0; i < 2; i++) {
        state.bufferH += to_string(tempObj["Hum"][i]);
        state.bufferH += ",";
      }

      state.active = true;

    }

    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      internal_transition();
      external_transition(TIME(), std::move(mbs));
    }

    typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;

        get_messages<typename defs::outT>(bags).push_back(state.bufferT);
        get_messages<typename defs::outH>(bags).push_back(state.bufferH);

        return bags;
    }

    TIME time_advance() const {
      if(state.active) {
        return TIME("00:00:00");
      }
      return std::numeric_limits<TIME>::infinity();
      // return TIME("01:00:00");

    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename DFC<TIME>::state_type& i) {
      os << "Sent Data by DFC: " << i.buffer ;
      return os;
    }
};
#endif