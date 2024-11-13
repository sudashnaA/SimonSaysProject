#include "display.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "display_macros.h"

volatile uint8_t left_byte = DISP_OFF;
volatile uint8_t right_byte = DISP_OFF;

void display_update(const uint8_t left, const uint8_t right)
{
    left_byte = left | DISP_LHS;
    right_byte = right;
}


ISR(SPI0_INT_vect)
{
    //rising edge on DISP_LATCH
    PORTA.OUTCLR = PIN1_bm;
    PORTA.OUTSET = PIN1_bm;  
    SPI0.INTFLAGS = SPI_IF_bm;
}

    //    xFABGCDE
    // 1: 01000001
    // 2: 00010100
    // 5: 00000000 success
    // 6: 01110111 fail
