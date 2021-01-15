/*
pm2008_i2c.cpp -- Arduino library to control Cubic PM2008 I2C

Copyright (c) 2018 Neosarchizo.
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

#include "cm1106_i2c.h"

/**
 * Start CM1106 I2C library
 */
void CM1106_I2C::begin(TwoWire &wirePort)
{
  _i2cPort = &wirePort; //Grab which port the user wants us to use

  //We expect caller to begin their I2C port, with the speed of their choice external to the library
  //But if they forget, we start the hardware here.
  _i2cPort->begin();
}

/**
 * Read result of measuring
 * @return Status
 */
uint8_t CM1106_I2C::get_status()
{
  return status;
}

/**
 * Read result of measuring
 * @return {@code 0} Reading PM2008 value succeeded
 *         {@code 1} Buffer(index) is short
 *         {@code 2} Frame header is different
 *         {@code 3} Checksum is wrong
 */
uint8_t CM1106_I2C::measure_result()
{
  _i2cPort->beginTransmission(CM1106_I2C_ADDRESS);
  _i2cPort->write(CM1106_I2C_CMD_MEASURE_RESULT);
  _i2cPort->endTransmission();
  delay(CM1106_I2C_DELAY_FOR_ACK);
  _i2cPort->requestFrom(CM1106_I2C_ADDRESS, 5);
  uint8_t idx = 0;

  while (_i2cPort->available())
  { // slave may send less than requested
    uint8_t b = _i2cPort->read();
    _buffer[idx++] = b;
    if (idx == 5)
      break;
  }

  if (idx < 5)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.println("CM1106_I2C::measure_result : buffer is short!");
#endif
    return 1;
  }

  // Check frame header
  if (_buffer[0] != CM1106_I2C_CMD_MEASURE_RESULT)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::measure_result : frame header is different ");
    Serial.println(_buffer[0], HEX);
#endif
    return 2;
  }

  // Check checksum
  uint8_t check_code = 0;
  
  for (uint8_t i = 0; i < 4; i++)
  {
    check_code -= _buffer[i];
  }

  if (_buffer[4] != check_code)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::measure_result failed : check code is different - _buffer[4] : ");
    Serial.print(_buffer[4], HEX);
    Serial.print(", check_code : ");
    Serial.println(check_code, HEX);
#endif
    return 3;
  }

  co2 = (_buffer[1] << 8) + _buffer[2];
  /// Status
  status = _buffer[3];

  return 0;
}

/**
 * Auto zero setting
 * @return {@code 0} Reading PM2008 value succeeded
 *         {@code 1} Buffer(index) is short
 *         {@code 2} Frame header is different
 *         {@code 3} Checksum is wrong
 *         {@code 4} zero_setting_switch is not 0 or 2
 *         {@code 5} period is not between 1 and 15
 *         {@code 6} concentration_value is not between 400 and 1499
 */
uint8_t CM1106_I2C::auto_zero_setting(uint8_t zero_setting_switch, uint8_t period, uint16_t concentration_value)
{
  if (zero_setting_switch != 0 && zero_setting_switch != 2)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::auto_zero_setting failed : zero_setting_switch is not 0 or 2 : ");
    Serial.println(zero_setting_switch);
#endif
    return 4;
  }

  if (period < 1 || 15 < period)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::auto_zero_setting failed : period is not between 1 and 15 : ");
    Serial.println(period);
#endif
    return 5;
  }

  if (concentration_value < 400 || 1499 < concentration_value)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::auto_zero_setting failed : concentration_value is not between 400 and 1499 : ");
    Serial.println(concentration_value);
#endif
    return 6;
  }

  _i2cPort->beginTransmission(CM1106_I2C_ADDRESS);
  _i2cPort->write(CM1106_I2C_CMD_AUTO_ZERO_SETTING);
  _i2cPort->write(100);                               // Wrong code accelerate value  : (default) 100
  _i2cPort->write(zero_setting_switch);               // Zero setting switch : 0(Open), 2(Close)
  _i2cPort->write(period);                            // Calibration period : 1 ~ 15
  _i2cPort->write((concentration_value >> 8) & 0xFF); // Calibration conecntration value : 400 ~ 1,499
  _i2cPort->write(concentration_value & 0xFF);        // Calibration conecntration value
  _i2cPort->write(100);                               // Reserved byte : (default) 100
  _i2cPort->endTransmission();
  delay(CM1106_I2C_DELAY_FOR_ACK);
  _i2cPort->requestFrom(CM1106_I2C_ADDRESS, 8);
  uint8_t idx = 0;

  while (_i2cPort->available())
  { // slave may send less than requested
    uint8_t b = _i2cPort->read();
    _buffer[idx++] = b;
    if (idx == 8)
      break;
  }

  if (idx < 8)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.println("CM1106_I2C::auto_zero_setting : buffer is short!");
#endif
    return 1;
  }

  // Check frame header
  if (_buffer[0] != CM1106_I2C_CMD_AUTO_ZERO_SETTING)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::auto_zero_setting : frame header is different ");
    Serial.println(_buffer[0], HEX);
#endif
    return 2;
  }

  // Check checksum
  uint8_t check_code = 0;
  
  for (uint8_t i = 0; i < 7; i++)
  {
    check_code -= _buffer[i];
  }

  if (_buffer[7] != check_code)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::auto_zero_setting failed : check code is different - _buffer[7] : ");
    Serial.print(_buffer[7], HEX);
    Serial.print(", check_code : ");
    Serial.println(check_code, HEX);
#endif
    return 3;
  }

#ifdef CM1106_I2C_DEBUG
  Serial.print("Wrong code accelerate value : ");
  Serial.println(_buffer[1]);

  Serial.print("Zero setting switch : ");
  Serial.println(_buffer[2]);

  Serial.print("Calibration period : ");
  Serial.println(_buffer[3]);

  Serial.print("Calibration concentration value : ");
  Serial.println((_buffer[4] << 8) + _buffer[5]);

  Serial.print("Reserved byte : ");
  Serial.println(_buffer[6]);
#endif
}

/**
 * Read serial
 * @return {@code 0} Reading PM2008 value succeeded
 *         {@code 1} Buffer(index) is short
 *         {@code 2} Frame header is different
 *         {@code 3} Checksum is wrong
 *         {@code 4} adjust_value is not between 400 and 1500
 */
uint8_t CM1106_I2C::calibration(uint16_t adjust_value)
{

  if (adjust_value < 400 || 1500 < adjust_value)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::calibration failed : adjust_value is not between 400 and 1500 : ");
    Serial.println(adjust_value);
#endif
    return 4;
  }

  _i2cPort->beginTransmission(CM1106_I2C_ADDRESS);
  _i2cPort->write(CM1106_I2C_CMD_CALIBRATION);
  _i2cPort->write((adjust_value >> 8) & 0xFF);
  _i2cPort->write(adjust_value & 0xFF);
  _i2cPort->endTransmission();
  delay(CM1106_I2C_DELAY_FOR_ACK);
  _i2cPort->requestFrom(CM1106_I2C_ADDRESS, 4);
  uint8_t idx = 0;

  while (_i2cPort->available())
  { // slave may send less than requested
    uint8_t b = _i2cPort->read();
    _buffer[idx++] = b;
    if (idx == 4)
      break;
  }

  if (idx < 4)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.println("CM1106_I2C::calibration : buffer is short!");
#endif
    return 1;
  }

  // Check frame header
  if (_buffer[0] != CM1106_I2C_CMD_CALIBRATION)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::calibration : frame header is different ");
    Serial.println(_buffer[0], HEX);
#endif
    return 2;
  }

  // Check checksum
  uint8_t check_code = 0;

  for (uint8_t i = 0; i < 3; i++)
  {
    check_code -= _buffer[i];
  }

  if (_buffer[3] != check_code)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::calibration failed : check code is different - _buffer[4] : ");
    Serial.print(_buffer[3], HEX);
    Serial.print(", check_code : ");
    Serial.println(check_code, HEX);
#endif
    return 3;
  }

#ifdef CM1106_I2C_DEBUG
  Serial.print("Adjust value : ");
  Serial.println((_buffer[1] << 8) + _buffer[2]);
#endif
}

/**
 * Read serial number
 * @return {@code 0} Reading PM2008 value succeeded
 *         {@code 1} Buffer(index) is short
 *         {@code 2} Frame header is different
 *         {@code 3} Checksum is wrong
 */
uint8_t CM1106_I2C::read_serial_number()
{

  _i2cPort->beginTransmission(CM1106_I2C_ADDRESS);
  _i2cPort->write(CM1106_I2C_CMD_READ_SERIAL_NUMBER);
  _i2cPort->endTransmission();
  delay(CM1106_I2C_DELAY_FOR_ACK);
  _i2cPort->requestFrom(CM1106_I2C_ADDRESS, 12);
  uint8_t idx = 0;

  while (_i2cPort->available())
  { // slave may send less than requested
    uint8_t b = _i2cPort->read();
    _buffer[idx++] = b;
    if (idx == 12)
      break;
  }

  if (idx < 12)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.println("CM1106_I2C::read_serial_number : buffer is short!");
#endif
    return 1;
  }

  // Check frame header
  if (_buffer[0] != CM1106_I2C_CMD_READ_SERIAL_NUMBER)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::read_serial_number : frame header is different ");
    Serial.println(_buffer[0], HEX);
#endif
    return 2;
  }

  // Check checksum
  uint8_t check_code = 0;
  
  for (uint8_t i = 0; i < 11; i++)
  {
    check_code -= _buffer[i];
  }

  if (_buffer[11] != check_code)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::read_serial_number failed : check code is different - _buffer[11] : ");
    Serial.print(_buffer[11], HEX);
    Serial.print(", check_code : ");
    Serial.println(check_code, HEX);
#endif
    return 3;
  }

  Serial.println("Serial number >>");

  Serial.print((_buffer[1] << 8) + _buffer[2], HEX);
  Serial.print("\t");

  Serial.print((_buffer[3] << 8) + _buffer[4], HEX);
  Serial.print("\t");

  Serial.print((_buffer[5] << 8) + _buffer[6], HEX);
  Serial.print("\t");

  Serial.print((_buffer[7] << 8) + _buffer[8], HEX);
  Serial.print("\t");

  Serial.println((_buffer[9] << 8) + _buffer[10], HEX);
}

/**
 * Check SW version
 * @return {@code 0} Reading PM2008 value succeeded
 *         {@code 1} Buffer(index) is short
 *         {@code 2} Frame header is different
 *         {@code 3} Checksum is wrong
 */
uint8_t CM1106_I2C::check_sw_version()
{

  _i2cPort->beginTransmission(CM1106_I2C_ADDRESS);
  _i2cPort->write(CM1106_I2C_CMD_CHECK_SW_VERSION);
  _i2cPort->endTransmission();
  delay(CM1106_I2C_DELAY_FOR_ACK);
  _i2cPort->requestFrom(CM1106_I2C_ADDRESS, 12);
  uint8_t idx = 0;

  while (_i2cPort->available())
  { // slave may send less than requested
    uint8_t b = _i2cPort->read();
    _buffer[idx++] = b;
    if (idx == 12)
      break;
  }

  if (idx < 12)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.println("CM1106_I2C::check_sw_version : buffer is short!");
#endif
    return 1;
  }

  // Check frame header
  if (_buffer[0] != CM1106_I2C_CMD_CHECK_SW_VERSION)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::check_sw_version : frame header is different ");
    Serial.println(_buffer[0], HEX);
#endif
    return 2;
  }

  // Check checksum
  uint8_t check_code = 0;

  for (uint8_t i = 0; i < 11; i++)
  {
    check_code -= _buffer[i];
  }

  if (_buffer[11] != check_code)
  {
#ifdef CM1106_I2C_DEBUG
    Serial.print("CM1106_I2C::check_sw_version failed : check code is different - _buffer[11] : ");
    Serial.print(_buffer[11], HEX);
    Serial.print(", check_code : ");
    Serial.println(check_code, HEX);
#endif
    return 3;
  }

  Serial.println("SW version >>");

  for (uint8_t i = 1; i < 10; i++)
  {
    Serial.print((char)_buffer[i]);
  }
  Serial.println();
}