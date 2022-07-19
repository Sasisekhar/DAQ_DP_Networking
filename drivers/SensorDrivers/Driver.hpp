#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "DHT11.h"

namespace drivers {
  class TEMPERATURE_HUMIDITY {
  private:
    Dht11 dht11;
  public:
    TEMPERATURE_HUMIDITY(PinName TempPin) : dht11(TempPin) {

    }

    void TempHumidity(double &temp) {
        dht11.read();
        temp = dht11.getCelsius();
    }
  };
}
#endif
