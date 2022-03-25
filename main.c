#include "main.h"

/**
 * main.c
 */
int main()
{
    WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer

    P2DIR |= BIT2;
    P2SEL |= BIT2;

    while (1)
        ;
    return 0;
}
