#ifndef DRIVER_HPP
#define DRIVER_HPP

#include "DHT11.h"

namespace drivers {
  class TEMPERATURE_HUMIDITY {
  private:
    Dht11 dht11;
  public:
    TEMPERATURE_HUMIDITY(PinName TempPin) : dht11(TempPin) { }

    int getTemp() {
      dht11.read();
      return dht11.getCelsius();
    }

    int getHum() {
      dht11.read();
      printf("%d\r\n", dht11.getHumidity());
      return dht11.getHumidity();
    }
  };
}
#endif
