#include <iostream>
#include <chrono>
#include <algorithm>
#include <string>

#include <cadmium/modeling/coupling.hpp>
#include <cadmium/modeling/ports.hpp>
#include <cadmium/modeling/dynamic_model_translator.hpp>
#include <cadmium/concept/coupled_model_assert.hpp>
#include <cadmium/modeling/dynamic_coupled.hpp>
#include <cadmium/modeling/dynamic_atomic.hpp>
#include <cadmium/engine/pdevs_dynamic_runner.hpp>
#include <cadmium/logger/tuple_to_ostream.hpp>
#include <cadmium/logger/common_loggers.hpp>
#include <cadmium/io/iestream.hpp>
#ifdef RT_ARM_MBED
#include <cadmium/real_time/arm_mbed/rt_clock.hpp>
#endif
#include "../atomics/Fusion.hpp"
#include "../atomics/Sensor.hpp"

#include <NDTime.hpp>
#ifdef RT_ARM_MBED
  #include "../mbed.h"
#else
const char* D3 = "./inputs/Temperature_Sensor_Values1.txt";
const char* D5 = "./inputs/Temperature_Sensor_Values2.txt";
const char* D7 = "./inputs/Temperature_Sensor_Values3.txt";
const char* D8 = "./inputs/Temperature_Sensor_Values4.txt";
const char* D10 = "./inputs/Temperature_Sensor_Values5.txt";
const char* t6_IN = "./inputs/Temperature_Sensor_Values6.txt";
const char* t7_IN = "./inputs/Temperature_Sensor_Values7.txt";
const char* t8_IN = "./inputs/Temperature_Sensor_Values8.txt";
const char* t9_IN = "./inputs/Temperature_Sensor_Values9.txt";
const char* t10_IN = "./inputs/Temperature_Sensor_Values10.txt";
#endif
using namespace std;

using hclock=chrono::high_resolution_clock;
using TIME = NDTime;

int main(int argc, char ** argv) {

  //This will end the main thread and create a new one with more stack.
  #ifdef RT_ARM_MBED
    //Set RTC
    set_time(1256729737);  // Set RTC time to Wed, 28 Oct 2009 11:35:37
    //Logging is done over cout in RT_ARM_MBED
    struct oss_sink_provider{
      static std::ostream& sink(){
        return cout;
      }
    };
  #else
    // all simulation timing and I/O streams are ommited when running embedded

    auto start = hclock::now(); //to measure simulation execution time

    /*************** Loggers *******************/

    static std::ofstream out_data("SensorFusion_Cadmium_output.txt");
    struct oss_sink_provider{
      static std::ostream& sink(){
        return out_data;
      }
    };
  #endif


  using info=cadmium::logger::logger<cadmium::logger::logger_info, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using debug=cadmium::logger::logger<cadmium::logger::logger_debug, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using state=cadmium::logger::logger<cadmium::logger::logger_state, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using log_messages=cadmium::logger::logger<cadmium::logger::logger_messages, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using routing=cadmium::logger::logger<cadmium::logger::logger_message_routing, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using global_time=cadmium::logger::logger<cadmium::logger::logger_global_time, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using local_time=cadmium::logger::logger<cadmium::logger::logger_local_time, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using log_all=cadmium::logger::multilogger<info, debug, state, log_messages, routing, global_time, local_time>;
  using logger_top=cadmium::logger::multilogger<log_messages, global_time>;

  using AtomicModelPtr=std::shared_ptr<cadmium::dynamic::modeling::model>;
  using CoupledModelPtr=std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>>;

  AtomicModelPtr Sensor1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor1", D3);
  AtomicModelPtr Sensor2 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor2", D5);
  AtomicModelPtr Sensor3 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor3", D7);
  AtomicModelPtr Sensor4 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor4", D8);
  AtomicModelPtr Sensor5 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor5", D10);
  AtomicModelPtr Sensor6 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor6", t6_IN);
  AtomicModelPtr Sensor7 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor7", t7_IN);
  AtomicModelPtr Sensor8 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor8", t8_IN);
  AtomicModelPtr Sensor9 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor9", t9_IN);
  
  AtomicModelPtr Fusion1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Fusion, TIME>("Fusion1");
  
  cadmium::dynamic::modeling::Ports iports_TOP = {};
  cadmium::dynamic::modeling::Ports oports_TOP = {};

  cadmium::dynamic::modeling::Models submodels_TOP = {Sensor1, Sensor2, Sensor3, Sensor4, Sensor5, Sensor6, Sensor7, Sensor8, Fusion1};

cadmium::dynamic::modeling::EICs eics_TOP = {};
cadmium::dynamic::modeling::EOCs eocs_TOP = {};

cadmium::dynamic::modeling::ICs ics_TOP = {
cadmium::dynamic::translate::make_IC<Sensor_defs::out, Fusion_defs::s1T>("Sensor1","Fusion1"),

  cadmium::dynamic::translate::make_IC<Sensor_defs::out, Fusion_defs::s2T>("Sensor2","Fusion1"),

  cadmium::dynamic::translate::make_IC<Sensor_defs::out, Fusion_defs::s3T>("Sensor3","Fusion1"),

  cadmium::dynamic::translate::make_IC<Sensor_defs::out, Fusion_defs::s4T>("Sensor4","Fusion1"),

  cadmium::dynamic::translate::make_IC<Sensor_defs::out, Fusion_defs::s5T>("Sensor5","Fusion1"),

  cadmium::dynamic::translate::make_IC<Sensor_defs::out, Fusion_defs::s6T>("Sensor6","Fusion1"),

  cadmium::dynamic::translate::make_IC<Sensor_defs::out, Fusion_defs::s7T>("Sensor7","Fusion1"),

  cadmium::dynamic::translate::make_IC<Sensor_defs::out, Fusion_defs::s8T>("Sensor8","Fusion1")


};
CoupledModelPtr TOP = std::make_shared<cadmium::dynamic::modeling::coupled<TIME>>(
    "TOP",
    submodels_TOP,
    iports_TOP,
    oports_TOP,
    eics_TOP,
    eocs_TOP,
    ics_TOP
    );
#ifdef RT_ARM_MBED
   // cadmium::dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});

      cadmium::dynamic::engine::runner<NDTime, cadmium::logger::not_logger> r(TOP, {0});
#else

 // cadmium::dynamic::engine::runner<NDTime, log_all> r(TOP, {0});
    cadmium::dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});
#endif

r.run_until(NDTime("100:00:00:000"));
#ifndef RT_ARM_MBED
return 0;
#endif

}
