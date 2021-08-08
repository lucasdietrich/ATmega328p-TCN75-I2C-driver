#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include <stdio.h>
#include <string.h>

#include "uart.h"

#include "i2c.h"
#include "tcn75.h"

static void print_temp(double temp)
{
  char temp_str[6];
  sprintf(temp_str, "%.2f", temp);
  usart_print("Temperature : ");
  usart_print(temp_str);
  usart_printl("°C");

  sleep_cpu();
}


uint8_t msb;
uint8_t lsb;

int main(void)
{
  usart_init();  
  usart_printl("TCN75 example starting");

  _delay_ms(1000);

  twi_init();

  // Master Receiver mode : 2 bytes

  TWCR = _BV(TWINT) | _BV(TWSTA) | _BV(TWEN); // send START condition

  twi_wait(TWI_STATUS_START); // waiting for start condition transmitted

  TWDR = (TCN75_ADDR << 1) | TWI_READ;

  TWCR = _BV(TWINT) | _BV(TWEN);

  twi_wait(TWI_STATUS_SLAR_ACK); // waiting for SLA+R transmitted

  // read first byte
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWEA); // Data byte will be received and ACK will be returned
  
  twi_wait(TWI_STATUS_DATA_ACK);

  msb = TWDR;

  // read second byte and nack will be returned
  TWCR = _BV(TWINT) | _BV(TWEN);

  twi_wait(TWI_STATUS_DATA_NACK);

  lsb = TWDR;

  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO); // stop transmission

  usart_printl("I2C recv finished");

  double temp = tcn75_temp2float(msb, lsb);

  print_temp(temp);
}