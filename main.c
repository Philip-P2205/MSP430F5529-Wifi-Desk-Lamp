#include "driverlib.h"
#include "ws2812b.h"

//*****************************************************************************
//! This example shows how SPI master talks to SPI slave using 3-wire mode.
//! Incrementing data is sent by the master starting at 0x01. Received data is
//! expected to be same as the previous transmission.  USCI RX ISR is used to
//! handle communication with the CPU, normally in LPM0. If high, P1.0 indicates
//! valid data reception.  Because all execution after LPM0 is in ISRs,
//! initialization waits for DCO to stabilize against ACLK.
//! ACLK = ~32.768kHz, MCLK = SMCLK = DCO ~ 1048kHz.  BRCLK = SMCLK/2
//!
//! Use with SPI Slave Data Echo code example.  If slave is in debug mode, P1.1
//! slave reset signal conflicts with slave's JTAG; to work around, use IAR's
//! "Release JTAG on Go" on slave device.  If breakpoints are set in
//! slave RX ISR, master must stopped also to avoid overrunning slave
//! RXBUF.
//!
//!                  MSP430F5529, Master
//!                 -----------------
//!            /|\ |                 |
//!             |  |                 |
//!          ---+->|RST              |
//!                |                 |
//!                |             P3.0|-> Data Out (UCB0SIMO)
//!                |                 |
//!                |             P3.1|<- Data In (UCB0SOMI)
//!                |                 |
//!                |             P3.2|-> Serial Clock Out (UCB0CLK)
//!
//!
//! This example uses the following peripherals and I/O signals.  You must
//! review these and change as needed for your own board:
//! - SPI peripheral
//! - GPIO Port peripheral (for SPI pins)
//! - UCB0SIMO
//! - UCB0SOMI
//! - UCB0CLK
//!
//! This example uses the following interrupt handlers.  To use this example
//! in your own application you must add these interrupt handlers to your
//! vector table.
//! - USCI_AB_VECTOR
//!
//*****************************************************************************

//*****************************************************************************
//
//Specify desired frequency of SPI communication
//
//*****************************************************************************

//uint8_t transmitData[] = { 0xAA, 0xF0, 0xFF, 0x0F, 0xAA, 0x00 };

#define LED_COUNT 10

uint32_t colors[4 * LED_COUNT];
ws2812b_led_t leds[LED_COUNT] = { //

        { .red = 255, //
          .green = 25, //
          .blue = 200 //
        },//

        { .red = 255, //
          .green = 0, //
          .blue = 0 //
        },//

        { .red = 0, //
          .green = 255, //
          .blue = 0 //
        },//

        { .red = 0, //
          .green = 0, //
          .blue = 255 //
        },//

        { .red = 255, //
          .green = 255, //
          .blue = 0 //
        },//

        { .red = 255, //
          .green = 0, //
          .blue = 255 //
        },//

        { .red = 0, //
          .green = 255, //
          .blue = 255 //
        },//

        { .red = 255, //
          .green = 255, //
          .blue = 255 //
        },//

        { .red = 0, //
          .green = 0, //
          .blue = 0 //
        },//

        { .red = 0, //
          .green = 0, //
          .blue = 0 //
        }//
        };

void initClockTo16MHz();
uint16_t setVCodeUp(uint8_t level);
bool increaseVCoreToLevel2();

void main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

//    increaseVCoreToLevel2();
//    initClockTo16MHz();

    ws2812b_init();

    while (1)
    {
        ws2812b_waitForBuffer();
//        uint8_t i;
//        for (i = 0; i < LED_COUNT; i++)
//            ws2812b_transmitLED(leds + i);
        ws2812b_transmitLED(&(leds[0]));
    }

    // For debugging
    //CPU off, enable interrupts
//    __bis_SR_register(LPM0_bits + GIE);
}

void initClockTo16MHz()
{
    UCSCTL3 |= SELREF_2;                      // Set DCO FLL reference = REFO
    UCSCTL4 |= SELA_2;                        // Set ACLK = REFO
    __bis_SR_register(SCG0);                  // Disable the FLL control loop
    UCSCTL0 = 0x0000;                         // Set lowest possible DCOx, MODx
    UCSCTL1 = DCORSEL_5;                     // Select DCO range 16MHz operation
    UCSCTL2 = FLLD_0 + 487;                   // Set DCO Multiplier for 16MHz
                                              // (N + 1) * FLLRef = Fdco
                                              // (487 + 1) * 32768 = 16MHz
                                              // Set FLL Div = fDCOCLK
    __bic_SR_register(SCG0);                  // Enable the FLL control loop

    // Worst-case settling time for the DCO when the DCO range bits have been
    // changed is n x 32 x 32 x f_MCLK / f_FLL_reference. See UCS chapter in 5xx
    // UG for optimization.
    // 32 x 32 x 16 MHz / 32,768 Hz = 500000 = MCLK cycles for DCO to settle
    __delay_cycles(500000);    //
    // Loop until XT1,XT2 & DCO fault flag is cleared
    do
    {
        UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG); // Clear XT2,XT1,DCO fault flags
        SFRIFG1 &= ~OFIFG;                          // Clear fault flags
    }
    while (SFRIFG1 & OFIFG);                       // Test oscillator fault flag
}

uint16_t setVCoreUp(uint8_t level)
{
    uint32_t PMMRIE_backup, SVSMHCTL_backup, SVSMLCTL_backup;

    //The code flow for increasing the Vcore has been altered to work around
    //the erratum FLASH37.
    //Please refer to the Errata sheet to know if a specific device is affected
    //DO NOT ALTER THIS FUNCTION

    //Open PMM registers for write access
    PMMCTL0_H = 0xA5;

    //Disable dedicated Interrupts
    //Backup all registers
    PMMRIE_backup = PMMRIE;
    PMMRIE &= ~(SVMHVLRPE | SVSHPE | SVMLVLRPE |
    SVSLPE | SVMHVLRIE | SVMHIE |
    SVSMHDLYIE | SVMLVLRIE | SVMLIE |
    SVSMLDLYIE);
    SVSMHCTL_backup = SVSMHCTL;
    SVSMLCTL_backup = SVSMLCTL;

    //Clear flags
    PMMIFG = 0;

    //Set SVM highside to new level and check if a VCore increase is possible
    SVSMHCTL = SVMHE | SVSHE | (SVSMHRRL0 * level);

    //Wait until SVM highside is settled
    while ((PMMIFG & SVSMHDLYIFG) == 0)
    {
        ;
    }

    //Clear flag
    PMMIFG &= ~SVSMHDLYIFG;

    //Check if a VCore increase is possible
    if ((PMMIFG & SVMHIFG) == SVMHIFG)
    {
        //-> Vcc is too low for a Vcore increase
        //recover the previous settings
        PMMIFG &= ~SVSMHDLYIFG;
        SVSMHCTL = SVSMHCTL_backup;

        //Wait until SVM highside is settled
        while ((PMMIFG & SVSMHDLYIFG) == 0)
        {
            ;
        }

        //Clear all Flags
        PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG |
        SVMLVLRIFG | SVMLIFG |
        SVSMLDLYIFG);

        //Restore PMM interrupt enable register
        PMMRIE = PMMRIE_backup;
        //Lock PMM registers for write access
        PMMCTL0_H = 0x00;
        //return: voltage not set
        return false;
    }

    //Set also SVS highside to new level
    //Vcc is high enough for a Vcore increase
    SVSMHCTL |= (SVSHRVL0 * level);

    //Wait until SVM highside is settled
    while ((PMMIFG & SVSMHDLYIFG) == 0)
    {
        ;
    }

    //Clear flag
    PMMIFG &= ~SVSMHDLYIFG;

    //Set VCore to new level
    PMMCTL0_L = PMMCOREV0 * level;

    //Set SVM, SVS low side to new level
    SVSMLCTL = SVMLE | (SVSMLRRL0 * level) |
    SVSLE | (SVSLRVL0 * level);

    //Wait until SVM, SVS low side is settled
    while ((PMMIFG & SVSMLDLYIFG) == 0)
    {
        ;
    }

    //Clear flag
    PMMIFG &= ~SVSMLDLYIFG;
    //SVS, SVM core and high side are now set to protect for the new core level

    //Restore Low side settings
    //Clear all other bits _except_ level settings
    SVSMLCTL &= (SVSLRVL0 + SVSLRVL1 + SVSMLRRL0 +
    SVSMLRRL1 + SVSMLRRL2);

    //Clear level settings in the backup register,keep all other bits
    SVSMLCTL_backup &=
            ~(SVSLRVL0 + SVSLRVL1 + SVSMLRRL0 + SVSMLRRL1 + SVSMLRRL2);

    //Restore low-side SVS monitor settings
    SVSMLCTL |= SVSMLCTL_backup;

    //Restore High side settings
    //Clear all other bits except level settings
    SVSMHCTL &= (SVSHRVL0 + SVSHRVL1 +
    SVSMHRRL0 + SVSMHRRL1 +
    SVSMHRRL2);

    //Clear level settings in the backup register,keep all other bits
    SVSMHCTL_backup &=
            ~(SVSHRVL0 + SVSHRVL1 + SVSMHRRL0 + SVSMHRRL1 + SVSMHRRL2);

    //Restore backup
    SVSMHCTL |= SVSMHCTL_backup;

    //Wait until high side, low side settled
    while (((PMMIFG & SVSMLDLYIFG) == 0) && ((PMMIFG & SVSMHDLYIFG) == 0))
    {
        ;
    }

    //Clear all Flags
    PMMIFG &= ~(SVMHVLRIFG | SVMHIFG | SVSMHDLYIFG |
    SVMLVLRIFG | SVMLIFG | SVSMLDLYIFG);

    //Restore PMM interrupt enable register
    PMMRIE = PMMRIE_backup;

    //Lock PMM registers for write access
    PMMCTL0_H = 0x00;

    return true;
}

bool increaseVCoreToLevel2()
{
    uint8_t level = 2;
    uint8_t actlevel;
    bool status = true;

    //Set Mask for Max. level
    level &= PMMCOREV_3;

    //Get actual VCore
    actlevel = PMMCTL0 & PMMCOREV_3;

    //step by step increase or decrease
    while ((level != actlevel) && (status == true))
    {
        if (level > actlevel)
        {
            status = setVCoreUp(++actlevel);
        }
    }

    return (status);
}
