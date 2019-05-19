#include <cm1106_i2c.h>

CM1106_I2C cm1106_i2c;

void setup() {
  cm1106_i2c.begin();
  Serial.begin(9600);
  delay(1000);
  cm1106_i2c.read_serial_number();
  delay(1000);
  cm1106_i2c.check_sw_version();
  delay(1000);
}

void loop() {
  uint8_t ret = cm1106_i2c.measure_result();

  if (ret == 0) {
    Serial.print("Co2 : ");
    Serial.println(cm1106_i2c.co2);
    Serial.print("Status : ");
    switch (cm1106_i2c.status) {
      case CM1106_I2C_STATUS_PREHEATING: {
          Serial.println("Preheating");
          break;
        }
      case CM1106_I2C_STATUS_NORMAL_OPERATION: {
          Serial.println("Normal operation");
          break;
        }
      case CM1106_I2C_STATUS_OPERATING_TROUBLE: {
          Serial.println("Operating trouble");
          break;
        }
      case CM1106_I2C_STATUS_OUT_OF_FS: {
          Serial.println("Out of FS");
          break;
        }
      case CM1106_I2C_STATUS_NON_CALIBRATED: {
          Serial.println("Non calibrated");
          break;
        }
    }
  }
  delay(1000);
}