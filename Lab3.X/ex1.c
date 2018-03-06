#include <xc.h>

unsigned char direction=0;

/************************************
/ Function delay
/ Used to introduce a delay of length t
************************************/
void delay (int t)
{
    unsigned char i; //declare a variable
    for (i = 0; i < t ; i++);
}

/************************************
/ Function LEDout
/ Used to display number on the LED array
/ in binary
************************************/

void LEDout(int number)
{
    LATC=(number & 0b00111100)<<2 | (LATC & 0b00001111);
    LATD=(number & 0b11000000)>>2 | (number & 0b00000011)<<2 | (LATD & 0b10000011);
     
}

// High priority interrupt routine
void interrupt InterruptHandlerHigh ()
{
    if (INTCONbits.INT0IF) {
        unsigned char A=0;
        unsigned char B=0;
        A = PORTCbits.RC3;
        B = PORTCbits.RC3;
        if (A&B) {
            direction=!direction; // change direction
        }

        INTCONbits.INT0IF = 0; //clear the interrupt flag
    }
}

void main (void)
{
    unsigned char counter=0;
    unsigned char timer=0;
    
    LATC=0; //set the output data latch levels to 0 on all pins
    LATD=0; 
    TRISC=0b00001000; //set the data direction registers to output 0-6, input 7
    TRISD=0;
    // Enable interrupts
    INTCONbits.GIEH = 1; // Global Interrupt Enable bit
    // Set the button on RC3 to trigger an
    // interrupt. It is always high priority
    INTCONbits.INT0IE = 1; //INT0 External Interrupt Enable bit

    while (1) {
        timer++;
        if (timer>50) {
            timer=0;
            if (direction==0) {
                counter++;
            } else {
                counter--;
            }
        }    
        LEDout(counter);
    }
}

