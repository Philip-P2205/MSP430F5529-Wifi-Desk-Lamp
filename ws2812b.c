/*
 * ws2812b.c
 *
 *  Created on: 25.10.2021
 *      Author: Philip Prohaska
 */

/*
 * Data Signal:
 *  - GRB-Encoding
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
 * the bti in the middle defines the value
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

#include <assert.h>
#include "driverlib.h"
#include "ws2812b.h"

void ws2812b_init(void)
{
    bool returnValue = ws2812b_init_spi();
    assert(returnValue != STATUS_FAIL);

    //Enable SPI module
    USCI_B_SPI_enable(USCI_B0_BASE);

    //Enable Receive interrupt
    USCI_B_SPI_clearInterrupt(USCI_B0_BASE, USCI_B_SPI_RECEIVE_INTERRUPT);
//    USCI_B_SPI_enableInterrupt(USCI_B0_BASE, USCI_B_SPI_RECEIVE_INTERRUPT);
}

bool ws2812b_init_spi(void)
{
    // Set DCO FLL reference = REFO
    UCS_initClockSignal(UCS_FLLREF, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);

    // Set ACLK = REFO
    UCS_initClockSignal(UCS_ACLK, UCS_REFOCLK_SELECT, UCS_CLOCK_DIVIDER_1);

    // Set Ratio and Desired MCLK Frequency and initialize DCO
    UCS_initFLLSettle(SMCLK_kHz, SMCLK_RATIO);

    //P3.0,1,2 option select
    //GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3, GPIO_PIN0 + GPIO_PIN1 + GPIO_PIN2);    // not supported by MSP430Fxxx
    P3SEL |= BIT0 | BIT1 | BIT2;
    //Initialize Master
    USCI_B_SPI_initMasterParam param = { 0 };
    param.selectClockSource = USCI_B_SPI_CLOCKSOURCE_SMCLK;
    param.clockSourceFrequency = UCS_getSMCLK();
    param.desiredSpiClock = SPICLK;
    param.msbFirst = USCI_B_SPI_MSB_FIRST;
    param.clockPhase = USCI_B_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
    param.clockPolarity = USCI_B_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;

    return USCI_B_SPI_initMaster(USCI_B0_BASE, &param);
}

/**
 * This function encodes an 8-bit color value into a 24-bit stream for the leds.
 */
uint32_t ws2812b_encode_byte_3bit(uint8_t byte)
{
    uint32_t encodedData = WS2812B_MASK_3BIT;

    // MSB to LSB:
    encodedData |= ((uint32_t) (byte & 0x80)) << 0x0F; // BIT7
    encodedData |= ((uint32_t) (byte & 0x40)) << 0x0D; // BIT6
    encodedData |= ((uint32_t) (byte & 0x20)) << 0x0B; // BIT5
    encodedData |= ((uint32_t) (byte & 0x10)) << 0x09; // BIT4
    encodedData |= ((uint32_t) (byte & 0x08)) << 0x07; // BIT3
    encodedData |= ((uint32_t) (byte & 0x04)) << 0x05; // BIT2
    encodedData |= ((uint32_t) (byte & 0x02)) << 0x03; // BIT1
    encodedData |= ((uint32_t) (byte & 0x01)) << 0x01; // BIT0

    return encodedData;
}

void ws2812b_encode_led_3bit(const ws2812b_led_t *led, uint32_t output[],
                             uint16_t index)
{
    // Encoding GRB
    output[index + 0] = ws2812b_encode_byte_3bit(led->green);
    output[index + 1] = ws2812b_encode_byte_3bit(led->red);
    output[index + 2] = ws2812b_encode_byte_3bit(led->blue);
}

void ws2812b_transmitLED(const ws2812b_led_t *led)
{
    uint32_t colors[] = { //
            ws2812b_encode_byte_3bit(led->green), //
            ws2812b_encode_byte_3bit(led->red), //
            ws2812b_encode_byte_3bit(led->blue), //
            };
    uint8_t *green = (uint8_t*) &(colors[0]);
    uint8_t *red = (uint8_t*) &(colors[1]);
    uint8_t *blue = (uint8_t*) &(colors[2]);

    // green
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(green + 2));
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(green + 1));
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(green + 0));

    // red
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(red + 2));
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(red + 1));
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(red + 0));

    // blue
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(blue + 2));
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(blue + 1));
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(blue + 0));

//    // alpha
//    USCI_B_SPI_transmitData(USCI_B0_BASE, 0xDB);
//    USCI_B_SPI_transmitData(USCI_B0_BASE, 0x6D);
//    USCI_B_SPI_transmitData(USCI_B0_BASE, 0xB6);
}

void ws2812b_transmitColor(uint32_t color)
{
    uint8_t *bytes = (uint8_t*) &color;
//    ws2812b_transmitData(*(bytes + 2));
    //USCI_A0 TX buffer ready?
    while (!USCI_B_SPI_getInterruptStatus(USCI_B0_BASE,
    USCI_B_SPI_TRANSMIT_INTERRUPT))
        ;
    //Transmit Data to slave
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(bytes + 2));

    //Transmit Data to slave
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(bytes + 1));

    //Transmit Data to slave
    USCI_B_SPI_transmitData(USCI_B0_BASE, *(bytes + 0));

}

void ws2812b_transmitByte(uint8_t byte)
{
    //USCI_A0 TX buffer ready?
    while (!USCI_B_SPI_getInterruptStatus(USCI_B0_BASE,
    USCI_B_SPI_TRANSMIT_INTERRUPT))
        ;
    //Transmit Data to slave
    USCI_B_SPI_transmitData(USCI_B0_BASE, byte);
}

inline void ws2812b_waitForBuffer(void)
{
    //USCI_A0 TX buffer ready?
    while (!USCI_B_SPI_getInterruptStatus(USCI_B0_BASE,
    USCI_B_SPI_TRANSMIT_INTERRUPT))
        ;
}
