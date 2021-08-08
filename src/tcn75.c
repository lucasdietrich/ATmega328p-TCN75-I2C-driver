#include "tcn75.h"

// @see DS21490D-page 11
float tcn75_temp2float(uint8_t t1, uint8_t t2)
{
    float temp;

    uint8_t msb = t1 >> 7;
    uint8_t value = t1 << 1 | t2 >> 7;

    // temp < 0
    if (msb)
    {
        temp = -0.5 * ~value;
    }
    else
    { // temp >= 0
        temp = 0.5 * value;
    }

    return temp;
}