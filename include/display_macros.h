#define DISP_SEG_B 0b01101111
#define DISP_SEG_C 0b01111011
#define DISP_SEG_E 0b01111110
#define DISP_SEG_F 0b00111111

#define DISP_SEG_SUCCESS 0b00000000
#define DISP_SEG_FAIL 0b01110111

#define DISP_BAR_LEFT (DISP_SEG_E & DISP_SEG_F)
#define DISP_BAR_RIGHT (DISP_SEG_B & DISP_SEG_C)

#define DISP_OFF 0b01111111

#define DISP_LHS (1 << 7)