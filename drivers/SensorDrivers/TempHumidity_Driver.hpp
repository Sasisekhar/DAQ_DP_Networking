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

    double TempHumidity() {
      dht11.read();
      return dht11.getCelsius();
    }
  };
}
#endif
