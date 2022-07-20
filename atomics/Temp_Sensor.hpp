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
#include "../drivers/SensorDrivers/Driver.hpp"

struct temp_sensor_defs {
  struct out : public out_port<int> { };
};

template<typename TIME>
class tempSensor {

  using defs=temp_sensor_defs;
	public:
		TIME pollingRate;

    tempSensor() noexcept {
      MBED_ASSERT(false);
    }

		tempSensor(PinName pin) noexcept {
      new (this) tempSensor(pin, TIME("00:00:10:000"));
    }

    tempSensor(PinName pin, TIME rate) noexcept {
      pollingRate = rate;
      state.sensor = new drivers::TEMPERATURE_HUMIDITY(pin);
    }

    struct state_type {
      int out;
      drivers::TEMPERATURE_HUMIDITY *sensor;
    }; state_type state;

    using input_ports=std::tuple<>;
    using output_ports=std::tuple<typename defs::out>;

    void internal_transition() {
      state.out = state.sensor->getTemp();
      // printf("IT_Debug: %d\r\n", state.out);
    }

    void external_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      MBED_ASSERT(false);
    }

    void confluence_transition(TIME e, typename make_message_bags<input_ports>::type mbs) {
      internal_transition();
      external_transition(TIME(), std::move(mbs));
    }

    typename make_message_bags<output_ports>::type output() const {
      typename make_message_bags<output_ports>::type bags;
        get_messages<typename defs::out>(bags).push_back((state.out < 100 && state.out > -100)? state.out : 50);
        return bags;
      }

    TIME time_advance() const {
      return pollingRate;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename tempSensor<TIME>::state_type& i) {
      os << "Temperature Output " << i.out;
      return os;
    }
};

#else
#include <cadmium/io/iestream.hpp>
    using namespace cadmium;
    using namespace std;

    //Port definition
    struct temp_sensor_defs{
      struct out : public out_port<double> {};
    };


    template<typename TIME>
    class tempSensor : public iestream_input<double,TIME, temp_sensor_defs>{
      public:
        tempSensor() = default;
        tempSensor(const char* file_path) : iestream_input<double,TIME, temp_sensor_defs>(file_path) {}
        tempSensor(const char* file_path, TIME t) : iestream_input<float,double, temp_sensor_defs>(file_path) {}
    };

#endif
#endif