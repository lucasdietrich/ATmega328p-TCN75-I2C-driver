#ifndef _TCN75_H
#define _TCN75_H

#include <stddef.h>
#include <stdint.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#ifdef __cplusplus
extern "C" {
#endif

// TCN75 freq 0 (min) > 100 (typ) > 400 (max) kHz

// A2, A1, A0 = 0
#define TCN75_ADDR  0b1001000

float tcn75_temp2float(uint8_t t1, uint8_t t2);


#ifdef __cplusplus
}
#endif


#endif