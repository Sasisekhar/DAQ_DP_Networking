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
#include "../atomics/DAQ_Packetizer.hpp"
#include "../atomics/Temp_Sensor.hpp"
#include "../atomics/Hum_Sensor.hpp"
#include "../atomics/Publisher.hpp"

#include <NDTime.hpp>


#ifdef RT_ARM_MBED
  #include "mbed.h"
#else

const char* t1 = "./inputs/Temperature_Sensor_Values1.txt";
const char* t2 = "./inputs/Temperature_Sensor_Values2.txt";
const char* t3 = "./inputs/Temperature_Sensor_Values3.txt";
const char* t4 = "./inputs/Temperature_Sensor_Values4.txt";
const char* D8;
const char* D9;

const char* daq_publish = "./outputs/DAQ_Publish.txt";

#endif
using namespace std;

using TIME = NDTime;

int main(int argc, char ** argv) {

  //This will end the main thread and create a new one with more stack.
  #ifdef RT_ARM_MBED

    MQTTDriver driver;

    //Logging is done over cout in RT_ARM_MBED
    struct oss_sink_provider{
      static std::ostream& sink(){
        return cout;
      }
    };
  
  #else
    // all simulation timing and I/O streams are ommited when running embedded

    auto start = chrono::high_resolution_clock::now(); //to measure simulation execution time

    /*************** Loggers *******************/

    static std::ofstream out_data("MessageOutputs.txt");
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

  #ifdef RT_ARM_MBED
  AtomicModelPtr Sensor1 = cadmium::dynamic::translate::make_dynamic_atomic_model<tempSensor, TIME>("Sensor1", D9);
  AtomicModelPtr Sensor2 = cadmium::dynamic::translate::make_dynamic_atomic_model<tempSensor, TIME>("Sensor2", D8);
  AtomicModelPtr Sensor3 = cadmium::dynamic::translate::make_dynamic_atomic_model<humSensor, TIME>("Sensor3", D9);
  AtomicModelPtr Sensor4 = cadmium::dynamic::translate::make_dynamic_atomic_model<humSensor, TIME>("Sensor4", D8);
  AtomicModelPtr Publisher1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Publisher, TIME>("Publisher1", "DATA/ALL");
  #else
  AtomicModelPtr Sensor1 = cadmium::dynamic::translate::make_dynamic_atomic_model<tempSensor, TIME>("Sensor1", t1);
  AtomicModelPtr Sensor2 = cadmium::dynamic::translate::make_dynamic_atomic_model<tempSensor, TIME>("Sensor2", t2);
  AtomicModelPtr Sensor3 = cadmium::dynamic::translate::make_dynamic_atomic_model<humSensor, TIME>("Sensor3", t3);
  AtomicModelPtr Sensor4 = cadmium::dynamic::translate::make_dynamic_atomic_model<humSensor, TIME>("Sensor4", t4);
  AtomicModelPtr Publisher1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Publisher, TIME>("Publisher1", daq_publish);
  #endif
  AtomicModelPtr DAQ_Packetizer1 = cadmium::dynamic::translate::make_dynamic_atomic_model<DAQ_Packetizer, TIME>("DAQ_Packetizer1");

  /***********************************************/
  /*************** DAQ Coupled *******************/
  /***********************************************/
  cadmium::dynamic::modeling::Ports iports_DAQ = {};
  cadmium::dynamic::modeling::Ports oports_DAQ = {};

  cadmium::dynamic::modeling::Models submodels_DAQ = {Sensor1, Sensor2, Sensor3, Sensor4, DAQ_Packetizer1, Publisher1};

  cadmium::dynamic::modeling::EICs eics_DAQ = {};
  cadmium::dynamic::modeling::EOCs eocs_DAQ = {};

  cadmium::dynamic::modeling::ICs ics_DAQ = {
    cadmium::dynamic::translate::make_IC<temp_sensor_defs::out, DAQ_Packetizer_defs::T1>("Sensor1","DAQ_Packetizer1"),
    cadmium::dynamic::translate::make_IC<temp_sensor_defs::out, DAQ_Packetizer_defs::T2>("Sensor2","DAQ_Packetizer1"),
    cadmium::dynamic::translate::make_IC<hum_sensor_defs::out, DAQ_Packetizer_defs::H1>("Sensor3","DAQ_Packetizer1"),
    cadmium::dynamic::translate::make_IC<hum_sensor_defs::out, DAQ_Packetizer_defs::H2>("Sensor4","DAQ_Packetizer1"),
    
    cadmium::dynamic::translate::make_IC<DAQ_Packetizer_defs::StJSONout, Publisher_defs::in>("DAQ_Packetizer1","Publisher1"),
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

  #ifdef RT_ARM_MBED
    cadmium::dynamic::engine::runner<NDTime, logger_top> r(DAQ, {0});
    // cadmium::dynamic::engine::runner<NDTime, cadmium::logger::not_logger> r(DAQ, {0});
  #else

  cadmium::dynamic::engine::runner<NDTime, logger_top> r(DAQ, {0});
  #endif

  r.run_until(NDTime("100:00:00:000"));
  #ifndef RT_ARM_MBED
  return 0;
  #endif

}
