#include "buzzer.h"

#include <avr/io.h>
#include <stdint.h>

//initialize frequencies (counts @ 0.8325Mhz)
// x=6 y=9
/*
#define Eh 2378 // 350Hz
#define Cs 2822 // 295Hz
#define A 1778 // 468Hz
#define El 4757 // 175Hz
*/
#define Eh 9523 >> 1
#define Cs 11337 >> 1
#define A 7122 >> 1
#define El 19047 >> 1
#define MAX_OCTAVE 2
#define MIN_OCTAVE -2

extern volatile uint8_t selected_tone;
uint16_t TONE0 = Eh << 2;
uint16_t TONE1 = Cs << 2;
uint16_t TONE2 = A << 2;
uint16_t TONE3 = El << 2;

// -----------------------------  BUZZER  -----------------------------

volatile int8_t octave = 0;

void increase_octave(void)
{
    if (octave < MAX_OCTAVE)
    {
        octave++;
    }
}

void decrease_octave(void)
{
    if (octave > MIN_OCTAVE)
    {
        octave--;

    }
}

void play_tone(const uint8_t tone)
{
    // Frequencies: 440 Hz, 659 Hz
    // Periods at octave -3: 60606, 40450
    uint16_t periods[4] = {TONE0, TONE1, TONE2, TONE3};
    if ((tone >= 0) && (tone < 4))
    {
    uint16_t period = periods[tone] >> (octave + 2);
    //  >> (octave + 3)
    TCA0.SINGLE.PERBUF = period;
    TCA0.SINGLE.CMP0BUF = period >> 1;
    }
}

void stop_tone(void)
{
    TCA0.SINGLE.CMP0BUF = 0;
}

void reset_tones(void)
{
    uint16_t TONE0 = Eh << 2;
    uint16_t TONE1 = Cs << 2;
    uint16_t TONE2 = A << 2;
    uint16_t TONE3 = El << 2;
    octave = 0;
}