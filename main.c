/**
 * main.c
 */

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include "ws2812b.h"

ws2812b_led_t color = { 0, 0, 0 };

int main()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    ws2812b_initClockTo25MHz(); // set clock to 25MHz. This is necessary to get the timing right for the leds.

    // For debugging only
    P2DIR = BIT0 + BIT2;
    P2SEL = BIT2; //output smclk at P2.2

    ws2812b_init(); // Initialize USCI_B0_SPI module and clear the leds to be black.
//    __delay_cycles(25000000); // 1 second delay at 25MHz
    while (1)
    {
        ws2812b_fillStripColor(&color); // fill the strip with the color
        ws2812b_showStrip(); // display the color in the strip
    }
    return 0;
}
