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
#include "../atomics/Packetizer.hpp"
#include "../atomics/Sensor.hpp"
#include "../atomics/Subscriber.hpp"
#include "../atomics/DP_Parser.hpp"
#include "../atomics/Fusion.hpp"

#include <NDTime.hpp>
#ifdef RT_ARM_MBED
  #include "../mbed.h"
#else
const char* t1 = "./inputs/Temperature_Sensor_Values1.txt";
const char* t2 = "./inputs/Temperature_Sensor_Values2.txt";
const char* t3 = "./inputs/Temperature_Sensor_Values3.txt";

const char* h1 = "./inputs/Temperature_Sensor_Values4.txt";
const char* h2 = "./inputs/Temperature_Sensor_Values5.txt";
const char* h3 = "./inputs/Temperature_Sensor_Values6.txt";

const char* c1 = "./inputs/Temperature_Sensor_Values7.txt";
const char* c2 = "./inputs/Temperature_Sensor_Values8.txt";
const char* c3 = "./inputs/Temperature_Sensor_Values9.txt";

const char* t = "./inputs/fused_t.txt";
const char* h = "./inputs/fused_h.txt";
const char* c = "./inputs/fused_c.txt";

#endif
using namespace std;

using hclock=chrono::high_resolution_clock;
using TIME = NDTime;

int main(int argc, char ** argv) {

  //This will end the main thread and create a new one with more stack.
  #ifdef RT_ARM_MBED
    //Set RTC
    set_time(1610538009);  // Set RTC time to Wed, 28 Oct 2009 11:35:37
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

  /*************************************************/
  /*************** Atomic models *******************/
  /*************************************************/
  AtomicModelPtr Sensor1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor1", t1);
  AtomicModelPtr Sensor2 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor2", t2);
  AtomicModelPtr Sensor3 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor3", t3);

  AtomicModelPtr Sensor4 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor4", h1);
  AtomicModelPtr Sensor5 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor5", h2);
  AtomicModelPtr Sensor6 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor6", h3);

  AtomicModelPtr Sensor7 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor7", c1);
  AtomicModelPtr Sensor8 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor8", c2);
  AtomicModelPtr Sensor9 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor9", c3);

  AtomicModelPtr Packetizer1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Packetizer, TIME>("Packetizer1");

  // AtomicModelPtr Subscriber1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Subscriber, TIME>("Subscriber1", t, h, c);
  AtomicModelPtr DP_Parser1 = cadmium::dynamic::translate::make_dynamic_atomic_model<DP_Parser, TIME>("DP_Parser1");
  AtomicModelPtr Fusion1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Fusion, TIME>("Fusion1");

  /***********************************************/
  /*************** DAQ Coupled *******************/
  /***********************************************/
  cadmium::dynamic::modeling::Ports iports_DAQ = {};
  cadmium::dynamic::modeling::Ports oports_DAQ = {};

  cadmium::dynamic::modeling::Models submodels_DAQ = {Sensor1, Sensor2, Sensor3, Sensor4, Sensor5, Sensor6, Sensor7, Sensor8, Sensor9, Packetizer1};

  cadmium::dynamic::modeling::EICs eics_DAQ = {};
  cadmium::dynamic::modeling::EOCs eocs_DAQ = {};

  cadmium::dynamic::modeling::ICs ics_DAQ = {
    cadmium::dynamic::translate::make_IC<Sensor_defs::out, Packetizer_defs::T1>("Sensor1","Packetizer1"),
    cadmium::dynamic::translate::make_IC<Sensor_defs::out, Packetizer_defs::T2>("Sensor2","Packetizer1"),
    cadmium::dynamic::translate::make_IC<Sensor_defs::out, Packetizer_defs::T3>("Sensor3","Packetizer1"),

    cadmium::dynamic::translate::make_IC<Sensor_defs::out, Packetizer_defs::H1>("Sensor4","Packetizer1"),
    cadmium::dynamic::translate::make_IC<Sensor_defs::out, Packetizer_defs::H2>("Sensor5","Packetizer1"),
    cadmium::dynamic::translate::make_IC<Sensor_defs::out, Packetizer_defs::H3>("Sensor6","Packetizer1"),

    cadmium::dynamic::translate::make_IC<Sensor_defs::out, Packetizer_defs::C1>("Sensor7","Packetizer1"),
    cadmium::dynamic::translate::make_IC<Sensor_defs::out, Packetizer_defs::C2>("Sensor8","Packetizer1"),
    cadmium::dynamic::translate::make_IC<Sensor_defs::out, Packetizer_defs::C3>("Sensor9","Packetizer1"),
  };

  CoupledModelPtr DAQ = std::make_shared<cadmium::dynamic::modeling::coupled<TIME>>(
    "DAQ",
    submodels_DAQ,
    iports_DAQ,
    oports_DAQ,
    eics_DAQ,
    eocs_DAQ,
    ics_DAQ
  );

  /**********************************************/
  /*************** DP Coupled *******************/
  /**********************************************/

  /***************************************************/
  /*************** Network Coupled *******************/
  /***************************************************/

  /***********************************************/
  /*************** TOP Coupled *******************/
  /***********************************************/

  #ifdef RT_ARM_MBED
    // cadmium::dynamic::engine::runner<NDTime, logger_top> r(TOP, {0});

        cadmium::dynamic::engine::runner<NDTime, cadmium::logger::not_logger> r(TOP, {0});
  #else

  // cadmium::dynamic::engine::runner<NDTime, log_all> r(DAQ, {0});
      cadmium::dynamic::engine::runner<NDTime, logger_top> r(DAQ, {0});
  #endif

  r.run_until(NDTime("100:00:00:000"));
  #ifndef RT_ARM_MBED
  return 0;
  #endif

}
