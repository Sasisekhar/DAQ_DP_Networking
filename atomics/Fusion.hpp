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
#define MAX_THREAD_INFO 10
#endif

using namespace cadmium;
using namespace std;


struct Fusion_defs
{
  struct in1 : public in_port<double>{};
  struct in2 : public in_port<double>{};

  struct out : public out_port<double> {};
};

template<typename TIME>
class Fusion
{
  using defs=Fusion_defs;
  public:
    Fusion() noexcept {
      for(int i=0;i<2;i++) { //Number of inputs hardcoded here. Change if modification required
        state.values[i] = 0;
      }
      state.Fused  = 0;
      state.Last   = 0;
      state.number_of_sensors = 2;
      state.criterion = 0.95;
      state.active = false;
    }

    struct state_type {
      double values[2];  //Number of inputs
      double Fused;
      double Last;
      double criterion;
      int number_of_sensors;
      bool active;
    }; state_type state;

    using input_ports=std::tuple<typename defs::in1, typename defs::in2>;
    using output_ports=std::tuple<typename defs::out>;


    void internal_transition (){
      state.Last = state.Fused;
      state.active = false;
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
      for(const auto &x : get_messages<typename defs::in1>(mbs)) {
        state.values[0] = x;
      }
      for(const auto &x : get_messages<typename defs::in2>(mbs)) {
        state.values[1] = x;
      }
      
      // printf("ET_DEBUG(FSN): {");
      // for(int i = 0; i < 2; i++) {
      //   printf("%.2f, ", state.values[i]);
      // }
      // printf("}\n\r");
  
    
      // Here goes the wrapper
      state.Fused = 
      faulty_sensor_and_sensor_fusion(
        compute_integrated_support_degree_score(state.values,
        compute_alpha(eigen_value_calculation(sdm_calculator(state.values, state.number_of_sensors))), 
        compute_phi(compute_alpha(eigen_value_calculation(sdm_calculator(state.values, state.number_of_sensors))), state.number_of_sensors),
        sdm_calculator(state.values, state.number_of_sensors),
        state.criterion, 
        state.number_of_sensors), state.values, state.criterion, state.number_of_sensors
      );

      // printf("ET_DEBUG(FSN): %.2f\r\n", state.Fused);

      // #ifdef RT_ARM_MBED
      // StoreData(state.values, state.number_of_sensors, state.Fused);
      // #endif

      // If the values are not up to the mark, we can discard them here if that can be done.
      state.active = true;
    }

    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      internal_transition();
      external_transition(TIME(), std::move(mbs));
    }

    typename make_message_bags<output_ports>::type output() const {
      typename make_message_bags<output_ports>::type bags;
        get_messages<typename defs::out>(bags).push_back(state.Fused);  
      return bags;
    }

    TIME time_advance() const {
      if(state.active) {
        return TIME("00:00:00");
      }
      return std::numeric_limits<TIME>::infinity();
      // return TIME("01:00:00");

    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename Fusion<TIME>::state_type& i) {
      os << "Sent Data by Fusion: " << i.Fused;
      return os;
    }
};
#endif