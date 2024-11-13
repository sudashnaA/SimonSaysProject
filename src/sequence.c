#include <stdint.h>
#include <stdio.h>

uint32_t sequence(uint32_t state)
{
    uint32_t STATE_LFSR = state;
    uint32_t bit = STATE_LFSR & 1;
    STATE_LFSR >>= 1;
    if (bit == 1)
    {
        STATE_LFSR ^= 0xE2023CAB;
    }
    return STATE_LFSR;
}
