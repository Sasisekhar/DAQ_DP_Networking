//!
//! @file an520_I2C.c,v
//!
//! Copyright (c) 2009 MEAS Switzerland
//!
//!
//!
//! @brief This C code is for starter reference only. It is written for the
//! MEAS Switzerland MS56xx pressure sensor modules and Atmel Atmega644p
//! microcontroller.
//!
//! @version 1.0 $Id: an520_I2C.c,v 1.0
//!
//! @todo

#include "mbed.h"
#include "MS5611.h"

double P;                       // compensated pressure value (mB)
double T;                       // compensated temperature value (degC)
double A;                       // altitude (ft)
double S;                       // sea level barometer (mB)

uint32_t C[8];                  //coefficient storage

//--------------------------------------------------------------------------------------------------------------------------------------//
// Constructor and destructor - default to be compatible with legacy m5611 driver

ms5611::ms5611(PinName sda, PinName scl)  : _i2c(sda, scl) {
        _i2c.frequency(400000);
        _i2cWAddr = MS5611_ADDR_W;
        _i2cRAddr = MS5611_ADDR_R;
        step = 1;
}

//--------------------------------------------------------------------------------------------------------------------------------------//
// Constructor and destructor - new, to allow for user to select i2c address based on CSB pin

ms5611::ms5611(PinName sda, PinName scl, CSBpolarity CSBpin)  : _i2c(sda, scl) {
        _i2c.frequency(400000);
        _i2cWAddr = MS5611_ADDR_W;
        _i2cRAddr = MS5611_ADDR_R;
        if(CSBpin == CSBpin_1) {
            _i2cWAddr -= 2;
            _i2cRAddr -= 2;

        }
        step = 1;
}

//********************************************************
//! @brief send I2C start condition and the address byte
//!
//! @return 0
//********************************************************

int32_t ms5611::m_i2c_start(bool readMode) {
    int32_t twst;
    _i2c.start();
    if(readMode == true) {
        twst = m_i2c_write(_i2cRAddr);
    } else {
        twst = m_i2c_write(_i2cWAddr);
    }
    return(twst);
}

//********************************************************
//! @brief send I2C stop condition
//!
//! @return none
//********************************************************

void ms5611::m_i2c_stop() {
    _i2c.stop();
}

//********************************************************
//! @brief send I2C stop condition
//!
//! @return remote ack status
//********************************************************

uint8_t ms5611::m_i2c_write(uint8_t data) {
    uint8_t twst = _i2c.write(data);
    return(twst);
}

//********************************************************
//! @brief read I2C byte with acknowledgment
//!
//! @return read byte
//********************************************************

uint8_t ms5611::m_i2c_readAck() {
    uint8_t twst = _i2c.read(1);
    return(twst);
}

//********************************************************
//! @brief read I2C byte without acknowledgment
//!
//! @return read byte
//********************************************************

uint8_t ms5611::m_i2c_readNak() {
    uint8_t twst = _i2c.read(0);
    return(twst);
}

//********************************************************
//! @brief send command using I2C hardware interface
//!
//! @return none
//********************************************************

void ms5611::m_i2c_send(uint8_t cmd) {
    uint8_t ret;
    ret = m_i2c_start(false);
    if(!(ret)) {
        m_i2c_stop();
    } else {
        ret = m_i2c_write(cmd);
        m_i2c_stop();
    }
}

//********************************************************
//! @brief send reset sequence
//!
//! @return none
//********************************************************

void ms5611::cmd_reset() {
#if defined  MS5611i2cLOWLEVEL
    m_i2c_send(MS5611_CMD_RESET);
#else
    cobuf[0] = MS5611_CMD_RESET;
    _i2c.write(_i2cWAddr, cobuf, 1, false);
#endif
    wait_us(4*1000);
    loadCoefs();
}

//********************************************************
//! @brief preform adc conversion
//!
//! @return 24bit result
//********************************************************

uint64_t ms5611::cmd_adc(uint8_t cmd) {
    uint64_t temp = 0;
    if( step & 0b101 ){ // or 0b1 either 0b100
#if defined  MS5611i2cLOWLEVEL
        m_i2c_send(MS5611_CMD_ADC_CONV + cmd);
#else
        cobuf[0] = 0;
        cobuf[1] = 0;
        cobuf[2] = 0;
        cobuf[0] = MS5611_CMD_ADC_CONV + cmd;
        _i2c.write(_i2cWAddr, cobuf, 1, false);
#endif
        /*
        switch (cmd & 0x0f) {
            case MS5611_CMD_ADC_256 : wait_us(900); break;
            case MS5611_CMD_ADC_512 : wait_ms(3); break;
            case MS5611_CMD_ADC_1024: wait_ms(4); break;
            case MS5611_CMD_ADC_2048: wait_ms(6); break;
            case MS5611_CMD_ADC_4096: wait_ms(10); break;
        }
        */
        step = step<<1; // will be 0b10 or 0b1000
        return 0;
    }
    else if( step & 0b1010 ){ // or 0b10 either 0b1000
#if defined  MS5611i2cLOWLEVEL
        m_i2c_send(MS5611_CMD_ADC_READ);
        m_i2c_start(true);
        temp = m_i2c_readAck();
        temp = (temp << 8) | m_i2c_readAck();
        temp = (temp << 8) | m_i2c_readNak();
        m_i2c_stop();
#else
        cobuf[0] = MS5611_CMD_ADC_READ;
        _i2c.write(_i2cWAddr, cobuf, 1, true);
        cobuf[0] = 0;
        _i2c.read(_i2cRAddr, cobuf, 3, false);
        temp = (cobuf[0] << 16) | (cobuf[1] << 8) | cobuf[2];
#endif
        step = step<<1; // will be 0b100 or 0b10000
        return temp;
    }
    return 0;
}

//********************************************************
//! @brief Read calibration coefficients
//!
//! @return coefficient
//********************************************************

uint32_t ms5611::cmd_prom(uint8_t coef_num) {
    uint32_t rC = 0;
#if defined  MS5611i2cLOWLEVEL
    m_i2c_send(MS5611_CMD_PROM_RD + coef_num * 2); // send PROM READ command
    m_i2c_start(true);
    rC = m_i2c_readAck();
    rC = (rC << 8) | m_i2c_readNak();
    m_i2c_stop();
#else
    cobuf[0] = 0;
    cobuf[1] = 0;
    cobuf[0] = MS5611_CMD_PROM_RD + coef_num * 2;
    _i2c.write(_i2cWAddr, cobuf, 1, true);
    cobuf[0] = 0;
    _i2c.read(_i2cRAddr, cobuf, 2, false);
    rC = (cobuf[0] << 8) | cobuf[1];
#endif
    return rC;
}

//********************************************************
//! @brief calculate the CRC code
//!
//! @return crc code
//********************************************************

uint8_t ms5611::crc4(uint32_t n_prom[]) {
    uint32_t n_rem;
    uint32_t crc_read;
    uint8_t n_bit;
    n_rem = 0x00;
    crc_read = n_prom[7];
    n_prom[7]=(0xFF00 & (n_prom[7]));
    for (int cnt = 0; cnt < 16; cnt++) {
            if (cnt%2 == 1) {
                n_rem ^= (uint16_t) ((n_prom[cnt>>1]) & 0x00FF);
            } else {
                n_rem ^= (uint16_t) (n_prom[cnt>>1]>>8);
            }
            for (n_bit = 8; n_bit > 0; n_bit--) {
                if (n_rem & (0x8000)) {
                    n_rem = (n_rem << 1) ^ 0x3000;
                } else {
                    n_rem = (n_rem << 1);
                }
            }
        }
    n_rem= (0x000F & (n_rem >> 12));
    n_prom[7]=crc_read;
    return (n_rem ^ 0x0);
}

/*
The CRC code is calculated and written in factory with the LSB byte in the prom n_prom[7] set to 0x00 (see
Coefficient table below). It is thus important to clear those bytes from the calculation buffer before proceeding
with the CRC calculation itself:
n_prom[7]=(0xFF00 & (n_prom[7])); //CRC byte is replaced by 0
As a simple test of the CRC code, the following coefficient table could be used:
uint32_t nprom[] = {0x3132,0x3334,0x3536,0x3738,0x3940,0x4142,0x4344,0x4500};
the resulting calculated CRC should be 0xB.

DB  15  14  13  12  11  10  9   8   7   6   5   4   3   2   1   0
Addr
0               16 bit reserved for manufacturer
1               Coefficient 1 (16 bit unsigned)
2               Coefficient 2 (16 bit unsigned)
3               Coefficient 3 (16 bit unsigned)
4               Coefficient 4 (16 bit unsigned)
5               Coefficient 5 (16 bit unsigned)
6               Coefficient 6 (16 bit unsigned)
7                                   0   0   0   0     CRC(0x0)
*/
/*
    //Returns 0x0b as per AP520_004
    C[0] = 0x3132;
    C[1] = 0x3334;
    C[2] = 0x3536;
    C[3] = 0x3738;
    C[4] = 0x3940;
    C[5] = 0x4142;
    C[6] = 0x4344;
    C[7] = 0x4546;
    n_crc = ms.crc4(C); // calculate the CRC
    pc.printf("testing CRC: 0x%x\n", n_crc);
*/

//********************************************************
//! @brief load all calibration coefficients
//!
//! @return none
//********************************************************

void ms5611::loadCoefs() {
    for (int i = 0; i < 8; i++){
        wait_us(50*1000);
        C[i] = cmd_prom(i);
    }
    uint8_t n_crc = crc4(C);
}

//********************************************************
//! @brief calculate temperature and pressure
//!
//! @return none
//********************************************************

uint8_t ms5611::calcPT() {
    if( step & 0b11 ){ // or 0b1 either 0b10
        D2 = cmd_adc(MS5611_CMD_ADC_D2 + MS5611_CMD_ADC_2048); // read D2
    }
    if( step & 0b1100 ){ // or 0b10 either 0b100
        D1 = cmd_adc(MS5611_CMD_ADC_D1 + MS5611_CMD_ADC_2048); // read D1
    }
    if( step & 0b10000 ){
        int64_t dT = D2 - ((uint64_t)C[5] << 8);
        int64_t OFF  = ((uint32_t)C[2] << 16) + ((dT * (C[4]) >> 7));     //was  OFF  = (C[2] << 17) + dT * C[4] / (1 << 6);
        int64_t SENS = ((uint32_t)C[1] << 15) + ((dT * (C[3]) >> 8));     //was  SENS = (C[1] << 16) + dT * C[3] / (1 << 7);
        int32_t TEMP = 2000 + (int64_t)dT * (int64_t)C[6] / (int64_t)(1 << 23);
        T = (double) TEMP / 100.0;

        if(TEMP < 2000) { // if temperature lower than +20 Celsius
            int64_t T1 = ((int64_t)TEMP - 2000) * ((int64_t)TEMP - 2000);
            int64_t OFF1  = (5 * T1) >> 1;
            int64_t SENS1 = (5 * T1) >> 2;

            if(TEMP < -1500) { // if temperature lower than -15 Celsius
                T1 = ((int64_t)TEMP + 1500) * ((int64_t)TEMP + 1500);
                OFF1  +=  7 * T1;
                SENS1 += 11 * T1 >> 1;
            }
            OFF -= OFF1;
            SENS -= SENS1;
        }
        P = (double)(((((int64_t)D1 * SENS ) >> 21) - OFF) / (double) (1 << 15)) / 100.0;
        step = 1;
        return 1;
    }
    return 0;
}

//********************************************************
//! @brief calculate temperature
//!
//! @return double temperature degC
//********************************************************

double ms5611::calcTemp() {
    calcPT();
    return(T);
}

//********************************************************
//! @brief calculate pressure
//!
//! @return double barometric pressure millibar
//********************************************************

double ms5611::calcPressure() {
    calcPT();
    return(P);
}

//********************************************************
//! @brief get pressure, no calculation
//!
//! @return double barometric pressure millibar
//********************************************************

double ms5611::getPressure() {
    //calcPT();
    return(P);
}

float ms5611::getAltitude() {
    A = (1 - pow(P/(double)1013.250, 0.190295)) * 44330.0;
    //A = (1 - (pow((P / (double)sea_pressure), 0.190284))) * 145366.45;
    return((float)A);
}

//********************************************************
//! @brief get altitude from known sea level barometer,
//! @      no pre-pressure calculation
//!
//! @enter float sea level barometer
//! @return float altitude in feet
//********************************************************

float ms5611::getAltitudeFT(float sea_pressure) {
    A = (1 - (pow((P / (double)sea_pressure), 0.190284))) * 145366.45;
    return((float)A);
}

//********************************************************
//! @brief get sea level pressure from known altitude(ft),
//! @      no pre-pressure calculation
//!
//! @enter float known altitude in feet
//! @return float seal level barometer in mb
//********************************************************

float ms5611::getSeaLevelBaroFT(float known_alt) {
    S = pow(pow((P * INHG), 0.190284) + 0.00001313 * known_alt , 5.2553026) * MB;
    return((float)S);
}

//********************************************************
//! @brief get sea level pressure from known altitude(m),
//! @      no pre-pressure calculation
//!
//! @enter float known altitude in meters
//! @return float seal level barometer in mb
//********************************************************

float ms5611::getSeaLevelBaroM(float known_alt) {
    S = pow(pow((P * INHG), 0.190284) + 0.00001313 * known_alt * FTMETERS , 5.2553026) * MB;
    return((float)S);
}
