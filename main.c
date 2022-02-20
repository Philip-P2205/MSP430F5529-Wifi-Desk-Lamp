#include <msp430.h> 
#include "driverlib/MSP430F5xx_6xx/driverlib.h"

#define SPICLK 1000000

char transmitData = 0x00;
char receiveData = 0x00;

void main(void)
{
    WDT_A_hold(WDT_A_BASE);


    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN3 + GPIO_PIN4);

    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_setOutputLowOnPin(GPIO_PORT_P3, GPIO_PIN3 + GPIO_PIN4);

    //Initialize Master
    USCI_A_SPI_initMasterParam param = { 0 };
    param.selectClockSource = USCI_A_SPI_CLOCKSOURCE_ACLK;
    param.clockSourceFrequency = UCS_getACLK();
    param.desiredSpiClock = SPICLK;
    param.msbFirst = USCI_A_SPI_MSB_FIRST;
    param.clockPhase = USCI_A_SPI_PHASE_DATA_CHANGED_ONFIRST_CAPTURED_ON_NEXT;
    param.clockPolarity = USCI_A_SPI_CLOCKPOLARITY_INACTIVITY_HIGH;
    bool returnValue = USCI_A_SPI_initMaster(USCI_A0_BASE, &param);
    if (STATUS_FAIL == returnValue)
    {
        return;
    }
    //Enable USCI_A_SPI module
    USCI_A_SPI_enable(USCI_A0_BASE);
    //Enable Receive interrupt
    USCI_A_SPI_enableInterrupt(USCI_A0_BASE, UCRXIE);
    //Configure port pins to reset slave
    // Wait for slave to initialize
    __delay_cycles(100);
    // Initialize data values
    transmitData = 0x00;
    // USCI_A0 TX buffer ready?
    while (!USCI_A_SPI_getInterruptStatus(USCI_A0_BASE, UCTXIFG))
        ;
    //Transmit Data to slave
    USCI_A_SPI_transmitData(USCI_A0_BASE, transmitData);
    // CPU off, enable interrupts
    __bis_SR_register(LPM0_bits + GIE);
}
//******************************************************************************
//
// This is the USCI_A0 interrupt vector service routine.
//
//******************************************************************************
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
    switch (__even_in_range(UCA0IV, 4))
    {
    // Vector 2 - RXIFG
    case 2:
        // USCI_A0 TX buffer ready?
        while (!USCI_A_SPI_getInterruptStatus(USCI_A0_BASE, UCTXIFG))
            ;
        receiveData = USCI_A_SPI_receiveData(USCI_A0_BASE);
        // Increment data
        transmitData++;
        // Send next value
        USCI_A_SPI_transmitData(USCI_A0_BASE, transmitData);
        //Delay between transmissions for slave to process information
        __delay_cycles(40);
        break;
    default:
        break;
    }
}
