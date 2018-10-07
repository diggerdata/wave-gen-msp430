#include <msp430.h>
#include "peripherals.h"

unsigned char counter;              // Current location in wave array
unsigned char wave[32] = {          // Wave array, preset to values of sine
    128, 140, 152, 164, 173, 181, 187, 191,
    192, 191, 187, 181, 173, 164, 152, 140,
    128, 116, 104, 92, 83, 75, 69, 65,
    64, 65, 69, 75, 83, 92, 104, 116 };

unsigned int i;                     // Used for 'for' loops.

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;                   // Stop WDT

    P1DIR |= BIT0;                              // P1.0 output
    counter = 0;                                // Reset counter

    // Initialize Timer
    TA2CCTL0 = CCIE;                               // CCR0 interrupt enabled
    TA2CCTL1 = CCIE;                               // CCR1 interrupt enabled
    TA2CCR0 = 256;                                 // Set PWM period to 256 clock ticks
    TA2CCR1 = wave[counter];                       // Set first duty cycle value
    TA2CTL = TASSEL_2 + MC_1 + TAIE + TACLR;     // SMCLK, upmode, enable interrupt, clear TA1R

    _BIS_SR(LPM0_bits + GIE);                   // Enter LPM0 w/ interrupt
}

/**
 * TimerA0 interrupt service routine
 **/
#pragma vector=TIMERA0_VECTOR
__interrupt void TIMERA0_ISR(void) {
    P1OUT |= BIT0;              // Set P1.0

    TA2CCR1 = wave[counter];       // Set next duty cycle value
    counter += 1;               // Add Offset to CCR0
    if ( counter == 32) {
        counter = 0;            // Reset counter
    }
}

/**
 * TimerA1 Interrupt Vector (TAIV) handler
 **/
#pragma vector=TIMERA1_VECTOR
__interrupt void TIMERA1_ISR(void) {
    switch( TA2IV ) {
        case  2:                // CCR1 interrupt
            P1OUT &= ~BIT0;     // Clear P1.0 to determine duty cycle.
            break;
        default:
            break;
    }
}
