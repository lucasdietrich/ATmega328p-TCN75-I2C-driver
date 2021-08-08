#ifndef _I2C_H
#define _I2C_H

#include <stddef.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>

#ifdef __cplusplus
extern "C" {
#endif

/*___________________________________________________________________________*/

#define SET_BIT(reg, val) (reg |= val); 
#define CLR_BIT(reg, val) (reg &= ~(val));

// SCL_freq = (F_CPU) / (16 + 2 (TWBR * PrescalerValue))

#define SCL_FREQ_100kHz     100000L
#define SCL_FREQ            SCL_FREQ_100kHz     // kHz

#define TW_ACK      1
#define TW_NACK     0

#define TW_READ     1
#define TW_WRITE    0

/*___________________________________________________________________________*/

enum { READY, MASTER_RECEIVER, MASTER_TRANSMITTER, ERROR};

extern volatile uint8_t twi_state;

typedef void (*twi_handler_t) (void);

/*___________________________________________________________________________*/

void twi_init(void);
void twi_set_addr(uint8_t addr);
void twi_recv(const uint8_t addr, uint8_t *const buffer, const uint8_t len, twi_handler_t rx_handler);

/*___________________________________________________________________________*/

void _twi_reply(uint8_t ack);
void _twi_start(void);
void _twi_stop(void);
void _twi_write(uint8_t data);

void _rx_handler(void);

// void _twi_send(uint8_t *buffer, const uint8_t len);
// void _twi_recv(uint8_t *buffer, const uint8_t len);

void _twi_exit(uint8_t err);

/*___________________________________________________________________________*/

#ifdef __cplusplus
}
#endif


#endif