/*
 * Copyright 2022 Philip Prohaska and Jonathan Margreiter
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *See the License for the specific language governing permissions and
 *limitations under the License.
 */

#include "ws2812b.h"

/**
 * The led strip model.
 * This array will save all of the color data of the leds strip.
 */
static ws2812b_led_t leds[WS2812B_LED_COUNT] = { { 0, 0, 0 } };

// static functions not to be exposed to the user:

/**
 * This function encodes the given byte using 6 bit encoding.
 *
 * @param byte The byte to encode
 *
 * @return The byte encoded using 6 bit encoding. Only the first 48-bits are used.
 */
static uint64_t ws2812b_encode_byte_6bit(uint8_t byte);

/**
 * This function transmits one byte using the USCI_B0_SPI module.
 *
 * @param byte The byte to transmit via SPI
 */
static inline void ws2812b_transmitByte(uint8_t byte);

/**
 * This function increases the vcore to the specified level.
 * Note that is is recommended to increase the vcore one step at a time.
 *
 * @param level The level the vcore should be set to
 */
static void ws2812b_set_vcore(unsigned int level);

void ws2812b_init()
{
    ws2812b_initClockTo25MHz(); // set clock to 25MHz. This is necessary to get the timing right for the leds.
    ws2812b_initSPI();          // Initialize the USCI_B0_SPI module
    ws2812b_clearStrip();
    ws2812b_showStrip();
}

void ws2812b_initSPI()
{
    UCB0CTL1 |= UCSWRST; // Put USCI state machin in reset

    P3SEL |= BIT0;                         // configure output pin as SPI output
                                           //    P3SEL2 |= OUTPUT_PIN;
    UCB0CTL0 |= UCCKPH + UCMSB + UCMST + UCSYNC; // 3-pin, MSB, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;                        // SMCLK source (16 MHz)
    UCB0BR0 = 5;                // 25 MHz / 5 = .2 us per bit (.1875 us per bit)
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST; // Initialize USCI state machine
}

void ws2812b_setLEDColor(uint16_t p, uint8_t r, uint8_t g, uint8_t b)
{
    if (p < WS2812B_LED_COUNT) // protection against memory overflow
    {
        leds[p].green = g;
        leds[p].red = r;
        leds[p].blue = b;
    }
}

void ws2812b_showStrip(void)
{
    uint64_t colors[3][WS2812B_LED_COUNT] = { { 0 } }; // Array for all the encoded led data

    uint64_t *green = colors[0];
    uint64_t *red = colors[1];
    uint64_t *blue = colors[2];

    uint8_t *g, *r, *b; // 8-bit color chunks to send with SPI 

    uint16_t i; // looping variable
    for (i = 0; i < WS2812B_LED_COUNT; i++)
    {
        colors[0][i] = ws2812b_encode_byte_6bit(leds[i].green);
        colors[1][i] = ws2812b_encode_byte_6bit(leds[i].red);
        colors[2][i] = ws2812b_encode_byte_6bit(leds[i].blue);
    }

    for (i = 0; i < WS2812B_LED_COUNT; i++)
    {
        g = (uint8_t*) &green[i]; // get the 8 bit chunks for green
        r = (uint8_t*) &red[i];   // get the 8 bit chunks for red
        b = (uint8_t*) &blue[i];  // get the 8 bit chunks for blue
        /*
         * Color encoding (saved in 64-bit variable, upper 2 bytes not used):
         * Encoded color: 11aa0011 bb0011cc 0011dd00 11ee0011 ff0011gg 0011hh00
         * Byte index:              5              4              3               2            1              0
         *
         */
        // green
        ws2812b_transmitByte(*(g + 5));
        ws2812b_transmitByte(*(g + 4));
        ws2812b_transmitByte(*(g + 3));
        ws2812b_transmitByte(*(g + 2));
        ws2812b_transmitByte(*(g + 1));
        ws2812b_transmitByte(*(g + 0));

        // red
        ws2812b_transmitByte(*(r + 5));
        ws2812b_transmitByte(*(r + 4));
        ws2812b_transmitByte(*(r + 3));
        ws2812b_transmitByte(*(r + 2));
        ws2812b_transmitByte(*(r + 1));
        ws2812b_transmitByte(*(r + 0));

        // blue
        ws2812b_transmitByte(*(b + 5));
        ws2812b_transmitByte(*(b + 4));
        ws2812b_transmitByte(*(b + 3));
        ws2812b_transmitByte(*(b + 2));
        ws2812b_transmitByte(*(b + 1));
        ws2812b_transmitByte(*(b + 0));
    }
}

void ws2812b_clearStrip(void)
{
    ws2812b_fillStrip(0, 0, 0);
}

void ws2812b_fillStrip(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t i;
    for (i = 0; i < WS2812B_LED_COUNT; i++)
        ws2812b_setLEDColor(i, r, g, b);
}

void ws2812b_fillStripColor(ws2812b_led_t *color)
{
    ws2812b_fillStrip(color->red, color->green, color->blue);
}

void ws2812b_initClockTo25MHz(void)
{
    // clock config for MSP430F5529

    P5SEL |= BIT4 + BIT5;          // port select XT1
    UCSCTL6 &= ~(XT1OFF + XCAP_3); // XT1 On, clear internal load caps
    UCSCTL6 |= XCAP_0; // internal load caps 2pF, 6pF, 9pF or 12pF could be selected, XCAP_0 => 2pF

    do // loop until XT1 fault flag is cleared
    {
        UCSCTL7 &= ~XT1LFOFFG;     // clear XT1 fault flags
    }
    while (UCSCTL7 & XT1LFOFFG); // test XT1 fault flag

    UCSCTL6 &= ~(XT1DRIVE_3); // reduce XT1driver strength to the lowest level

    __bis_SR_register(SCG0); // disable the FLL control loop

#ifdef WS2812B_CLOCK_25MHz
    ws2812b_set_vcore(0x01); // Increase Vcore setting to level 3...
    ws2812b_set_vcore(0x02); // ...to support fsystem=25MHz
    ws2812b_set_vcore(0x03); // NOTE: Change core voltage one level at a time

    UCSCTL0 = 0x0000;       // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_7;    // select DCO range 50MHz
    UCSCTL2 = FLLD_0 + 762; // 32768Hz * (762 + 1) = 25MHz
#endif

#ifdef WS2812B_CLOCK_16MHz
    ws2812b_set_vcore(0x01); // Increase Vcore setting to level 3...
    ws2812b_set_vcore(0x02); // ...to support fsystem=16MHz
    ws2812b_set_vcore(0x03); // NOTE: Change core voltage one level at a time

    UCSCTL0 = 0x0000;       // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_5;    // select DCO range 16MHz
    UCSCTL2 = FLLD_0 + 487; // 32768Hz * (487 + 1) = 16MHz
#endif

#ifdef WS2812B_CLOCK_8MHz
    UCSCTL1 = DCORSEL_3;    // select DCO range 1MHz to 10MHz
    UCSCTL2 = FLLD_1 + 243; // 32768Hz * (243 + 1) = 8MHz
#endif

    UCSCTL3 = SELREF_0 + FLLREFDIV_0; // Set DCO FLL reference = XT1, FLL reference divider 1
    UCSCTL4 = SELA_0 + SELS_3 + SELM_3; // select clock sources for ACLK, MCLK and SMCLK
    UCSCTL5 = DIVPA_0 + DIVA_0 + DIVS_0 + DIVM_0; // select clock dividers

    __bic_SR_register(SCG0); // enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.

#ifdef WS2812B_CLOCK_8MHz
    __delay_cycles(250000); // 32 x 32 x 8 MHz / 32768 Hz = 250000 = MCLK cycles for DCO to settle
#endif

#ifdef WS2812B_CLOCK_16MHz
    __delay_cycles(500000); // 32 x 32 x 16 MHz / 32,768 Hz = 500000 = MCLK cycles for DCO to settle
#endif

#ifdef WS2812B_CLOCK_25MHz
    __delay_cycles(782000); // 32 x 32 x 25 MHz / 32768 Hz = 782000 = MCLK cycles for DCO to settle
#endif
}

static uint64_t ws2812b_encode_byte_6bit(uint8_t byte)
{
    uint64_t encodedData = WS2812B_MASK_6BIT; // 48bits required!

    // encoding: abcdefgh --> 11aa00 11bb00 11cc00 11dd00 11ee00 11ff00 11gg00 11hh00

    encodedData |= ((uint64_t) (byte & 0x80)) << 0x25; // BIT7-1
    encodedData |= ((uint64_t) (byte & 0x80)) << 0x26; // BIT7-2
    encodedData |= ((uint64_t) (byte & 0x40)) << 0x20; // BIT6-1
    encodedData |= ((uint64_t) (byte & 0x40)) << 0x21; // BIT6-2
    encodedData |= ((uint64_t) (byte & 0x20)) << 0x1B; // BIT5-1
    encodedData |= ((uint64_t) (byte & 0x20)) << 0x1C; // BIT5-2
    encodedData |= ((uint64_t) (byte & 0x10)) << 0x16; // BIT4-1
    encodedData |= ((uint64_t) (byte & 0x10)) << 0x17; // BIT4-2
    encodedData |= ((uint64_t) (byte & 0x08)) << 0x11; // BIT3-1
    encodedData |= ((uint64_t) (byte & 0x08)) << 0x12; // BIT3-2
    encodedData |= ((uint64_t) (byte & 0x04)) << 0x0C; // BIT2-1
    encodedData |= ((uint64_t) (byte & 0x04)) << 0x0D; // BIT2-2
    encodedData |= ((uint64_t) (byte & 0x02)) << 0x07; // BIT1-1
    encodedData |= ((uint64_t) (byte & 0x02)) << 0x08; // BIT1-2
    encodedData |= ((uint64_t) (byte & 0x01)) << 0x02; // BIT0-1
    encodedData |= ((uint64_t) (byte & 0x01)) << 0x03; // BIT0-2

    return encodedData;
}

static inline void ws2812b_transmitByte(uint8_t byte)
{
    // USCI_B0 TX buffer ready?
    while (!(UCB0IFG & UCTXIFG))
        ;
    // Transmit Data to slave
    UCB0TXBUF = byte;
}

static void ws2812b_set_vcore(unsigned int level)
{
    PMMCTL0_H = PMMPW_H; // Open PMM registers for write

    SVSMHCTL = SVSHE + SVSHRVL0 * level + SVMHE + SVSMHRRL0 * level; // Set SVS/SVM high side new level
    SVSMLCTL = SVSLE + SVMLE + SVSMLRRL0 * level; // Set SVM low side to new level

    while ((PMMIFG & SVSMLDLYIFG) == 0)
        ; // Wait till SVM is settled

    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG); // Clear already set flags
    PMMCTL0_L = PMMCOREV0 * level;     // Set VCore to new level

    if ((PMMIFG & SVMLIFG)) // Wait till new level reached
        while ((PMMIFG & SVMLVLRIFG) == 0)
            ;

    SVSMLCTL = SVSLE + SVSLRVL0 * level + SVMLE + SVSMLRRL0 * level; // Set SVS/SVM low side to new level

    PMMCTL0_H = 0x00; // Lock PMM registers for write access
}
