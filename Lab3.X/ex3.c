#include <xc.h>
#pragma config MCLRE=OFF, LVP=OFF, OSC = IRCIO, WDTEN = OFF //internal oscillator, WDT off

unsigned char counter=0;
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

void counterCheck(int number)
{
    if (number==0) {
        counter++;
    } else {
        counter--;
    }
}

void directionCheck()
{
    unsigned char A=0;
    unsigned char B=0;
    A = PORTCbits.RC3;
    B = PORTCbits.RC3;
    if (A&B) {
        direction=!direction; // change direction
    }
}

// Low priority interrupt routine
void interrupt  low_priority InterruptHandlerLow ()
{
    if (INTCONbits.TMR0IF) {
        TMR0H=0b11011111;
        TMR0L=0b11111111;
        counterCheck(direction);
        
        INTCONbits.TMR0IF=0; //clear the interrupt flag
    }
}

void interrupt InterruptHandlerHigh ()
{
    if (INTCONbits.INT0IF) {
        directionCheck();
        
        INTCONbits.INT0IF = 0; //clear the interrupt flag
    }
}

void main (void)
{
    //near the start of your main function
    OSCCON = 0x72; //8MHz clock ? why 0x72? Check the data sheet!
    while(!OSCCONbits.IOFS); //Wait for OSC to become stable
    
    // variables    
    
    //initial timer values
    TMR0H=0b11011111;
    TMR0L=0b11111111;
    
    // Enable interrupts
    INTCONbits.GIEH = 1; // Global Interrupt Enable bit
    RCONbits.IPEN = 1; // Enable interrupt priority
    INTCONbits.GIEL = 1; // // Peripheral/Low priority Interrupt Enable bit
    // Set the button on RC3 to trigger an
    // interrupt. It is always high priority
    INTCONbits.INT0IE = 1; //INT0 External Interrupt Enables bit
    
    //timer setup
    T0CONbits.TMR0ON=1; //turn on timer0
    T0CONbits.T016BIT=0; // 16bit mode
    T0CONbits.T0CS=0; // use internal clock (Fosc/4)
    T0CONbits.PSA=0; // disable pre scaler
    T0CONbits.T0PS=0b111; // set pre scaler value(not used)

    // Generate an interrupt on timer overflow
    INTCONbits.TMR0IE=1; //enable TMR0 overflow interrupt
    INTCON2bits.TMR0IP=0; // TMR0 Low priority
 
    LATC=0; //set the output data latch levels to 0 on all pins
    LATD=0; 
    TRISC=0b00001000; //set the data direction registers to output 0-6, input 7
    TRISD=0;

    while (1) { 
        LEDout(counter);
    }
}

