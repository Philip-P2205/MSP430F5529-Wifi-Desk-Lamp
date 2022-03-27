/*
 * Copyright 2022 Philip Prohaska and Jonathan Margreiter
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required b-y applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *See the License for the specific language governing permissions and
 *limitations under the License.
 */

#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>
#include "mesp-ws2812b.h"

//ws2812b_led_t color = { 0, 0, 0 };

/*
 * TODO: write function to handle the incoming data.
 * mesp module is finished
 */

mespWS2812B_color_t color = { .r = 255, .g = 0, .g = 0 };
int main()
{
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

//  ws2812b_initClockTo25MHz(); // set clock to 25MHz. This is necessary to get the timing right for the leds.

//  // For debugging only
//  P2DIR = BIT0 + BIT2;
//  P2SEL = BIT2; // output smclk at P2.2
//
//  ws2812b_init(); // Initialize USCI_B0_SPI module and clear the leds to be black.
//                  //    __delay_cycles(25000000); // 1 second delay at 25MHz

    mespWS2812B_init();
//    mespWS2812B_single(&color);
    mespWS2812B_enable();
    while (1)
    {
        mespWS2812B_loop();
    }
    return 0;
}

