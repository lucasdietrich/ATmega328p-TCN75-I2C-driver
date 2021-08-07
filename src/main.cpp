#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#include "uart.h"

#include "i2c.h"

int main(void)
{
  usart_init();
  
  usart_printl("TCN75 example starting");

  sleep_cpu();
}