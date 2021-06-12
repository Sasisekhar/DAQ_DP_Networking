#include "CO2.h"

CO2::CO2(PinName const &mg_pin) : _ain(mg_pin){
    }
float CO2::MGRead()
{
    int i;
    float v=0;
    //v=_ain.read();
    //cout<<"V Value = "<<v;
    for (i=0;i<READ_SAMPLE_TIMES;i++) {
        v += _ain.read();
        ThisThread::sleep_for(READ_SAMPLE_INTERVAL);
    }
    //cout<<"Value of V is "<<(v/READ_SAMPLE_TIMES)<<endl;
    v = (v/READ_SAMPLE_TIMES)*5 ;
    cout<<"Value of V is "<<v<<endl;
    return v;
}

int  CO2::MGGetPercentage(float volts, float *pcurve)
{
   if ((volts/DC_GAIN )>=ZERO_POINT_VOLTAGE) {
      return -1;
   } else {
      return pow(10, ((volts/DC_GAIN)-pcurve[1])/pcurve[2]+pcurve[0]);
   }
}
