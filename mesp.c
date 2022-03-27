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
#include "mesp.h"

static mesp_data_frame_t frame;

static uint8_t data[0xFF];

static mesp_callback_fct_t callback_fct;

static uint8_t receive_index = 0;
static uint8_t mesp_status = 0;

void mesp_loop(void)
{
    if (mesp_status == MESP_STATUS_FINISHED)
    {
        callback_fct(&frame);
        receive_index = 0;
        mesp_status = MESP_STATUS_START;
    }
}

void mesp_init(mesp_callback_fct_t callback)
{
    callback_fct = callback; // Save callback function for later use
    frame.data = data;
    mesp_initSPI(); // init USCI_A0_SPI module
    mesp_status = MESP_STATUS_START;
}

void mesp_initSPI(void)
{
    P3SEL |= BIT3 + BIT4; // MISO, MOSI
    P2SEL |= BIT7;  // CLK
    UCA0CTL1 |= UCSWRST;
    UCA0CTL0 &= ~(UCMST + UCCKPH + UCCKPL); // ensure slave mode is active, clock inactive when low
    UCA0CTL0 |= UCSYNC + UCMSB; // 3-pin, 8-bit, MSB-first
    UCA0CTL1 &= ~UCSWRST;
    UCA0IE |= UCRXIE;
}

inline void mesp_disableIncoming(void)
{
    P1OUT &= ~BIT6; // ESP will not send data with RDY pin low, set it to low to disable communication
}

inline void mesp_enableIncoming(void)
{
    P1OUT |= BIT6; // ESP will not send data with RDY pin low, set i to high to enable communimaion
}

#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch (__even_in_range(UCA0IV, 4))
    {
    case 0: // Vector 0 - no interrupt
        break;
    case 2: // Vector 2 - RXIFG
        // check if all the data has been received
        if (mesp_status == MESP_STATUS_DATA && receive_index >= frame.length)
            mesp_status = MESP_STATUS_END; // Change the status to receive end code

        switch (mesp_status)
        {
        case MESP_STATUS_START:
            // Has the start code been sent?
            if (UCA0RXBUF == MESP_START_CODE)
                mesp_status = MESP_STATUS_CMD; // Change the status to receive command
            break;

        case MESP_STATUS_CMD:
            frame.cmd = UCA0RXBUF;       // save the command
            mesp_status = MESP_STATUS_LENGTH; // Change the status to receive data length
            break;

        case MESP_STATUS_LENGTH:
            frame.length = UCA0RXBUF;  // save the data length
            receive_index = 0;                 // initialize the receive index
            mesp_status = MESP_STATUS_DATA; // Change the status to receive data
            break;

        case MESP_STATUS_DATA:
            frame.data[receive_index] = UCA0RXBUF; // save the received data to the current index
            receive_index++;
            // status changed before switch statement
            break;

        case MESP_STATUS_END:
            // Has the end code been sent?
            if (UCA0RXBUF == MESP_END_CODE)
            {
                mesp_status = MESP_STATUS_FINISHED; // Change the status to finished

            }
            break;
        default:
            break;
        }
        break;
    case 4: // Vector 4  - TXIFG
        break;
    default:
        break;
    }
}
