#include "initialisation.h"
#include <avr/io.h>

void button_init(void)
{
    // Enable pull-up resistors for PBs
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm; // S1
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm; // S2
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm; // S3
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm; // S4
}

void uart_init(void)
{
    // 9600 baud
    USART0.BAUD = 1389;

    // Enable receive complete interrupt
    USART0.CTRLA = USART_RXCIE_bm;
    USART0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
}

void pwm_init(void)
{
    /** EX: 12.2

    TASK: Configure TCA0 for waveform output on PB0. The buzzer should
    be silent initially.
    */
    PORTB.OUTSET = PIN0_bm;  //TURN BUZZER OFF 
    PORTB.DIRSET = PIN0_bm | PIN2_bm;  //SET PB0 as output

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV2_gc;   // prescaler/1
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0_bm; 

    TCA0.SINGLE.PER =  0; 
    TCA0.SINGLE.CMP0 = 0;  //initialise to 0% duty cycle for buzzer

    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
    /** CODE: Write your code for Ex 12.2 within this function. */

}

void spi_init(void)
{
    // Configure pins
    // DISP LATCH (PA1)
    PORTA.OUTCLR = PIN1_bm;
    PORTA.DIRSET = PIN1_bm;
    // SPI CLK (PC0), SPI MOSI (PC2)
    PORTC.DIRSET = PIN0_bm | PIN2_bm;

    PORTMUX.SPIROUTEA = PORTMUX_SPI0_ALT1_gc;   // Select alternate pin configuration for SPI
    SPI0.CTRLB = SPI_SSD_bm;                    // Disable client select line
    SPI0.INTCTRL = SPI_IE_bm;                   // Enable interrupt
    SPI0.CTRLA = SPI_MASTER_bm | SPI_ENABLE_bm; // Enable SPI as master
}

void timer_init(void)
{
    // Interrupt every 5 ms
    TCB1.CCMP = 16667;
    TCB1.INTCTRL = TCB_CAPT_bm;
    TCB1.CTRLA = TCB_ENABLE_bm;
}

void tcb0_init(void)
{
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // Configure TCB1 in periodic interrupt mode
    TCB0.CCMP = 3333;                // Set interval for 1 ms (3333 clocks @ 3.333 MHz)
    TCB0.INTCTRL = TCB_CAPT_bm;      // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;      // Enable
}

void adc_init(void) 
{
    ADC0.CTRLA = ADC_ENABLE_bm;      // Enable ADC
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;  // /2 clock prescaler 

    // Need 4 CLK_PER cycles @ 3.3 MHz for 1us, select VDD as ref
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc;
    ADC0.CTRLE = 64;                               // Sample duration of 64
    //ADC0.CTRLF = ADC_FREERUN_bm;  // Free running, left adjust
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;              // Select AIN2 (potentiomenter R1)
    ADC0.INTFLAGS = ADC_RESRDY_bm;

    // 8-bit resolution, single-ended
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc;
} 
