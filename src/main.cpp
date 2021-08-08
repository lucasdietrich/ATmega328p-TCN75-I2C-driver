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
}

static uint8_t rx_buffer[2];

int main(void)
{
  usart_init();  
  usart_printl("TCN75 example starting");

  sei();

  _delay_ms(1000);

  twi_init();

  // Master Receiver mode : 2 bytes
  twi_recv(TCN75_ADDR, rx_buffer, 2);

  while(1)
  {
    if (twi_state == READY)
    {
      usart_printl("I2C recv finished");
      double temp = tcn75_temp2float(rx_buffer[0], rx_buffer[1]);
      print_temp(temp);

      _delay_ms(1000);

      twi_recv(TCN75_ADDR, rx_buffer, 2);
    }
  }

  sleep_cpu();
}