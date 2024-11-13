#include <stdint.h>

extern volatile uint8_t left_byte;
extern volatile uint8_t right_byte;

void display_update(const uint8_t left, const uint8_t right);
