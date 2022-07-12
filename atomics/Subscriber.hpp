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

#include <cadmium/io/iestream.hpp>
using namespace cadmium;
using namespace std;

//Port definition
struct Subscriber_defs{
    struct out : public out_port<string> {};
};


template<typename TIME>
class Subscriber : public iestream_input<string,TIME, Subscriber_defs>{
    public:
        Subscriber() = default;
        Subscriber(const char* filename) :   iestream_input<string, TIME, Subscriber_defs>(filename) {}
};