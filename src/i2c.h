#ifndef _I2C_H
#define _I2C_H

#include <stddef.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SET_BIT(reg, val) (reg |= val); 
#define CLR_BIT(reg, val) (reg &= ~(val));

// SCL_freq = (F_CPU) / (16 + 2 (TWBR * PrescalerValue))

#define SCL_FREQ_100kHz     100000L
#define SCL_FREQ            SCL_FREQ_100kHz     // kHz

// 0xf8
#define TWI_STATUS_MASK ((1 << TWS7) | (1 << TWS6) | (1 << TWS5) | (1 << TWS4) | (1 << TWS3))

// rename to TWI_MR_ // twi master receive
#define TWI_STATUS_START      0x08
#define TWI_STATUS_RSTART     0x10
#define TWI_STATUS_SLAR_ARBL  0x38
#define TWI_STATUS_SLAR_ACK   0x40
#define TWI_STATUS_SLAR_NACK  0x48
#define TWI_STATUS_DATA_ACK   0x50
#define TWI_STATUS_DATA_NACK  0x58

#define TWI_READ              0b1
#define TWI_WRITE             0b1

void twi_init(void);
void twi_wait(uint8_t status);
void twi_exit(uint8_t err);


#ifdef __cplusplus
}
#endif


#endif