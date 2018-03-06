#include <xc.h>
#pragma config MCLRE=OFF, LVP=OFF, OSC = IRCIO, WDTEN = OFF //internal oscillator, WDT off

unsigned int ADResult;
unsigned char ledCount=0;
unsigned char timer1flag=0;

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

unsigned int getLDR() 
{
    ADCON0bits.GO=1; // Start conversion

    while (ADCON0bits.GO);   // Wait until conversion done   

    ADResult =ADRESL; // Get the 8 bit LSB result
    ADResult += ((unsigned int)ADRESH << 8); // Get the 2 bit MSB result
    // ADRESH is promoted to an integer type otherwise the bits will be lost 
    ADResult = ADResult>>2;
    
    return ADResult;
}

void changeLEDstate()
{
    if (ledCount==0) {
        ledCount=0b11111111;
    } else {
        ledCount=0;
    }
}

// High priority interrupt routine LONG TIMER INTERUPT
void interrupt InterruptHandlerHigh ()
{
    if (INTCONbits.TMR0IF) {
        timer1flag=1;
        ledCount=0;
        INTCONbits.TMR0IF=0; //clear the interrupt flag
    }
}

// Low priority interrupt routine SHORT TIMER INTERUPT
void interrupt low_priority InterruptHandlerLow ()
{
    if (PIR1bits.TMR1IF) {
        changeLEDstate();
        T0CONbits.TMR0ON=1; //turn on timer0
        T1CONbits.TMR1ON=0; //turn off timer1
        PIR1bits.TMR1IF = 0; //clear the interrupt flag
    }
}

void main (void)
{   
    //near the start of your main function
    OSCCON = 0x72; //8MHz clock ? why 0x72? Check the data sheet!
    while(!OSCCONbits.IOFS); //Wait for OSC to become stable
    
    // variables 
    
    //initial timer values
//    TMR0H=0b11111111;
      TMR0L=0;
    
    // Enable interrupts
    INTCONbits.GIEH = 1; // Global Interrupt Enable bit
    RCONbits.IPEN = 1; // Enable interrupt priority
    INTCONbits.GIEL = 1; // // Peripheral/Low priority Interrupt Enable bit
    // Set the button on RC3 to trigger an
    // interrupt. It is always high priority
    INTCONbits.INT0IE = 1; //INT0 External Interrupt Enables bit
    
    //timer0 setup
    T0CONbits.TMR0ON=1; //turn on timer0
    T0CONbits.T016BIT=0; // 16bit mode
    T0CONbits.T0CS=0; // use internal clock (Fosc/4)
    T0CONbits.PSA=0; // disable pre scaler
    T0CONbits.T0PS=0b111; // set pre scaler value(not used)
    
    // Generate an interrupt on timer overflow
    INTCONbits.TMR0IE=1; //enable TMR0 overflow interrupt
    INTCON2bits.TMR0IP=1; // TMR0 High priority
    
    //timer1 setup
    T1CONbits.TMR1ON=1; //turn on timer1
    T1CONbits.RD16=1; // 16bit mode
    T1CONbits.TMR1CS=0; // use internal clock (Fosc/4)
    T1CONbits.T1CKPS=0b11; // set pre scaler value(not used)
    
    // Generate an interrupt on timer overflow
    PIE1bits.TMR1IE=1; //enable TMR1 overflow interrupt
    IPR1bits.TMR1IP=0; // TMR1 Low priority
 
    LATC=0; //set the output data latch levels to 0 on all pins
    LATD=0; 
    TRISC=0b00001000; //set the data direction registers to output 0-6, input 7
    TRISD=0;
    TRISA = 0b00001000; // Select pin RA3 as input
    ANSEL0 = 0b00001000; // Set pin AN3 to analogue input
    ANSEL1 = 0; // Set rest of AN pins as Digital I/O
    /* Initialise ADC */
    ADCON0=0b00001101; // Single shot, ADC port channel 3 (AN3), Enable ADC
    ADCON1=0b00000000; // Use Internal Voltage Reference
    // (Vdd and Vss)
    ADCON2=0b10101011; // Right justify result and timing settings

    while (1) {

        ADResult=getLDR();
                
        if (ADResult < 120 & ledCount==0) {
            timer1flag=0;
            T1CONbits.TMR1ON=0; //turn off timer0
            T0CONbits.TMR0ON=0; //turn on timer1
            TMR0H=0; // set values of timer0 to 0
            TMR0L=0;
        } else if (ADResult > 160 & ledCount==0b11111111) {
            T1CONbits.TMR1ON=0;
        } else if (ADResult > 160 & ledCount==0 & timer1flag==0) {                        
            T0CONbits.TMR0ON=0; //turn off timer0
            T1CONbits.TMR1ON=1; //turn on timer1
            TMR0H=0; // set values of timer0 to 0
            TMR0L=0;
        } else if (ADResult < 120 & ledCount==0b11111111) {
//                TMR0H=0b11111111;
//                TMR0L=0b11111110;
            T1CONbits.TMR1ON=1;
        }
        
        LEDout(ledCount);
        
    }
}

