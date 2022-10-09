#pragma once
//
//    FILE: CHT8305.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.2
// PURPOSE: Arduino library for CHT8305 temperature and humidity sensor
//     URL: https://github.com/RobTillaart/CH8305
//


#include "Arduino.h"
#include "Wire.h"


#define CHT8305_LIB_VERSION              (F("0.1.2"))

//  ERRORS
#define CHT8305_OK                       0
#define CHT8305_ERROR_ADDR               -10
#define CHT8305_ERROR_I2C                -11
#define CHT8305_ERROR_CONNECT            -12
#define CHT8305_ERROR_LASTREAD           -20

//  REGISTERS
#define CHT8305_REG_TEMPERATURE          0x00
#define CHT8305_REG_HUMIDITY             0x01
#define CHT8305_REG_CONFIG               0x02
#define CHT8305_REG_ALERT                0x03
#define CHT8305_REG_VOLTAGE              0x04
#define CHT8305_REG_MANUFACTURER         0xFE
#define CHT8305_REG_VERSION              0xFF

//  REGISTER MASKS
#define CHT8305_CFG_SOFT_RESET          0x8000
#define CHT8305_CFG_CLOCK_STRETCH       0x4000
#define CHT8305_CFG_HEATER              0x2000
#define CHT8305_CFG_MODE                0x1000
#define CHT8305_CFG_VCCS                0x0800
#define CHT8305_CFG_TEMP_RES            0x0400
#define CHT8305_CFG_HUMI_RES            0x0300
#define CHT8305_CFG_ALERT_MODE          0x00C0
#define CHT8305_CFG_ALERT_PENDING       0x0020
#define CHT8305_CFG_ALERT_HUMI          0x0010
#define CHT8305_CFG_ALERT_TEMP          0x0008
#define CHT8305_CFG_VCC_ENABLE          0x0004
#define CHT8305_CFG_VCC_RESERVED        0x0003


class CHT8305
{
public:
  CHT8305(TwoWire *wire = &Wire);

#if defined (ESP8266) || defined(ESP32)
  //  address and writeProtectPin is optional
  int      begin(int sda, int scl, const uint8_t address = 0x50);
#endif
  //  address and writeProtectPin is optional
  int      begin(const uint8_t address = 0x50);
  bool     isConnected();

  //  read the temperature and humidity.
  int      read();
  //  lastRead is in MilliSeconds since start sketch
  uint32_t lastRead();
  float    getHumidity();
  float    getTemperature();

  //  adding offsets works well in normal range
  //  might introduce under- or overflow at the ends of the sensor range
  void     setHumOffset(float offset);
  void     setTempOffset(float offset);
  float    getHumOffset();
  float    getTempOffset();

  //  CONFIG REGISTER
  void     setConfigRegister(uint16_t bitmask);
  uint16_t getConfigRegister();
  //
  //  |  bit  |  mask  |  name           |  description  |
  //  |:-----:|:------:|:----------------|:--------------|
  //  |  15   | 0x8000 |  soft reset     |  1 = reboot the sensor to default
  //  |  14   | 0x4000 |  clock stretch  |  1 = ON, 0 = OFF (default)
  //  |  13   | 0x2000 |  heater         |  1 = ON, 0 = OFF (default)  ==> user is responsible for timing!
  //  |  12   | 0x1000 |  mode           |  1 = read both (default), 0 = read T or RH
  //  |  11   | 0x0800 |  vccs           |  1 = >2.8V,  0 = <2.8V
  //  |  10   | 0x0400 |  T-RES          |  1 = 11 bit, 0 = 14 bit (default)
  //  |  9-8  | 0x0300 |  H-RES          |  10 = 8 bit, 01 = 11 bit, 00 = 14 bit (default)
  //  |  7-6  | 0x00C0 |  ALTM           |  Alert Mode (datasheet)
  //  |  5    | 0x0020 |  APS            |  Alert pending status
  //  |  4    | 0x0010 |  H-ALT          |  Humidity Alert status
  //  |  3    | 0x0004 |  T-ALT          |  Temperature Alert status
  //  |  2    | 0x0002 |  VCC enable     |  1 = enable VCC measurement, 0 = disable (default)
  //  |  1-0  | 0x0003 |  reserved.      |  do not change.
  //
  //  specific config functions. See datasheet!
  //
  void     softReset();

  void     setI2CClockStretch(bool on = false);
  bool     getI2CClockStretch();

  void     setHeaterOn(bool on = false);              //  WARNING: user is responsible for timing!
  bool     getHeater();

  void     setMeasurementMode(bool both = true);
  bool     getMeasurementMode();

  bool     getVCCstatus();

  void     setTemperatureResolution(uint8_t res = 0); //  1 = 11 bit, other = 14 bit
  uint8_t  getTemperatureResolution();

  void     setHumidityResolution(uint8_t res = 0);    //  2 = 8 bit, 1 = 11 bit, other = 14 bit
  uint8_t  getHumidityResolution();                   //  idem

  void     setVCCenable(bool enable = false);
  bool     getVCCenable();

  //  ALERT FUNCTIONS
  //  mode   trigger
  //    0      T or H     (default)
  //    1      T
  //    2      H
  //    3      T and H
  bool     setAlertTriggerMode(uint8_t mode = 0);     //  0, 1, 2, 3
  uint8_t  getAlertTriggerMode();
  bool     getAlertPendingStatus();
  bool     getAlertHumidityStatus();
  bool     getAlertTemperatureStatus();

  //       it is mandatory to set both values.
  //       optionally use 125.0 for temperature and 100.0 for humidity
  bool     setAlertLevels(float temperature, float humidity);
  float    getAlertLevelTemperature();
  float    getAlertLevelHumidity();


  //  VOLTAGE
  float    getVoltage();


  //  META DATA
  uint16_t getManufacturer();     //  expect 0x5959
  uint16_t getVersionID();        //  may vary


private:
  float    _humOffset     = 0.0;
  float    _tempOffset    = 0.0;
  float    _humidity      = 0.0;
  float    _temperature   = 0.0;
  uint32_t _lastRead      = 0;

  TwoWire* _wire;
  uint8_t  _address       = 0x40;

  int      _readRegister(uint8_t reg, uint8_t * buf, uint8_t size);
  int      _writeRegister(uint8_t reg, uint8_t * buf, uint8_t size);

  void     _setConfigMask(uint16_t mask);
  void     _clrConfigMask(uint16_t mask);
};


// -- END OF FILE --
