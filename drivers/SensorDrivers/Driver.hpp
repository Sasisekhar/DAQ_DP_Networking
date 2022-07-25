#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "DHT11.h"

namespace drivers {
  class TEMPERATURE_HUMIDITY {
  private:
    Dht11 dht11;
  public:
    TEMPERATURE_HUMIDITY(PinName TempPin) : dht11(TempPin) { }

    void getVal(double &val1, double &val2) {

      dht11.read();
      val1 = dht11.getCelsius();
      val2 = dht11.getHumidity();

    }
  };
}
#endif
