#ifndef BOOST_SIMULATION_PUBLISHER_HPP
#define BOOST_SIMULATION_PUBLISHER_HPP

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


#include <cadmium/io/oestream.hpp>
using namespace cadmium;
using namespace std;

//Port definition
struct Publisher_defs{
    struct in : public in_port<string> {};
};


template<typename TIME>
class Publisher : public oestream_output<string,TIME, Publisher_defs> {
    public:
        Publisher() = default;
        Publisher(const char* file_path) : oestream_output<string,TIME, Publisher_defs>(file_path) {}
};

#endif