#include <avr/interrupt.h>
#include "buzzer.h"
#include "display.h"
#include "display_macros.h"
#include "initialisation.h"
#include "timer.h"
#include "sequence.h"
#include "state.h"

// initialise variables
volatile SP_STATE state = START;
volatile SP_STATE nextstate;
volatile uint8_t counter = 0;
volatile uint8_t sequence_length = 1;
volatile uint8_t selected_button;
volatile uint16_t playback_delay;
volatile uint32_t seedoriginal = 0x11594268;

// bitmasks for the seven segment display, from 0-9
volatile uint8_t segs [] = {
    0x08,0x6B,0x44,0x41,0x23,0x11,0x10,0x4B,0x00,0x01
};

// gets the nth digit of an integer, n is the digit & k is the integer
uint8_t nthdig(uint8_t n, uint8_t k)
{
    while(n--)
        k/=10;
    return k%10;
}

int main(void)
{
    // init seed values, step & delay stage
    uint32_t seed = 0x11594268;
    uint32_t seed1 = 0x11594268;
    uint32_t step;
    uint8_t delay_stage = 0;

    // init all peripherals
    cli();
    adc_init();
    pwm_init();
    spi_init();
    tcb0_init();
    button_init();
    timer_init();
    uart_init();
    sei();

    // init debouncing variables & variables to store gameplay values.
    uint8_t pb_current = 0xFF;
    uint8_t pb_prev = 0xFF;
    uint8_t pb_changed;
    uint8_t pb_falling;
    uint8_t pb_rising;
    uint8_t selected_tone;
    uint8_t disp_left;
    uint8_t disp_right;
    uint8_t score;

    // Main loop
    while (1)
    {
        // values to check state of pushbuttons
        pb_prev = pb_current;
        pb_current = pb_debounced_state;
        pb_changed = pb_prev ^ pb_current;
        pb_falling = pb_changed & pb_prev;
        pb_rising = pb_changed & pb_current;

        switch(state)
        {
            case START:
            // set counter to 0, this tracks what step simon is currently on
            // reset seeds, turn display & buzzer off & have no button selected.
            counter = 0;
            selected_button = 0;
            seed = seedoriginal;
            seed1 = seedoriginal;
            stop_tone();
            display_update(DISP_OFF, DISP_OFF);
            // start conversion
            ADC0.COMMAND = ADC_START_IMMEDIATE_gc;
            // when conversion ready set playback delay
            while ((ADC0.INTFLAGS & ADC_RESRDY_bm) == 0)
            {
            }
            playback_delay = (uint16_t)((((ADC0.RESULT) * 2000) >> 8) + ((250*(255 - ADC0.RESULT)) >> 8));
            // clear intflag
            ADC0.INTFLAGS = ADC_RESRDY_bm;
            state = SIMON;
            break;

            case SIMON:
            // simon playing
                counter++;
                if (counter <= sequence_length)
                {
                    // seed will be sequenced
                    seed = sequence(seed);
                }
                // step value, will be 0,1,2,3 & are the possible steps simon can play
                step = seed & 0b11;
                nextstate = SIMON;
                // if sequence length has been reached
                if (counter > sequence_length)
                {
                    counter = 0;
                    state = PLAYING;
                }
                // tone Eh, segment EF on digit1
                else if (step == 0)
                {
                    state = TONE0;
                }
                // tone c#, segment BC on digit1
                else if (step == 1)
                {
                    state = TONE1;
                }
                // tone A, segment EF on digit2
                else if (step == 2)
                {
                    state = TONE2;
                }
                // tone El, segment BC on digit2
                else if (step == 3)
                {
                    state = TONE3;
                }
            break;

            case TONE0:
                // sets tone to Eh
                selected_tone = 0;
                // selects segment EF on left digit
                disp_left = DISP_BAR_LEFT;
                disp_right = DISP_OFF;
                // if the player is playing the game, will go to BUTTONON state to display
                if (selected_button)
                {
                    state = BUTTONON;
                }
                // if simon is playing then will go straight to displaying
                else
                {
                state = DISPLAY;
                }
            break;

            case TONE1:
                // sets tone to C#
                selected_tone = 1;
                // selected segment is BC on digit 1
                disp_left = DISP_BAR_RIGHT;
                disp_right = DISP_OFF;
                if (selected_button)
                {
                    state = BUTTONON;
                }
                else
                {
                state = DISPLAY;
                }
            break;

            case TONE2:
                // sets tone to A
                selected_tone = 2;
                // selected segment is EF on digit 2
                disp_left = DISP_OFF;
                disp_right = DISP_BAR_LEFT;
                if (selected_button)
                {
                    state = BUTTONON;
                }
                else
                {
                state = DISPLAY;
                }
            break;

            case TONE3:
                // selected tone is El
                selected_tone = 3;
                // selected segment is BC on digit 2
                disp_left = DISP_OFF;
                disp_right = DISP_BAR_RIGHT;
                if (selected_button)
                {
                    state = BUTTONON;
                }
                else
                {
                state = DISPLAY;
                }
            break;

            case DISPLAY:
                // display the selected segment & play the selected tone
                display_update(disp_left,disp_right);
                play_tone(selected_tone);
                // begin count for the playback delay
                count = 1;
                state = WAITING;
            break;

            case WAITING:
            // will turn off buzzer & tone at half of the playback delay then remain silent / off 
            // for the remainder of the playback delay
                if (delay_stage == 0 && elapsed_time >= playback_delay >> 1)
                {
                    // first stage of delay
                    stop_tone();
                    display_update(DISP_OFF, DISP_OFF);
                    delay_stage = 1;
                    // if user was successful or failed then 2nd delay stage will be skipped
                    if (nextstate == SUCCESS)
                    {
                        delay_stage = 0;
                        count = 0;
                        elapsed_time = 0;
                        state = SUCCESS;
                    }
                    else if (nextstate == FAIL)
                    {
                        delay_stage = 0;
                        count = 0;
                        elapsed_time = 0;
                        state = FAIL;
                    }
                }
                else if (delay_stage == 1 && elapsed_time >= playback_delay)
                {
                    // second delay stage
                    delay_stage = 0;
                    count = 0;
                    elapsed_time = 0;
                    state = nextstate;
                }
            break;

            case WAITINGLONG:
            // will turn display off when playback delay has been reached
                if (delay_stage == 0 && elapsed_time >= playback_delay >> 1)
                {
                    stop_tone();
                    delay_stage = 1;
                }
                else if (delay_stage == 1 && elapsed_time >= playback_delay)
                {
                    display_update(DISP_OFF, DISP_OFF);
                    delay_stage = 0;
                    elapsed_time = 0;
                    count = 0;
                    state = nextstate;
                }
            break;

            case PLAYING:
            // player playing
                counter++;
                if (counter <= sequence_length)
                {
                    // seed will be sequenced
                    seed1 = sequence(seed1);
                }
                step = seed1 & 0b11;
                nextstate = PLAYING;
                // correct button is s1
                if (step == 0)
                {
                    selected_button = PIN4_bm;
                    state = BUTTONCHECK;
                }
                // correct button is s2
                else if (step == 1)
                { 
                    selected_button = PIN5_bm;
                    state = BUTTONCHECK;
                }
                // correct button is s3
                else if (step == 2)
                { 
                    selected_button = PIN6_bm;
                    state = BUTTONCHECK;
                }
                // correct button is s4
                else if (step == 3)
                { 
                    selected_button = PIN7_bm;
                    state = BUTTONCHECK;
                }
            break;

            case BUTTONCHECK:
                if (counter >= sequence_length)
                    {
                        // if the player is on the last step of the round
                        nextstate = SUCCESS;
                    }
                // check if user input matches the correct button
                if (pb_falling & selected_button)
                {
                    // choose a tone based on what user presses
                    if (selected_button == PIN4_bm)
                    {
                        state = TONE0;
                    }
                    else if (selected_button == PIN5_bm)
                    {
                        state = TONE1;
                    }
                    else if (selected_button == PIN6_bm)
                    {
                        state = TONE2;
                    }
                    else if (selected_button == PIN7_bm)
                    {
                        state = TONE3;
                    }
                }
                // if user input is wrong
                else if (pb_falling & (~selected_button))
                {
                    nextstate = FAIL;
                    // tone will be selected based on what user presses
                    if (pb_falling & PIN4_bm)
                    {
                        selected_button = PIN4_bm;
                        state = TONE0;
                    }
                    else if (pb_falling & PIN5_bm)
                    {
                        selected_button = PIN5_bm;
                        state = TONE1;
                    }
                    else if (pb_falling & PIN6_bm)
                    {
                        selected_button = PIN6_bm;
                        state = TONE2;
                    }
                    else if (pb_falling & PIN7_bm)
                    {
                        selected_button = PIN7_bm;
                        state = TONE3;
                    }
                }
            break;

            case BUTTONON:
                    // will play tone & update display if buttons have been pressed
                    play_tone(selected_tone);
                    display_update(disp_left,disp_right);
                    // begin playback delay count
                    count = 1;
                    // when button released
                    if (pb_rising & selected_button)
                    {
                        // if button was held for longer than playback delay then skip waiting state
                        if (elapsed_time >= (playback_delay >> 1))
                        {
                            count = 0;
                            elapsed_time = 0;
                            stop_tone();
                            display_update(DISP_OFF,DISP_OFF);
                            state = nextstate;
                        }
                        // otherwise will go to waiting state
                        else
                        {
                            state = WAITING;

                        }
                    }
            break;

            case SUCCESS:
            display_update(DISP_SEG_SUCCESS, DISP_SEG_SUCCESS);
            printf("SUCCESS\n");
            printf("%u\n",sequence_length);
            sequence_length++;
            state = WAITINGLONG;
            nextstate = START;
            break;

            case FAIL:
            display_update(DISP_SEG_FAIL, DISP_SEG_FAIL);
            printf("GAME OVER\n");
            printf("%u\n",sequence_length); 
            score = sequence_length;
            nextstate = DISPLAY_SCORE;
            state = WAITINGLONG;
            break;

            case DISPLAY_SCORE:
            if (score >= 10)
            {
                uint8_t a = nthdig(0,score);
                uint8_t b = nthdig(1,score);
                display_update(segs[b],segs[a]);
            }
            else if (score < 10)
            {
                uint8_t a = nthdig(0,score);
                display_update(DISP_OFF,segs[a]);
            }
            nextstate = OFF;
            state = WAITINGLONG;
            break;

            case OFF:
            display_update(DISP_OFF,DISP_OFF);
            seedoriginal = seed;
            sequence_length = 1;
            nextstate = START;
            state = WAITINGLONG;
            break;
        }
    }
}