#include <msp430.h>
#include "peripherals.h"

typedef enum {DC, SIN, SQUARE, TRIANGLE} waveType;

void adc_convert(void);
void starttimerA2(void);
void DACAttack(int measurement);

unsigned char counter;              // Current location in sinWave array
unsigned char sinWave[32] = {          // sinWave array, preset to values of sine
    128, 140, 152, 164, 173, 181, 187, 191,
    192, 191, 187, 181, 173, 164, 152, 140,
    128, 116, 104, 92, 83, 75, 69, 65,
    64, 65, 69, 75, 83, 92, 104, 116 };


unsigned int i;                     // Used for 'for' loops.
volatile unsigned int in_wheel;
volatile unsigned int DACPin;
unsigned int done = 0;

int squareToggle = 1;
waveType state = DC;

void adc_convert() {
    ADC12CTL0 &= ~ADC12SC; // clear the start bit
    ADC12CTL0 |= ADC12SC; // Sampling and conversion start
}

void DACAttack(int measurement){

}

void main(void) {
    WDTCTL = WDTPW + WDTHOLD;                   // Stop WDT

    REFCTL0 &= ~REFMSTR;

    P8SEL &= ~BIT0;
    P8DIR |= BIT0;
    P8OUT |= BIT0;

    P6SEL &= ~BIT1;
    P6DIR |= BIT1;
    P6IN |= BIT1;

    ADC12CTL0 = ADC12SHT0_9 | ADC12REFON | ADC12ON | ADC12MSC;
    ADC12CTL1 = ADC12SHP | ADC12CONSEQ_1;
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_0;
    ADC12MCTL1 = ADC12SREF_0 + ADC12INCH_0 + ADC12EOS;
    __delay_cycles(100);

    ADC12CTL0 |= ADC12ENC;

    ADC12IE = BIT1;
    _BIS_SR(GIE);


    configKeypad();
    starttimerA2();
    configDisplay();
    DACInit();

    Graphics_clearDisplay(&g_sContext);
    Graphics_drawStringCentered(&g_sContext, "BTN 1=DC", AUTO_STRING_LENGTH, 48, 20, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "BTN 2=SQ", AUTO_STRING_LENGTH, 48, 30, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "BTN 3=SAW", AUTO_STRING_LENGTH, 48, 40, TRANSPARENT_TEXT);
    Graphics_drawStringCentered(&g_sContext, "BTN 4=TRI", AUTO_STRING_LENGTH, 48, 50, TRANSPARENT_TEXT);
    Graphics_flushBuffer(&g_sContext); // update display

    done = 1;

    volatile int btn;

    while(1){
        btn = getKey();

        if(btn == '1') {
            state = DC;
        }
        else if(btn == '2') {
            state = SIN;
        }
        else if(btn == '3') {
            state = SQUARE;
//            TA2CCR0 = 256;
        }
        else if(btn == '4') {
            state = TRIANGLE;
        }
    }
}

/**
 * TimerA0 interrupt service routine
 **/
#pragma vector=TIMER2_A0_VECTOR
__interrupt void TimerA2_ISR(void) {
    adc_convert();
    if(done == 1) {
    switch(state){
        case DC:
            DACSetValue(in_wheel);
            break;
        case SIN:
            if(counter%2 == 0){ // by changing the integer modulo, you can change the frequency of the wave.
                                // when X = 2, it is changing at 80 Hz.
                DACSetValue(0);
            }
            else {
                DACSetValue(in_wheel); // set the next duty cycle value to either 0 or 4095/X
            }
            break;
        case SQUARE:
            DACSetValue(counter*128);
            break;
        case TRIANGLE:
            if(counter >8){
                DACSetValue((16-counter)*256); // the duty cycle will climb up and down between 0 and 200
            }
            else{
                DACSetValue(counter*256);
            }
            break;
    }
    counter += 1;               // Add Offset to CCR0
    if ( counter == 16) {
        counter = 0;            // Reset counter
    }
    }
}

#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR(void) {
    in_wheel = ADC12MEM1;
    DACPin = ADC12MEM0;
    TA2CCR0 = (in_wheel/12)+10;
}

void starttimerA2(void) {
    TA2CTL = TASSEL_1 + MC_1 + ID_0;
    TA2CCR0 = 10;
    TA2CCTL0 = CCIE;
}
