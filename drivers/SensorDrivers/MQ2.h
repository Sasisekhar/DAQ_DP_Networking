#ifndef MQ2_h
#define MQ2_h

#include "mbed.h"

#define RL_VALUE                    1                                           //define the load resistance on the board, in kilo ohms
#define RO_DEFAULT                  10                                          //Ro is initialized to 10 kilo ohms
#define RO_CLEAN_AIR_FACTOR         9.83f                                       //RO_CLEAR_AIR_FACTOR=(Sensor resistance in clean air)/RO, which is derived from the chart in datasheet
#define CALIBARAION_SAMPLE_TIMES    5                                           //define how many samples you are going to take in the calibration phase
#define CALIBRATION_SAMPLE_INTERVAL 50                                          //define the time interal(in milisecond) between each samples
#define READ_SAMPLE_INTERVAL        50                                          //define how many samples you are going to take in normal operation
#define READ_SAMPLE_TIMES           5                                           //define the time interal(in milisecond) between each samples

//The curves
static float LPGCurve[]   = {2.3f,0.21f,-0.47f};
static float COCurve[]    = {2.3f,0.72f,-0.34f};
static float SmokeCurve[] = {2.3f,0.53f,-0.44f};

typedef struct {
  float lpg;
  float co;
  float smoke;
} MQ2_data_t;

typedef enum  {
    GAS_LPG = 0,
    GAS_CO = 1,
    GAS_SMOKE = 2
} GasType;


class MQ2 {
public:
    MQ2(PinName pin) : _pin(pin){
      Ro = RO_DEFAULT;
    };
    void read(MQ2_data_t *ptr);
    float readLPG();
    float readCO();
    float readSmoke();
    void begin();
private:
    AnalogIn _pin;
    float MQRead();
    float MQGetGasPercentage(float rs_ro_ratio, GasType gas_id);
    int MQGetPercentage(float rs_ro_ratio, float *pcurve);
    float MQCalibration();
    float MQResistanceCalculation(int raw_adc);
    float Ro;
};

#endif
