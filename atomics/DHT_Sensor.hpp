#ifndef DHT_SENSOR_HPP
#define DHT_SENSOR_HPP

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

struct dht_sensor_defs {
  struct T : public out_port<double> { };
  struct H : public out_port<double> { };
};

template<typename TIME>
class dhtSensor {

  using defs=dht_sensor_defs;
	public:
		TIME pollingRate;

    dhtSensor() noexcept {
      MBED_ASSERT(false);
    }

		dhtSensor(PinName pin) noexcept {
      new (this) dhtSensor(pin, TIME("00:00:10:000"));
    }

    dhtSensor(PinName pin, TIME rate) noexcept {

      for(int i = 0; i < 2; i++) {
        state.out[i] = 0;
      }

      pollingRate = rate;
      state.sensor = new drivers::TEMPERATURE_HUMIDITY(pin);
    }

    struct state_type {
      double out[2];
      drivers::TEMPERATURE_HUMIDITY *sensor;
    }; state_type state;

    using input_ports=std::tuple<>;
    using output_ports=std::tuple<typename defs::T, typename defs::H>;

    void internal_transition() {
      state.sensor->getVal(state.out[0], state.out[1]);
      #ifndef RT_SASI_DEBUG
      printf("The values from the sensor are: %f, %f \n\r", state.out[0], state.out[1]);
      #endif
      // ThisThread::sleep_for(500ms);
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
        get_messages<typename defs::T>(bags).push_back((state.out[0] < 100 && state.out[0] > -100)? state.out[0] : 50);
        get_messages<typename defs::H>(bags).push_back((state.out[1] < 100 && state.out[1] > -100)? state.out[1] : 50);
        return bags;
      }

    TIME time_advance() const {
      return pollingRate;
    }

    friend std::ostringstream& operator<<(std::ostringstream& os, const typename dhtSensor<TIME>::state_type& i) {
      os << "Temperature Output " << i.out;
      return os;
    }
};

#else
#include <cadmium/io/iestream.hpp>
    using namespace cadmium;
    using namespace std;

    //Port definition
    struct dht_sensor_defs{
      struct out : public out_port<double> {};
    };


    template<typename TIME>
    class dhtSensor : public iestream_input<double,TIME, dht_sensor_defs>{
      public:
        dhtSensor() = default;
        dhtSensor(const char* file_path) : iestream_input<double,TIME, dht_sensor_defs>(file_path) {}
        dhtSensor(const char* file_path, TIME t) : iestream_input<float,double, dht_sensor_defs>(file_path) {}
    };

#endif
#endif