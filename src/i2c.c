#include "i2c.h"
#include "uart.h"

#include <avr/sleep.h>

/*___________________________________________________________________________*/

volatile uint8_t twi_state = READY;
volatile uint8_t twi_error = 0; 
volatile uint8_t twi_flags = 0; 

static volatile uint8_t sla_rw;

static volatile uint8_t tx_len;
static volatile uint8_t *tx_buffer;
static volatile uint8_t sent;

static volatile uint8_t rx_len;
static volatile uint8_t *rx_buffer;
static volatile uint8_t received;

static twi_handler_t twi_handler = NULL;

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

void twi_set_handler(twi_handler_t handler)
{
    twi_handler = handler;
}

void twi_set_blocking(const uint8_t blocking)
{
    if (blocking == TW_BLOCKING)
    {
        twi_set_handler(NULL);
        twi_flags |= _BV(TW_BLOCKING_FLAG);
    }
    else
    {
        twi_flags &= ~_BV(TW_BLOCKING_FLAG);
    }
}


int8_t twi_recv(const uint8_t addr, uint8_t *const buffer, const uint8_t len)
{
    if (twi_state == READY)
    {
        twi_set_addr(addr);

        twi_state = MASTER_RECEIVER;
        twi_error = 0;

        rx_buffer = buffer;
        rx_len = len;
        received = 0;

        _twi_start();

        if (twi_flags & _BV(TW_BLOCKING_FLAG))
        {
            while (twi_state != READY);
        }

        return twi_error;
    }
    return -1;
}

int8_t twi_send(const uint8_t addr, uint8_t *const buffer, const uint8_t len)
{
    if (twi_state == READY)
    {
        twi_set_addr(addr);

        twi_state = MASTER_TRANSMITTER;
        twi_error = 0;

        tx_buffer = buffer;
        tx_len = len;
        sent = 0;
        
        _twi_start();
        
        if (twi_flags & _BV(TW_BLOCKING_FLAG))
        {
            while (twi_state != READY);
        }

        return twi_error;
    }
    
    return -1;
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
            if (twi_state == MASTER_TRANSMITTER)
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
            rx_buffer[received++] = TWDR;

        case TW_MR_SLA_ACK:
            _twi_reply(rx_len - received > 1 ? TW_ACK : TW_NACK);
            break;

        case TW_MR_DATA_NACK:
            rx_buffer[received++] = TWDR;
            twi_state = READY;   // driver is ready again
            _twi_stop();
            if (twi_handler != NULL)
            {
                twi_handler();
            }
            break;

        case TW_MR_ARB_LOST:
        case TW_MR_SLA_NACK:
            _twi_start();
            break;
        
        case TW_MT_DATA_ACK:
            sent++;
        case TW_MT_SLA_ACK:
            if (sent < tx_len)
            {
                TWDR = tx_buffer[sent];
                _twi_reply(1);
            }
            else
            {
                twi_state = READY;   // driver is ready again
                _twi_stop();    // all data have been sent
                if (twi_handler != NULL)
                {
                    twi_handler();
                }
            }
            break;
        
        case TW_MT_SLA_NACK:
        case TW_MT_DATA_NACK:
            _twi_stop();
            break;

        case TW_NO_INFO:
        case TW_BUS_ERROR:
            twi_state = READY;
            twi_error = 1;
            _twi_stop();
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