/*
cm1106_i2c.h -- Arduino library to control Cubic CM1106 I2C

Copyright (c) 2019 Neosarchizo.
All rights reserved.

This file is part of the library CM1106 I2C.

CM1106 I2C is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

CM1106 I2C is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _CM1106_I2C_h
#define _CM1106_I2C_h

#include <Arduino.h>
#include <Wire.h>

#define CM1106_I2C_ADDRESS 0x31

// Packet Format
// [Command][DF0][DF1]...[DFn][CS]

// Commands
#define CM1106_I2C_CMD_MEASURE_RESULT 0x01
#define CM1106_I2C_CMD_AUTO_ZERO_SETTING 0x10
#define CM1106_I2C_CMD_CALIBRATION 0x03
#define CM1106_I2C_CMD_READ_SERIAL_NUMBER 0x1F
#define CM1106_I2C_CMD_CHECK_SW_VERSION 0x1E

// Status
#define CM1106_I2C_STATUS_PREHEATING 0x0
#define CM1106_I2C_STATUS_NORMAL_OPERATION 0x1
#define CM1106_I2C_STATUS_OPERATING_TROUBLE 0x2
#define CM1106_I2C_STATUS_OUT_OF_FS 0x3
#define CM1106_I2C_STATUS_NON_CALIBRATED 0x5

// CM1107
#define CM1106_I2C_STATUS_CM1107_PREHEATING 0x0
#define CM1106_I2C_STATUS_CM1107_OPERATING_NORMAL 0x1
#define CM1106_I2C_STATUS_CM1107_OVER_MEASUREMENT_RANGE 0x2
#define CM1106_I2C_STATUS_CM1107_LESS_THAN_MEASUREMENT_RANGE 0x3
#define CM1106_I2C_STATUS_CM1107_CALIBRATED 0x4
#define CM1106_I2C_STATUS_CM1107_LIGHT_AGING 0x5
#define CM1106_I2C_STATUS_CM1107_DRIFT 0x6

#define CM1106_I2C_DELAY_FOR_ACK 500

// #define CM1106_I2C_DEBUG

class CM1106_I2C
{

public:
  void begin(TwoWire &wirePort = Wire); //By default use Wire port
  uint8_t measure_result();
  uint8_t auto_zero_setting(uint8_t zero_setting_switch, uint8_t period, uint16_t concentration_value);
  uint8_t calibration(uint16_t adjust_value);
  uint8_t read_serial_number();
  uint8_t check_sw_version();
  uint8_t get_status();

  uint8_t status;
  uint16_t co2; // 0 ~ 5,000 ppm

private:
  uint8_t _buffer[32];
  TwoWire *_i2cPort; //The generic connection to user's chosen I2C hardware
};

#endif
