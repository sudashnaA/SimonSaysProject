#include "timer.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#include "display.h"
#include "display_macros.h"

volatile uint8_t pb_debounced_state = 0xFF;
volatile uint8_t count = 0;
volatile uint16_t elapsed_time = 0;

static void pb_debounce(void)
{
    // Sample pushbutton state
    uint8_t pb_sample = PORTA.IN;

    static uint8_t vcount0=0, vcount1=0;   //vertical counter bits

    uint8_t pb_changed = (pb_sample ^ pb_debounced_state);

    //vertical counter update
    vcount1 = (vcount1 ^ vcount0) & pb_changed;  //vcount1 (bit 1 of vertical counter)
    vcount0 = ~vcount0 & pb_changed;             //vcount0 (bit 0 of vertical counter)

    // This variable contains a raw sample of the pushbuttons.
    // Modify this code to implement debouncing correctly.
    pb_debounced_state ^= (vcount0 & vcount1);        //toggle pb_debounced
}

static void spi_write(void)
{
    static uint8_t current_side = 0;

    if (current_side)
    {
        SPI0.DATA = left_byte;
    }
    else
    {
        SPI0.DATA = right_byte;
    }

    // Toggle the current side
    current_side = !current_side;
}

ISR(TCB1_INT_vect)
{
    pb_debounce();
    spi_write();

    TCB1.INTFLAGS = TCB_CAPT_bm;
}

ISR(TCB0_INT_vect)
{
    if (count == 1)
    {
        elapsed_time++;
    }
    TCB0.INTFLAGS = TCB_CAPT_bm;
}