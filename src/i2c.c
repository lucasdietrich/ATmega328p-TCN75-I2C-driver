#include "i2c.h"
#include "uart.h"

#include <avr/sleep.h>

/*___________________________________________________________________________*/

volatile uint8_t state;

static volatile uint8_t sla_rw;


static volatile uint8_t tx_len;
static volatile uint8_t tx_buffer[1];

static volatile uint8_t rx_len;
static volatile uint8_t *rx_buffer;

/*___________________________________________________________________________*/


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

    // enable Two-Wire Peripherals (+ interrupt)
    // _BV(TWEA); ?
    TWCR = _BV(TWEN) | _BV(TWIE);
}

void twi_set_addr(uint8_t addr)
{
    sla_rw = ((addr & 0x7F) << 1);
}

void twi_recv(const uint8_t addr, uint8_t *const buffer, const uint8_t len)
{
    twi_set_addr(addr);

    state = MASTER_RECEIVER;

    rx_buffer = buffer;
    rx_len = len;
    
    _twi_start();
}

/*___________________________________________________________________________*/

ISR(TWI_vect)
{
    const uint8_t status = TWSR & TW_STATUS_MASK;
    usart_print("TW interrupt status = 0x");
    usart_hex(status);
    usart_transmit('\n');

    switch(status)
    {
        case TW_START:
        case TW_REP_START:
            if (state == MASTER_TRANSMITTER)
            {
                TWDR = sla_rw | TW_WRITE;
            }
            else
            {
                TWDR = sla_rw | TW_READ;
            }
            _twi_reply(1);
            break;
        
        
        case TW_MR_DATA_ACK:
            rx_buffer[--rx_len] = TWDR;

        case TW_MR_SLA_ACK:
            _twi_reply(rx_len > 1 ? TW_ACK : TW_NACK);
            break;

        case TW_MR_DATA_NACK:
            rx_buffer[--rx_len] = TWDR;
            state = READY;   // driver is ready again
            _twi_stop();
            break;

        case TW_MR_ARB_LOST:
        case TW_MR_SLA_NACK:
            _twi_start();
            break;

        case TW_NO_INFO:
        case TW_BUS_ERROR:
            _twi_exit(status);  // todo remove

            state = READY;
            break;
    }
}

void _twi_start(void)
{
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
}

void _twi_write(uint8_t data)
{
    sla_rw |= TW_WRITE;

    // set tx buffer
    tx_len = 1;
    *tx_buffer = data;
}

void _twi_reply(uint8_t ack)
{
    if (ack == TW_ACK) // ack
    {
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
    }
    else // nack
    {
        TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWEA);
    }
}

void _twi_stop(void)
{
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTO);
}

void _twi_reset(void)
{
    TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTO) | _BV(TWSTA);
}

void _twi_exit(uint8_t err)
{
  usart_print("error : TWSR[2:7] = 0x");
  usart_hex(err);

  sleep_cpu();
}

/*___________________________________________________________________________*/