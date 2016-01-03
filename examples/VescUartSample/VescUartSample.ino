#include "VescUart.h"
#include <SoftwareSerial.h>

SoftwareSerial vescUart (10, 11);

// callback functions for VESC Uart library
void fWrite (const uint8_t *what, const size_t lenght) { vescUart.write(what, lenght); }
int fAvailable () { return vescUart.available(); }
int fRead () { return vescUart.read(); }

struct bldcMeasure measuredValues;

void setup() {
  // put your setup code here, to run once:
  vescUart.begin(19200);

  VescUartGetValue(fWrite, fAvailable, fRead, measuredValues);

}

void loop() {
  // put your main code here, to run repeatedly:

}
