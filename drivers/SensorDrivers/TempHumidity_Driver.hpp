#ifndef TEMPHUMIDITY_DRIVER_HPP
#define TEMPHUMIDITY_DRIVER_HPP

#include "DHT11.h"

namespace drivers {
  class TEMPHUMIDITY_DRIVER {
  private:
    Dht11 dht11;
  public:
    TEMPHUMIDITY_DRIVER(PinName TempPin) : dht11(TempPin) {

    }

    void TempHumidity(double &temp) {
        dht11.read();
        temp = dht11.getCelsius();
    }
  };
}
#endif
