#include <Arduino.h>
#include <Wire.h>

#define ADDR_6713  0x15

int readCO2(void)
{
  int data[4];
  Wire.beginTransmission(ADDR_6713);
  Wire.write(0x04);
  Wire.write(0x13);
  Wire.write(0x8B);
  Wire.write(0x00);
  Wire.write(0x01);

  // end transmission
  Wire.endTransmission();
  // read report of current gas measurement in ppm
  delay(10);
  Wire.requestFrom(ADDR_6713, 4);
  if (Wire.available() == 4) {
    data[0] = Wire.read();
    data[1] = Wire.read();
    data[2] = Wire.read();
    data[3] = Wire.read();
  } else {
    return -1;
  }

  return ((data[2] * 0xFF ) + data[3]);
}
