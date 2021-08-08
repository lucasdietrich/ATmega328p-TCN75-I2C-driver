#include "i2c.h"
#include "uart.h"

#include <avr/sleep.h>

void twi_init(void)
{
    // init TWI bit rate prescaler to 1
    CLR_BIT(TWSR, 1 << TWPS0);
    CLR_BIT(TWSR, 1 << TWPS1);

    // init Bit Rate Generator Unit
    TWBR = ((F_CPU / SCL_FREQ) - 16) / 2;

    // set internal pullups on SDA, SCL
    DDRC  &= ~(_BV(PC4) | _BV(PC5));
    PORTC |= _BV(PC4) | _BV(PC5);

    // enable Two-Wire Peripherals
    // disable interrupt | _BV(TWIE)
    //  | _BV(TWEA);
    TWCR = _BV(TWEN);
}

void twi_wait(uint8_t status)
{
    usart_print("Waiting ... expected status TWSR[2:7] = 0x");
    usart_hex(status);
    usart_transmit('\n');

    while(!(TWCR & (1 << TWINT)));

    if ((TWSR & TWI_STATUS_MASK) != status)
    {
        twi_exit(TWSR & TWI_STATUS_MASK);
    }
}

void twi_exit(uint8_t err)
{
  usart_print("error : TWSR[2:7] = 0x");
  usart_hex(err);

  sleep_cpu();
}