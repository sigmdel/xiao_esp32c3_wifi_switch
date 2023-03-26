/*!
 *@file DFRobot_DHT20.cpp
 *@brief Define the basic structure of class DFRobot_DHT20, the implementation of basic methods.
 *@copyright   Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence     The MIT License (MIT)
 *@author [fengli](li.feng@dfrobot.com)
 *@version  V1.0
 *@date  2021-6-24
 *@get from https://www.dfrobot.com
 *@https://github.com/DFRobot/DFRobot_DHT20
*/


#ifndef DFRobot_DHT20_H
#define DFRobot_DHT20_H

#include <Arduino.h>
#include <string.h>
#include <Wire.h>
//#define ENABLE_DBG

#ifdef ENABLE_DBG
#define DBG(...) {Serial.print("[");Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DBG(...)
#endif
//extern Stream *dbg;

struct TempAndHumidity_t {
  float temperature;
  float humidity;
};


class DFRobot_DHT20
{
public:
  /*!
   * @brief Construct the function
   * @param pWire IC bus pointer object and construction device, can both pass or not pass parameters, Wire in default.
   * @param address Chip IIC address, 0x38 in default.
   */
  DFRobot_DHT20(TwoWire *pWire = &Wire, uint8_t address = 0x38);

  /**
   * @brief init function
   * @return Return 0 if initialization succeeds, otherwise return non-zero and error code.
   */
  int begin(void);

  /**
   * @brief Get ambient temperature, unit: °C
   * @return ambient temperature, measurement range: -40°C ~ 80°C
   */
  float getTemperature();

  /**
   * @brief Get relative humidity, unit: %RH.
   * @return relative humidity, measurement range: 0-100%
   */
  float getHumidity();

  /**
   * @brief Get ambiant temperature (°C) and relative humidity (%RH) in a combined operation
   * @return TempAndHumidity_t = temperature (-40°C - 80°C) and humidity measurements (0 - 100%)
   */
  TempAndHumidity_t getTempAndHumidity();

private:

  /**
   * @brief Write command into sensor chip
   * @param pBuf  Data included in command
   * @param size  The number of the byte of command
   */
    void  writeCommand(const void *pBuf,size_t size);
  /**
   * @brief Write command into sensor chip
   * @param pBuf  Data included in command
   * @param size  The number of the byte of command
   * @return      Return 0 if the reading is done, otherwise return non-zero.
   */
    uint8_t  readData(void *pBuf,size_t size);

    void readSensor(void);

    TwoWire *_pWire;
    uint8_t _address;

    float temperature;
    float humidity;

};

#endif
