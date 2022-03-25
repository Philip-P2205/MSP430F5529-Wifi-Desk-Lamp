/*
 * ws2812b.c
 *
 *  Created on: 25.03.2022
 *      Author: Philip Prohaska
 */
#include "ws2812b.h"

// WS2812 takes GRB format
typedef struct
{
    uint8_t green;
    uint8_t red;
    uint8_t blue;
} LED;

static LED leds[NUM_LEDS] = { { 0, 0, 0 } };

// Initializes everything needed to use this library. This clears the strip.
void initStrip()
{
    UCB0CTL1 |= UCSWRST; // Put USCI state machin in reset

    P3SEL |= OUTPUT_PIN;    // configure output pin as SPI output
//    P3SEL2 |= OUTPUT_PIN;
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; // 3-pin, MSB, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;   // SMCLK source (16 MHz)
    UCB0BR0 = 5;            // 25 MHz / 5 = .2 us per bit (.1875 us per bit)
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;   // Initialize USCI state machine

    clearStrip();
}

// Sets the color of a certain LED (0 indexed)
void setLEDColor(uint16_t p, uint8_t r, uint8_t g, uint8_t b)
{
    leds[p].green = g;
    leds[p].red = r;
    leds[p].blue = b;
}
// Send colors to the strip and show them. Disables interrupts while processing.
void showStrip()
{
    __bic_SR_register(GIE);

    // send GRB color for every LED
    uint16_t i, j;
    for (i = 0; i < NUM_LEDS; i++)
    {
        uint8_t *grb = (uint8_t*) &(leds[i]);

//        send green then red then blue
        for (j = 0; j < 3; j++)
        {
            uint8_t mask = 0x80; // 0b10000000, MSB first

//            check each of the 8 bits and send the corresponding code
            while (!(UCB0IFG & UCTXIFG))
                ; // Wait for transmit to finish
//            bit is 1
            if (grb[j] & mask)
                UCB0TXBUF = HIGH_CODE; // send code for 1
//            bit is 0
            else
                UCB0TXBUF = LOW_CODE; // send code for 0

            mask >>= 1; // check next bit
        }
    }

    // send RESET code for at least 50 us (1250 cylces at 25MHz)
    __delay_cycles(1250);

    __bis_SR_register(GIE);
}

// clear the color of all LEDs (make them black)
inline void clearStrip()
{
    fillStrip(0, 0, 0);
}

// Fills the strip with the specified color and displays it.
void fillStrip(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t i;
    for (i = 0; i < NUM_LEDS; i++)
        setLEDColor(i, r, g, b);
    showStrip();
}
