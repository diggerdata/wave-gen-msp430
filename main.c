#include <msp430.h>
#include "peripherals.h"

typedef enum {DC, SIN, SQUARE, TRIANGLE} waveType;

void ConvertADC(void);
void DACSetValue(unsigned int dac_code);

unsigned char counter;              // Current location in sinWave array
unsigned char sinWave[32] = {          // sinWave array, preset to values of sine
    128, 140, 152, 164, 173, 181, 187, 191,
    192, 191, 187, 181, 173, 164, 152, 140,
    128, 116, 104, 92, 83, 75, 69, 65,
    64, 65, 69, 75, 83, 92, 104, 116 };


unsigned int i;                     // Used for 'for' loops.
int squareToggle = 1;

void ConvertADC() {
    ADC12CTL0 &= ~ADC12SC; // clear the start bit
    ADC12CTL0 |= ADC12SC; // Sampling and conversion start
}

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;                   // Stop WDT

    waveType = DC; // set waveType to a default

    P1DIR |= BIT0;                              // P1.0 output
    counter = 0;                                // Reset counter

    REFCTL0 &= ~REFMSTR;    // Reset REFMSTR to hand over control of
    // internal reference voltages to
    // ADC12_A control registers

    // internal reference voltage of 1.5 for pot
    ADC12CTL0 = ADC12ON | ADC12REFON | ADC12SHT0_9 | ADC12MSC;
    // use mem1 to hold pot values
    ADC12MCTL1 = ADC12SREF_0 + ADC12INCH_0 + ADC12EOS;

    __delay_cycles(100); // delay to allow settling
    ADC12CTL0 |= ADC12ENC; // enable conversion

    // Initialize Timer
    TA2CCTL0 = CCIE;                               // CCR0 interrupt enabled
    TA2CCTL1 = CCIE;                               // CCR1 interrupt enabled
    TA2CCR0 = 256;                                 // Set PWM period to 256 clock ticks
    TA2CCR1 = sinWave[counter];                        // Set first duty cycle value
    TA2CTL = TASSEL_2 + MC_1 + TAIE + TACLR;     // SMCLK, upmode, enable interrupt, clear TA1R

    _BIS_SR(LPM0_bits + GIE);                   // Enter LPM0 w/ interrupt

    while(1){
        waveType = getKey();
    }
}

/**
 * TimerA0 interrupt service routine
 **/
#pragma vector=TIMERA0_VECTOR
__interrupt void TIMERA0_ISR(void) {
    P1OUT |= BIT0;              // Set P1.0

    potReading = ADC12MEM1;
    switch(waveType){
    case default:
        // cry
        break;
    case DC:
        TA2CCR1 = potReading/20; // sets the duty cycle to be between 0 and 4095/X, controlled with the pot
        break;
    case SIN:
        TA2CCR1 = sinWave[counter];       // Set next duty cycle value
        break;
    case SQUARE:
        if(counter%2 == 0){ // by changing the integer modulo, you can change the frequency of the wave.
                            // when X = 2, it is changing at 80 Hz.
            squareToggle = squareToggle*(-1);
        }
        TA2CCR1 = (squareToggle*potReading + 2048)/20; // set the next duty cycle value to either 0 or 4095/X
        break;
    case TRIANGLE:
        if(counter >16){
            TA2CCR1 = (32-counter)*potReading/328; // the duty cycle will climb up and down between 0 and 200
        }
        else{
            TA2CCR1 = counter*potReading/328;
        }
    }
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
