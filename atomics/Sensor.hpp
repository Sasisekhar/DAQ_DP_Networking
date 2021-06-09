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