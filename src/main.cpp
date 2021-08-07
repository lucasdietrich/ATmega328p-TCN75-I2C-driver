#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "uart.h"

#include "i2c.h"

#include <Arduino.h>
#include <Wire.h>

// Arduino Pro (ATMega328p)
// SDA : Analog In 4 ()
// SCL : Analog In 5 ()


#define TCN75_ADDR 0b1001000 // with pins 5~7 set to GND, the device address is 0x48w

#define TEMPERATURE_REGISTER 0
#define CONFIG_REGISTER 1
#define HISTERESYS_REGISTER 2
#define SETPONT_REGISTER 3

// @see DS21490D-page 11
float convertTemp(byte t1, byte t2) {
  float temp;

  byte msb = t1 >> 7;
  byte value = t1 << 1 | t2 >> 7;

  // temp < 0
  if (msb) {
    temp = -0.5 * ~value;    
  } else { // temp >= 0
    temp = 0.5 * value;
  }

  return temp;
}

void setup() {
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(250000);  // start serial for output

  // move to temperature register
  Wire.beginTransmission(TCN75_ADDR);
  Wire.write((byte)TEMPERATURE_REGISTER);
  Wire.endTransmission();
}

void loop() {
  Wire.requestFrom(TCN75_ADDR, 2);

  byte t1 = Wire.read();
  byte t2 = Wire.read();

  float temp = convertTemp(t1, t2);

  Serial.print(temp);
  Serial.print(" °C\n");
  
  delay(1000);
}

// int main(void)
// {
//   usart_init();
  
//   usart_printl("TCN75 example starting");

//   sleep_cpu();
// }