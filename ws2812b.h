/*
 * Copyright 2022 Philip Prohaska and Jonathan Margreiter
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at*
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *See the License for the specific language governing permissions and
 *limitations under the License.
 */

#ifndef WS2812B_H_
#define WS2812B_H_

#include <msp430.h>
#include <stdint.h>

// Change this to the number of LEDs your strip has
#define WS2812B_LED_COUNT 10

// DO NOT TOUCH THESE OR THE CODE WILL BREAK!
#define WS2812B_MASK_6BIT 0x0000C30C30C30C30

#define WS2812B_CLOCK_25MHz

/*
 * This is the data holding container for a single led.
 * A LED-strip is modeled by using an array of this container
 */
typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} ws2812b_led_t;

/**
 * This function initializes the USCI_B0_SPI module, initializes all the LEDs to be black and displays them.
 */
extern void ws2812b_init(void);

/**
 * This function initializes the USCI_B0_SPI module. For the code to work, the MSP needs to run at 25MHz.
 */
extern void ws2812b_initSPI(void);

/**
 * This function sets the color of a single led at index 'p'.
 *
 * @param p The index of the led
 * @param r The red value of the color
 * @param g The green value of the color
 * @param b The blue value of the color
 */
extern void ws2812b_setLEDColor(uint16_t p, uint8_t r, uint8_t g, uint8_t b);

/**
 * This function displays the current led strip.
 * Beware that during execution of this function interrupts will be disabled.
 * This is necessary so the data can be sent in one go without any interruptions
 */
extern void ws2812b_showStrip(void);

/**
 * This function fills the led strip with black color values
 */
extern void ws2812b_clearStrip(void);

/**
 * This function fills the entire led strip with a single color with the color values r, g and b
 *
 * @param r The red value of the color
 * @param g The green value of the color
 * @param b The blue value of the color
 */
extern void ws2812b_fillStrip(uint8_t r, uint8_t g, uint8_t b);

/**
 * This function fills the entire strip with a single color 'color'
 *
 * @param color The color value
 */
extern void ws2812b_fillStripColor(ws2812b_led_t *color);

/**
 * This function initializes the MSP MCLK and SMCLK to 25MHz.
 */
extern void ws2812b_initClockTo25MHz(void);
#endif /* WS2812B_H_ */
