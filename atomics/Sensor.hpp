#ifndef BOOST_SIMULATION_SENSOR_HPP
#define BOOST_SIMULATION_SENSOR_HPP

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
#include <cadmium/real_time/arm_mbed/embedded_error.hpp>
#include "mbed.h"
#include "../drivers/SensorDrivers/TempHumidity_Driver.hpp"

struct Sensor_defs
{
        struct out : public out_port<double> { };
  };

template<typename TIME>
class Sensor
{
using defs=Sensor_defs;
	public:
		TIME pollingRate;
    Sensor() noexcept {
      MBED_ASSERT(false);
    }

		Sensor(PinName TempPin) noexcept {
      new (this) Sensor(TempPin, TIME("00:00:01:000"));
    		}

        Sensor(PinName TempPin, TIME rate) noexcept {
          pollingRate = rate;
          state.Temp = new drivers::TEMPHUMIDITY_DRIVER(TempPin);
        }

    		struct state_type {
    			double lastTemp;
    			double outputTemp;
          drivers::TEMPHUMIDITY_DRIVER* Temp;
    			}; state_type state;

    		using input_ports=std::tuple<>;
    		using output_ports=std::tuple<typename defs::out>;

    		void internal_transition() {
    			//state.lastTemp = state.outputTemp;
          //state.lastHum = state.outputHum;
          state.Temp->TempHumidity(state.outputTemp);
         // printf("Temp = %d \n", state.outputTemp);
         // printf("Hum = %d \n", state.outputHum);
    			}

    		void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      			MBED_ASSERT(false);
      //throw std::logic_error("External transition called in a model with no input ports");
    }

    		void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      			internal_transition();
      			external_transition(TIME(), std::move(mbs));
    }

    		typename make_message_bags<output_ports>::type output() const {
    			typename make_message_bags<output_ports>::type bags;
    			//if(state.lastTemp != state.outputTemp) {
    				get_messages<typename defs::out>(bags).push_back(state.outputTemp);
    				//}
            //if(state.lastHum != state.outputHum) {
                         //}
            return bags;
          }

    		TIME time_advance() const {
    			  return pollingRate;
   		 }

   		 friend std::ostringstream& operator<<(std::ostringstream& os, const typename Sensor<TIME>::state_type& i) {
      os << "Temperature Output " << i.outputTemp;
      return os;
    }
  };
  #else
#include <cadmium/io/iestream.hpp>
    using namespace cadmium;
    using namespace std;

    //Port definition
    struct Sensor_defs{
      struct out : public out_port<double> {};
    };


    template<typename TIME>
    class Sensor : public iestream_input<double,TIME, Sensor_defs>{
      public:
        Sensor() = default;
        Sensor(const char* file_path) : iestream_input<double,TIME, Sensor_defs>(file_path) {}
        Sensor(const char* file_path, TIME t) : iestream_input<float,double, Sensor_defs>(file_path) {}
    };

#endif
#endif