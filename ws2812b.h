/*
 * ws2812b.h
 *
 *  Created on: 25.10.2021
 *      Author: Philip Prohaska
 */

#ifndef WS2812B_H_
#define WS2812B_H_

#include <stdint.h>
#include <msp430.h>

#define WS2812B_LED_COUNT 10

// DO NOT TOUCH !
#define WS2812B_MASK_3BIT 0x924924
#define WS2812B_MAX_SEND_BUFFER_SIZE 1 /*(WS2812B_LED_COUNT * 9)*/
#define SMCLK_kHz 8192
#define SMCLK_RATIO 256
#define SPICLK 2300000

typedef struct
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} ws2812b_led_t;

/**
 * This function initializes everything needed for the library to work.
 */
extern void ws2812b_init(void);
/**
 * This function initializes USCI_B_SPI. This module is used for transmitting the data to the leds.
 *
 * The pinout for USCI_B_SPI is the following:
 *  - MOSI: P3.0
 *  - MISO: P3.1
 *  - CLK:   P3.2
 *
 * However for this library only the MOSI pin is needed.
 */
extern bool ws2812b_init_spi(void);

/**
 * This function encodes an 8-bit (1 byte) value into 32 bits that can be sent to the leds.
 *
 * @param byte The data that should be encoded.
 */
extern uint32_t ws2812b_encode_byte_3bit(uint8_t byte);
/**
 * This function encodes a LEDs red, green and blue values into a 32-bit encoded value that is saved in output.
 *
 * @param led A pointer to the led that should be encoded.
 * @param output The output field for the encoded color values. Must have at least 3 free spots.
 * @param index The staring index for the output values. Including output[index] there must be at least 3 spaces available.
 */
extern void ws2812b_encode_led_3bit(const ws2812b_led_t *led, uint32_t output[],
                                    uint16_t index);


extern void ws2812b_waitForBuffer(void);

/**
 * This function transmits a single leds color values.
 *
 * @param led A pointer to the LED to transmit the color values from.
 */
extern void ws2812b_transmitLED(const ws2812b_led_t *led);

/**
 * This function transmits a single 32-bit encoded color value.
 *
 * @param color The 32-bit encoded color value to transmit.
 *
 */
extern void ws2812b_transmitColor(uint32_t color);

/**
 * This function transmits a single byte.
 *
 * @param data The data to transmit.
 */
extern void ws2812b_transmitByte(uint8_t byte);

#endif /* WS2812B_H_ */
/*
 * Data Signal:
 *  - GRBA-Encoding
 *  - MSB-First
 *  - Bit time ~0.4us
 *  - Frequency: 2,5MHz
 */

/*
 ******************
 * 3-bit encoding *
 ******************
 *
 * 8 bits from LED color stream encoded in 3 byte for transport stream (SPI TX)
 * or: 1 bit from LED color stream encoded in 3 bit for transport stream
 *
 *                      _
 * ZERO: 100     |__
 *                    __
 * ONE :  110      |_
 *
 * the bit in the middle defines the value
 *
 * data stream: 0x23                 0  0  1  0  0  0  1  1
 * encoding:                        1x01x01x01x01x01x01x01x0
 * transport stream:                100100110100100100110110
 *
 * initial mask: 0x92 0x49 0x24     100100100100100100100100
 *
 * sourcebit :                       7  6  5  4  3  2  1  0
 * encoding  :                      1x01x01x01x01x01x01x01x0
 * targetbit :                       6  3  0  5  2  7  4  1
 * targetbyte:                      |   0   |   1   |   2   |
 *
 * sourcebit -> (targetbit,targetbyte)
 * 7->(6,0)
 * 6->(3,0)
 * 5->(0,0)
 * 4->(5,1)
 * 3->(2,1)
 * 2->(7,2)
 * 1->(4,2)
 * 0->(1,2)
 */

/*
 ******************
 * 4-bit encoding *
 ******************
 *
 * 2 bits from LED color stream encoded in 1 byte for transport stream (SPI TX)
 * or: 1 bit from LED color stream encoded in 4 bit for transport stream
 *
 *                      _
 * ZERO: 1000 = 0x8      |___
 *                      ___
 * ONE : 1110 = 0xE        |_
 *
 * SPI Clock around 3.2MHz (e.g. 6.7MHz/2 = 3.35MHz)
 *
 */
