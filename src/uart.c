#include <stdint.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "buzzer.h"
#include "timer.h"
#include "state.h"
#include "sequence.h"

typedef enum
{
    OUTPUT,
    RESET,
} Serial_State;

uint8_t chosen_button;
SP_STATE chosen_state;
Serial_State SERIAL_STATE;
uint32_t seed_inital = 0x11594268;

// -------------------------  SERIAL PARSER  -------------------------

static uint8_t hexchar_to_int(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('a' <= c && c <= 'f')
        return 10 + c - 'a';
    else
        return 16; // Invalid
}

ISR(USART0_RXC_vect)
{
    char rx_data = USART0.RXDATAL;

     if (state == SUCCESS || state == FAIL)
    {
        SERIAL_STATE = OUTPUT;
    }
    else if (state == BUTTONCHECK)
        {
            if (rx_data == '1' || rx_data == 'q')
            {
                chosen_state = TONE0;
                chosen_button = PIN4_bm;
            }
            else if (rx_data == '2' || rx_data == 'w')
            {
                chosen_state = TONE1;
                chosen_button = PIN5_bm;
            }
            else if (rx_data == '3' || rx_data == 'e')
            {
                chosen_state = TONE2;
                chosen_button = PIN6_bm;
            }
            else if (rx_data == '4' || rx_data == 'r')
            {
                chosen_state = TONE3;
                chosen_button = PIN7_bm;
            }
            if (chosen_button != selected_button)
            {
                nextstate = FAIL;
            }
            selected_button = 0;
            state = chosen_state;
    }
            
    switch (SERIAL_STATE)
    {
        case OUTPUT:
            if (state == SUCCESS)
            {
                printf("SUCCESS\n");
                printf("%u\n",sequence_length);
            }
            else if (state == FAIL)
            {
                printf("GAME OVER\n");
                printf("%u\n",sequence_length); 
            }
        break;

        case RESET:
            seedoriginal = 0x11594268;
            reset_tones();
            sequence_length = 1;
            state = START;
        break;

        default:
            SERIAL_STATE = RESET;
        break;
    }

}



/*
 if (rx_data == ',' || rx_data == 'k')
        {
            increase_octave();
        }
        else if (rx_data == '.' || rx_data == 'l')
        {
            decrease_octave();
        }
    if (rx_data == 'p' || rx_data == '0')
        {
            SERIAL_STATE = RESET;
        }


*/