#include <iostream>
#include <vector>
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

/***************** Includes for DP **********************/
#ifdef DP_COUPLED

  #include "../atomics/DP/Subscriber.hpp"
  #include "../atomics/DP/Data_Parser.hpp"
  #include "../atomics/DP/Fusion.hpp"
  #include "../atomics/DP/DP_Packetizer.hpp"
  #include "../atomics/DP/Publisher.hpp"
  #include <NDTime.hpp>

  #ifdef RT_ARM_MBED
    #include "../drivers/MQTT/MQTTDriver.h"
    #include "../mbed-os/mbed.h"

  #else
    const char* t = "./outputs/Network_Publish1.txt";
    const char* h = "./outputs/Network_Publish2.txt";
    const char* dp_publish = "./outputs/DP_Publish.txt";
  #endif

#endif
/*********************************************************/

/***************** Includes for DAQ **********************/
#ifdef DAQ_COUPLED
  #include "../atomics/DAQ/DAQ_Packetizer.hpp"
  #include "../atomics/DAQ/DHT_Sensor.hpp"
  #include "../atomics/DP/Publisher.hpp"
  #include "../atomics/DAQ/Sensor.hpp"
  #include <NDTime.hpp>

  #ifdef RT_ARM_MBED
    #include "../drivers/MQTT/MQTTDriver.h"
    #include "../mbed-os/mbed.h"
  #else
    const char* t1 = "./inputs/Temperature_Sensor_Values1.txt";
    const char* t2 = "./inputs/Temperature_Sensor_Values2.txt";
    const char* h1 = "./inputs/Temperature_Sensor_Values3.txt";
    const char* h2 = "./inputs/Temperature_Sensor_Values4.txt";
    const char* daq_publish = "./outputs/DAQ_Publish.txt";
  #endif

#endif
/*********************************************************/

/*************** Includes for Network ********************/
#ifdef NETWORK_COUPLED
  #include "../atomics/DP/Subscriber.hpp"
  #include "../atomics/DP/Publisher.hpp"
  #include "../atomics/Network/DataFlowController.hpp"
  #include <NDTime.hpp>

  const char* network_subscribe = "./outputs/DAQ_Publish.txt";
  const char* network_publish1 = "./outputs/Network_Publish1.txt";
  const char* network_publish2 = "./outputs/Network_Publish2.txt";

#endif
/*********************************************************/


using namespace std;

#ifndef RT_ARM_MBED
  using hclock=chrono::high_resolution_clock;
#endif

using TIME = NDTime;

int main(int argc, char ** argv) {

  /**************** Initializers required for both DAQ and DP MBED implementations ********************/
  #ifdef RT_ARM_MBED

    //Instantiate the MQTT driver and create a client
    MQTTDriver driver;
    driver.init();  //initialise driver

    #ifdef DP_COUPLED //List of topics for he DP to subscribe to
      string topics[] = {"ARSLAB/DATA/TEMP", "ARSLAB/DATA/HUM"}; //topics for fusion subscription
      // string ip = driver.networkInfo();
      // printf("%s\r\n", driver.networkInfo().c_str());
      printf("Connecting to the broker...\n\r");
      if(driver.connect("Fusion_DP")) { //Establish connection to the broker
        printf("Connected!\n\r");
      }
    #elif DAQ_COUPLED
       string ip = driver.networkInfo();
      printf("%s\r\n", ip.c_str());
      printf("Connecting to the broker...\n\r");
      if(driver.connect((char*)ip.c_str())) { //Establish connection to the broker
        printf("Connected!\n\r");
      }
      string publishTopic = driver.networkInfo() + "/DATA/ALL";
    #endif

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

    static std::ofstream out_data("logs.txt");
    struct oss_sink_provider{
      static std::ostream& sink(){
        return out_data;
      }
    };

  #endif


  /***************************************************/
  /******************* Loggers ***********************/
  /***************************************************/

  using info=cadmium::logger::logger<cadmium::logger::logger_info, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using debug=cadmium::logger::logger<cadmium::logger::logger_debug, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using state=cadmium::logger::logger<cadmium::logger::logger_state, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using log_messages=cadmium::logger::logger<cadmium::logger::logger_messages, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using routing=cadmium::logger::logger<cadmium::logger::logger_message_routing, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using global_time=cadmium::logger::logger<cadmium::logger::logger_global_time, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using local_time=cadmium::logger::logger<cadmium::logger::logger_local_time, cadmium::dynamic::logger::formatter<TIME>, oss_sink_provider>;
  using log_all=cadmium::logger::multilogger<info, debug, state, log_messages, routing, global_time, local_time>;
  using logger_top=cadmium::logger::multilogger<log_messages, global_time>;

  /***************************************************/
  /***************************************************/
  /***************************************************/

  using AtomicModelPtr=std::shared_ptr<cadmium::dynamic::modeling::model>;
  using CoupledModelPtr=std::shared_ptr<cadmium::dynamic::modeling::coupled<TIME>>;

  #ifdef DP_COUPLED
    /***************************************************/
    /************** DP Atomic models *******************/
    /***************************************************/
    
    #ifdef RT_ARM_MBED
      AtomicModelPtr Subscriber1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Subscriber, TIME>("Subscriber1", topics, &driver, false);
      AtomicModelPtr Publisher2 = cadmium::dynamic::translate::make_dynamic_atomic_model<Publisher, TIME>("Publisher2", "DATA/FUSED", &driver);
    #else
      AtomicModelPtr Subscriber1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Subscriber, TIME>("Subscriber1", t);
      AtomicModelPtr Subscriber2 = cadmium::dynamic::translate::make_dynamic_atomic_model<Subscriber, TIME>("Subscriber2", h);
      AtomicModelPtr Publisher2 = cadmium::dynamic::translate::make_dynamic_atomic_model<Publisher, TIME>("Publisher2", dp_publish);
    #endif

    AtomicModelPtr Data_Parser1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Data_Parser, TIME>("Data_Parser1");
    AtomicModelPtr Data_Parser2 = cadmium::dynamic::translate::make_dynamic_atomic_model<Data_Parser, TIME>("Data_Parser2");
    AtomicModelPtr Fusion1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Fusion, TIME>("Fusion1");
    AtomicModelPtr Fusion2 = cadmium::dynamic::translate::make_dynamic_atomic_model<Fusion, TIME>("Fusion2");
    AtomicModelPtr DP_Packetizer1 = cadmium::dynamic::translate::make_dynamic_atomic_model<DP_Packetizer, TIME>("DP_Packetizer1");

    /*************************************************/
    /*************************************************/
    /*************************************************/

    /*************************************************/
    /************ DP Coupled model *******************/
    /*************************************************/

    cadmium::dynamic::modeling::Ports iports_DP = {};
    cadmium::dynamic::modeling::Ports oports_DP = {};

    #ifdef RT_ARM_MBED
    cadmium::dynamic::modeling::Models submodels_DP = {Subscriber1, Data_Parser1, Data_Parser2, Fusion1, Fusion2, DP_Packetizer1, Publisher2};
    #else
    cadmium::dynamic::modeling::Models submodels_DP = {Subscriber1, Subscriber2, Data_Parser1, Data_Parser2, Fusion1, Fusion2, DP_Packetizer1, Publisher2};
    #endif

    cadmium::dynamic::modeling::EICs eics_DP = {};
    cadmium::dynamic::modeling::EOCs eocs_DP = {};

    cadmium::dynamic::modeling::ICs ics_DP = {

      #ifdef RT_ARM_MBED
      cadmium::dynamic::translate::make_IC<subscriber_defs::out1, Data_Parser_defs::in>("Subscriber1","Data_Parser1"),
      cadmium::dynamic::translate::make_IC<subscriber_defs::out2, Data_Parser_defs::in>("Subscriber1","Data_Parser2"),
      #else
      cadmium::dynamic::translate::make_IC<subscriber_defs::out, Data_Parser_defs::in>("Subscriber1","Data_Parser1"),
      cadmium::dynamic::translate::make_IC<subscriber_defs::out, Data_Parser_defs::in>("Subscriber2","Data_Parser2"),
      #endif

      cadmium::dynamic::translate::make_IC<Data_Parser_defs::out1, Fusion_defs::in1>("Data_Parser1","Fusion1"),
      cadmium::dynamic::translate::make_IC<Data_Parser_defs::out2, Fusion_defs::in2>("Data_Parser1","Fusion1"),

      cadmium::dynamic::translate::make_IC<Data_Parser_defs::out1, Fusion_defs::in1>("Data_Parser2","Fusion2"),
      cadmium::dynamic::translate::make_IC<Data_Parser_defs::out2, Fusion_defs::in2>("Data_Parser2","Fusion2"),

      cadmium::dynamic::translate::make_IC<Fusion_defs::out, DP_Packetizer_defs::T>("Fusion1","DP_Packetizer1"),
      cadmium::dynamic::translate::make_IC<Fusion_defs::out, DP_Packetizer_defs::H>("Fusion2","DP_Packetizer1"),

      cadmium::dynamic::translate::make_IC<DP_Packetizer_defs::StJSONout, Publisher_defs::in> ("DP_Packetizer1", "Publisher2")
    };

    CoupledModelPtr DP = std::make_shared<cadmium::dynamic::modeling::coupled<TIME>>(
      "DP",
      submodels_DP,
      iports_DP,
      oports_DP,
      eics_DP,
      eocs_DP,
      ics_DP
    );
    
  #endif
  /***************************************************/
  /***************************************************/
  /***************************************************/

  #ifdef DAQ_COUPLED
    /*************************************************/
    /*********** DAQ Atomic models *******************/
    /*************************************************/

    #ifdef RT_ARM_MBED
    #ifdef DAQ_ETH
      AtomicModelPtr Sensor1 = cadmium::dynamic::translate::make_dynamic_atomic_model<dhtSensor, TIME>("Sensor1", D3);
      AtomicModelPtr Sensor2 = cadmium::dynamic::translate::make_dynamic_atomic_model<dhtSensor, TIME>("Sensor2", D5);
    #else
        AtomicModelPtr Sensor1 = cadmium::dynamic::translate::make_dynamic_atomic_model<dhtSensor, TIME>("Sensor1", D9);
        AtomicModelPtr Sensor2 = cadmium::dynamic::translate::make_dynamic_atomic_model<dhtSensor, TIME>("Sensor2", D8);
    #endif
      AtomicModelPtr Publisher1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Publisher, TIME>("Publisher1", publishTopic, &driver);
    #else
      AtomicModelPtr Sensor1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor1", t1);
      AtomicModelPtr Sensor2 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor2", t2);
      AtomicModelPtr Sensor3 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor3", h1);
      AtomicModelPtr Sensor4 = cadmium::dynamic::translate::make_dynamic_atomic_model<Sensor, TIME>("Sensor4", h2);
      AtomicModelPtr Publisher1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Publisher, TIME>("Publisher1", daq_publish);
    #endif

    AtomicModelPtr DAQ_Packetizer1 = cadmium::dynamic::translate::make_dynamic_atomic_model<DAQ_Packetizer, TIME>("DAQ_Packetizer1");

    /*************************************************/
    /*************************************************/
    /*************************************************/

    /*************************************************/
    /**************** DAQ Coupled ********************/
    /*************************************************/
    cadmium::dynamic::modeling::Ports iports_DAQ = {};
    cadmium::dynamic::modeling::Ports oports_DAQ = {};

    #ifdef RT_ARM_MBED
      cadmium::dynamic::modeling::Models submodels_DAQ = {Sensor1, Sensor2, DAQ_Packetizer1, Publisher1};
    #else
      cadmium::dynamic::modeling::Models submodels_DAQ = {Sensor1, Sensor2, Sensor3, Sensor4, DAQ_Packetizer1, Publisher1};
    #endif

    cadmium::dynamic::modeling::EICs eics_DAQ = {};
    cadmium::dynamic::modeling::EOCs eocs_DAQ = {};

    cadmium::dynamic::modeling::ICs ics_DAQ = {

      #ifdef RT_ARM_MBED
        cadmium::dynamic::translate::make_IC<dht_sensor_defs::T, DAQ_Packetizer_defs::T1>("Sensor1","DAQ_Packetizer1"),
        cadmium::dynamic::translate::make_IC<dht_sensor_defs::T, DAQ_Packetizer_defs::T2>("Sensor2","DAQ_Packetizer1"),
        cadmium::dynamic::translate::make_IC<dht_sensor_defs::H, DAQ_Packetizer_defs::H1>("Sensor1","DAQ_Packetizer1"),
        cadmium::dynamic::translate::make_IC<dht_sensor_defs::H, DAQ_Packetizer_defs::H2>("Sensor2","DAQ_Packetizer1"),
      #else
        cadmium::dynamic::translate::make_IC<Sensor_defs::out, DAQ_Packetizer_defs::T1>("Sensor1","DAQ_Packetizer1"),
        cadmium::dynamic::translate::make_IC<Sensor_defs::out, DAQ_Packetizer_defs::T2>("Sensor2","DAQ_Packetizer1"),
        cadmium::dynamic::translate::make_IC<Sensor_defs::out, DAQ_Packetizer_defs::H1>("Sensor3","DAQ_Packetizer1"),
        cadmium::dynamic::translate::make_IC<Sensor_defs::out, DAQ_Packetizer_defs::H2>("Sensor4","DAQ_Packetizer1"),
      #endif
      
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

    /*************************************************/
    /*************************************************/
    /*************************************************/
  #endif
  /*************************************************/
  /*************************************************/
  /*************************************************/

  #ifdef NETWORK_COUPLED
    /*************************************************/
    /********* Network Atomic models *****************/
    /*************************************************/

    AtomicModelPtr Publisher1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Publisher, TIME>("Publisher1", network_publish1);
    AtomicModelPtr Publisher2 = cadmium::dynamic::translate::make_dynamic_atomic_model<Publisher, TIME>("Publisher2", network_publish2);
    AtomicModelPtr Subscriber1 = cadmium::dynamic::translate::make_dynamic_atomic_model<Subscriber, TIME>("Subscriber1", network_subscribe);
    AtomicModelPtr DFC1 = cadmium::dynamic::translate::make_dynamic_atomic_model<DFC, TIME>("DFC1");

    /*************************************************/
    /*************************************************/
    /*************************************************/

    /*************************************************/
    /************** Network Coupled ******************/
    /*************************************************/
    cadmium::dynamic::modeling::Ports iports_NETWORK = {};
    cadmium::dynamic::modeling::Ports oports_NETWORK = {};

    cadmium::dynamic::modeling::Models submodels_NETWORK = {Publisher1, Publisher2, Subscriber1, DFC1};

    cadmium::dynamic::modeling::EICs eics_NETWORK = {};
    cadmium::dynamic::modeling::EOCs eocs_NETWORK = {};

    cadmium::dynamic::modeling::ICs ics_NETWORK = {

      cadmium::dynamic::translate::make_IC<subscriber_defs::out, DFC_defs::in>("Subscriber1","DFC1"),
      
      cadmium::dynamic::translate::make_IC<DFC_defs::outT, Publisher_defs::in>("DFC1","Publisher1"),
      cadmium::dynamic::translate::make_IC<DFC_defs::outH, Publisher_defs::in>("DFC1","Publisher2")
    };

    CoupledModelPtr NETWORK = std::make_shared<cadmium::dynamic::modeling::coupled<TIME>>(
      "NETWORK",
      submodels_NETWORK,
      iports_NETWORK,
      oports_NETWORK,
      eics_NETWORK,
      eocs_NETWORK,
      ics_NETWORK
    );

    /*************************************************/
    /*************************************************/
    /*************************************************/
  #endif


  /*************************************************/
  /******************* Runner **********************/
  /*************************************************/

  #ifdef DP_COUPLED

    #ifdef RT_ARM_MBED
      // cadmium::dynamic::engine::runner<NDTime, logger_top> r(DP, {0});
      cadmium::dynamic::engine::runner<NDTime, cadmium::logger::not_logger> r(DP, {0});
    #else

    cadmium::dynamic::engine::runner<NDTime, logger_top> r(DP, {0});

    #endif
  #endif

  #ifdef DAQ_COUPLED

    #ifdef RT_ARM_MBED
      cadmium::dynamic::engine::runner<NDTime, logger_top> r(DAQ, {0});
      // cadmium::dynamic::engine::runner<NDTime, cadmium::logger::not_logger> r(DAQ, {0});
      // cadmium::dynamic::engine::runner<NDTime, cadmium::logger::not_logger> r(test, {0});
    #else

    cadmium::dynamic::engine::runner<NDTime, logger_top> r(DAQ, {0});
    #endif
  #endif

  #ifdef NETWORK_COUPLED
  cadmium::dynamic::engine::runner<NDTime, logger_top> r(NETWORK, {0});
  #endif

  /*************************************************/
  /*************************************************/
  /*************************************************/

  r.run_until(NDTime("100:00:00:000"));
  #ifndef RT_ARM_MBED
  return 0;
  #endif

}