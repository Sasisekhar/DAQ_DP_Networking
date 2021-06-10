#ifndef MS5611_H
#define MS5611_H

#include "mbed.h"

//#define MS5611i2cLOWLEVEL   1               //if the use of low-level I2C routines is needed
//#warning "MS5611 using low level I2C routines"

#define SEA_PRESS   1013.25                 //default sea level pressure level in mb
#define KNOWNALT    327.0                   //default known altitude, 5200 Franklin Dr., 94588
#define INHG        0.02952998751           //convert mb to in/Hg constant
#define MB          33.8638815              //convert in/Hg to mb constant
#define FTMETERS    0.3048                  //convert feet to meters


/** Software routines to access the Measurement Specialties' MS5611-01BA03
 *  Variometer Module using the I2C bus option.  The MS5611 is a 24 bit
 *  temperature and pressure transducer for high accuracy Barometer and
 *  Altimeter applications.  It also includes compensation coefficients
 *  stored within the device.
 *
 *  Code adapted from Measurement Specialties:
 *  "AN520 C-code example for MS56xx, MS57xx (except analog sensor), and
 *  MS58xx series pressure sensors"
 *
 *  Note: AN520 has not been updated for use with the MS5611.  Changes
 *  were necessary to "calcPT()" in order to correct scaling of
 *  pressure readings.
 *
 *  Features:
 *          Altitude resolution to 10cm
 *          Fast conversion down to 1 ms
 *          Low power, 1 μA (standby < 0.15 μA)
 *          QFN package 5.0 x 3.0 x 1.0 mm^3
 *          Supply voltage 1.8 to 3.6 V
 *          Integrated digital pressure sensor (24 bit DeltaSigma ADC)
 *          Operating range: 10 to 1200 mbar, -40 to +85 °C
 *          I2C and SPI interface up to 20 MHz
 *          No external components (Internal oscillator)
 *          Excellent long term stability
 *
 * @code
 * #include "mbed.h"
 * #include "ms5611.h"
 *
 * //ms5611 ms(p9, p10);                        // i2c pins used
 * ms5611 ms(p9, p10, ms5611::CSBpin_0);      // NEW!! with rev 7. User can set polarity of CSB pin
 * //ms5611 ms(p9, p10, ms5611::CSBpin_1);
 *
 * Serial pc(USBTX, USBRX);                   // local terminal interface
 *
 *
 * int main (void) {
 *     pc.baud(921600);                        // set up USB serial speed
 *
 *     // set up the ms5611
 *     pc.printf("\n\nInitializing the MS5611..\n");
 *     ms.cmd_reset();
 *     pc.printf("Ready\n");
 *
 *     while(1) {
 *         double Temp = ms.calcTemp();                         //calculate press and temp, then returns current temperature in degC
 *         double Press = ms.calcPressure();                    //calculate press and temp, then returns current pressure in mb
 *         double GetPress = ms.getPressure();                  //returns current pressure in mb. Does no calculations.  Ususally done after calcTemp()
 *         double Altitude = ms.getAltitudeFT(1013.25);         //enter pressure at sea level in mb, returns altitude in feet
 *         double PressSeaLvlFT = ms.getSeaLevelBaroFT(327.2);  //enter known altitude in feet, returns sea level pressure in mb
 *         double PressSeaLvlM = ms.getAltitudeFT(99.73);       //enter known altitude in meters, returns seal level pressure in mb
 *
 *         pc.printf("Temp: %.2f degC\n", Temp);
 *         pc.printf("Barometer: %.1f mB  %.3f in/Hg\n", Press, Press * 0.0295301);
 *         pc.printf("Alt: %.1f ft\n", Altitude);
 *         pc.printf("Sea_Lvl: %.1f ft   %.2f m\n", PressSeaLvlFT, PressSeaLvlM);
 *         wait(2.0);
 *     }
 * }
 *
 * @endcode
 */

//_____ M A C R O S

#define MS5611_ADDR_W 0xEE // Module address write mode (CSBpin = 0);
#define MS5611_ADDR_R 0xEF // Module address read mode
#define MS5611_CMD_RESET 0x1E // ADC reset command
#define MS5611_CMD_ADC_READ 0x00 // ADC read command
#define MS5611_CMD_ADC_CONV 0x40 // ADC conversion command
#define MS5611_CMD_ADC_D1 0x00 // ADC D1 conversion
#define MS5611_CMD_ADC_D2 0x10 // ADC D2 conversion
#define MS5611_CMD_ADC_256 0x00 // ADC OSR=256
#define MS5611_CMD_ADC_512 0x02 // ADC OSR=512
#define MS5611_CMD_ADC_1024 0x04 // ADC OSR=1024
#define MS5611_CMD_ADC_2048 0x06 // ADC OSR=2048
#define MS5611_CMD_ADC_4096 0x08 // ADC OSR=4096
#define MS5611_CMD_PROM_RD 0xA0 // Prom read command

    /** Create ms5611 controller class
     *
     * @param ms5611 class
     *
     */
class ms5611 {

public:
    enum CSBpolarity {
        CSBpin_0,  //CSB pin is grounded, I2C address is 0xEE and 0xEF
        CSBpin_1,  //CSB pin is tied to Vdd, I2C address is 0xEC and 0xED
    };
    /** Create a MS5611 object using the specified I2C object
     *   - User fixed I2C address 0xEE, CSB pin = 0
     *   - This is the default legacy constructor
     * @param sda - mbed I2C interface pin
     * @param scl - mbed I2C interface pin
     */
    ms5611(PinName sda, PinName scl);
    /** Create a MS5611 object using the specified I2C object
     *   - User defined use of the CSB pin
     *   - CSB pin = 0, user set I2C address to 0xEE
     *   - CSB pin = 1, user set I2C address to 0xEC
     * @param sda - mbed I2C interface pin
     * @param scl - mbed I2C interface pin
     * @param ms5611::CSBpin_0 - CSB pin tied to ground
     * @param ms5611::CSBpin_1 - CSB pin tied to VDD
     */
    ms5611(PinName sda, PinName scl, CSBpolarity CSBpin);
    /** Initialize the MS5611 and set up the coefficients
     *    First - reset the MS5611
     *    Second - load coefficient values from the MS5611 PROM
     *    Third  - calculate coefficient checksum
     *  This routine only needs to be run once at boot up
     *
     * @param NONE
     */
    void cmd_reset();
    /** Calculate and return compensated temperature
     *    Returns double temperature in degC
     *
     * @param NONE
     */
    double calcTemp();
    /** Calculate and return compensated barometric pressure
     *    Returns double pressure in millibars
     *
     * @param NONE
     */
    double calcPressure();
    /** Return compensated barometric pressure
     *    Returns double pressure in millibars
     *    DOES NOT RE-CALCULATE FIRST!!!
     *    Saves time if you calcTemp(); first
     *
     * @param NONE
     */
    double getPressure();
    /** Calculate and returns altitude in feet
     *    Returns float altitude in feet
     *
     * @param float known pressure (mB) at sea level
     */
    float getAltitude();
    float getAltitudeFT(float sea_pressure);
    /** Calculate and returns sea level baro
     *    Returns float seal level barometer in feet
     *
     * @param float known altitude in feet
     */
    float getSeaLevelBaroFT(float known_alt);
    /** Calculate and returns sea level baro
     *    Returns float seal level barometer in meters
     *
     * @param float known altitude in meters
     */
    float getSeaLevelBaroM(float known_alt);

    uint8_t calcPT();


private:
#if not defined  MS5611i2cLOWLEVEL
    char cobuf[3];
#endif
    uint8_t _i2cWAddr;
    uint8_t _i2cRAddr;
    int32_t m_i2c_start(bool readMode);
    void m_i2c_stop();
    uint8_t m_i2c_write(uint8_t data);
    uint8_t m_i2c_readAck();
    uint8_t m_i2c_readNak();
    void m_i2c_send(uint8_t cmd);
    void loadCoefs();
    uint64_t cmd_adc(uint8_t cmd);
    uint32_t cmd_prom(uint8_t coef_num);
    uint8_t crc4(uint32_t  n_prom[]);
    uint32_t PTbuffer[8];       // calibration coefficients

    uint8_t step;
    int32_t D1, D2;

protected:
    I2C     _i2c;


};
#endif
