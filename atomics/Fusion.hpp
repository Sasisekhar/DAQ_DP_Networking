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

using namespace cadmium;
using namespace std;


struct Fusion_defs
{
  struct s1T : public in_port<double>{};
  struct s2T : public in_port<double>{};
  struct s3T : public in_port<double>{};
  struct s4T : public in_port<double>{};
  struct s5T : public in_port<double>{};
  struct s6T : public in_port<double>{};
  struct s7T : public in_port<double>{};
  struct s8T : public in_port<double>{};

  struct outT : public out_port<double> {};
};

template<typename TIME>
class Fusion
{
  using defs=Fusion_defs;
  	public:
      Fusion() noexcept {
        for(int i=0;i<8;i++) {
          state.sT[i] = 0;
        }
        state.FusedT = 0;
        state.LastT = 0;
        state.number_of_sensors = 8;
        state.criterion = 0.95;
        state.active = false;
      }

      struct state_type {
        double sT [8];
        double FusedT;
        double LastT;
        double criterion;
        int number_of_sensors;
        bool active;
        }; state_type state;

        using input_ports=std::tuple<typename defs::s1T, typename defs::s2T, typename defs::s3T, typename defs::s4T, typename defs::s5T, typename defs::s6T, typename defs::s7T, typename defs::s8T>;
      	using output_ports=std::tuple<typename defs::outT>;


        void internal_transition (){
          state.LastT = state.FusedT;
          state.active = false;
         }

        void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs){
          for(const auto &x : get_messages<typename defs::s1T>(mbs)) {
            state.sT[0] = x;
          }
          for(const auto &x : get_messages<typename defs::s2T>(mbs)) {
            state.sT[1] = x;
          }
          for(const auto &x : get_messages<typename defs::s3T>(mbs)) {
            state.sT[2] = x;
          }
          for(const auto &x : get_messages<typename defs::s4T>(mbs)) {
            state.sT[3] = x;
          }
          for(const auto &x : get_messages<typename defs::s5T>(mbs)) {
            state.sT[4] = x;
          }
          for(const auto &x : get_messages<typename defs::s6T>(mbs)) {
            state.sT[5] = x;
          }
          for(const auto &x : get_messages<typename defs::s7T>(mbs)) {
            state.sT[6] = x;
          }
          for(const auto &x : get_messages<typename defs::s8T>(mbs)) {
            state.sT[7] = x;
          }
        
         //Here goes the wrapper
         state.FusedT = 
faulty_sensor_and_sensor_fusion(
compute_integrated_support_degree_score(state.sT,
            compute_alpha(eigen_value_calculation(sdm_calculator(state.sT, state.number_of_sensors))), 
            compute_phi(compute_alpha(eigen_value_calculation(sdm_calculator(state.sT, state.number_of_sensors))), state.number_of_sensors),
            sdm_calculator(state.sT, state.number_of_sensors),
            state.criterion, 
            state.number_of_sensors), state.sT, state.criterion, state.number_of_sensors );
  
          //If the values are not up to the mark, we can discard them here if that can be done.
      		state.active = true;
      	}

        void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
        internal_transition();
        external_transition(TIME(), std::move(mbs));
      }

      typename make_message_bags<output_ports>::type output() const {
        typename make_message_bags<output_ports>::type bags;
          get_messages<typename defs::outT>(bags).push_back(state.FusedT);

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
                 os << "Sent Data by Fusion: " << i.FusedT ;
                 return os;
               }
      };
      #endif