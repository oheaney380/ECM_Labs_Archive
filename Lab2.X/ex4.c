#include <xc.h>

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
void main (void)
{
    unsigned int ADResult=0;
    unsigned char ledCount=0;
    unsigned char counter=0;
    unsigned char storeVal=0;
    unsigned char storeValLedCount=0;
    
    LATC=0; //set the output data latch levels to 0 on all pins
    LATD=0; 
    TRISC=0b00001000; //set the data direction registers to output 0-6, input 7
    TRISD=0;
    TRISA = 0b00001000; // Select pin RA7 as input
    ANSEL0 = 0b00001000; // Set pin AN7 to analogue input
    ANSEL1 = 0; // Set rest of AN pins as Digital I/O
     /* Initialise ADC */
    ADCON0=0b00001101; // Single shot, ADC port channel 3 (AN3), Enable ADC
    ADCON1=0b00000000; // Use Internal Voltage Reference
    // (Vdd and Vss)
    ADCON2=0b10101011; // Right justify result and timing settings

    while (1) {
        ADCON0bits.GO=1; // Start conversion
    
        while (ADCON0bits.GO);   // Wait until conversion done   

        ADResult =ADRESL; // Get the 8 bit LSB result
        ADResult += ((unsigned int)ADRESH << 8); // Get the 2 bit MSB result
        // ADRESH is promoted to an integer type otherwise the bits will be lost 
        ADResult = ADResult>>2;       
                
        if (ADResult < 100) {
            ledCount = 0b11111111;
        } else if (100 <= ADResult & ADResult < 110) {
            ledCount = 0b01111111;
        } else if (110 <= ADResult & ADResult < 120) {
            ledCount = 0b00111111;
        } else if (120 <= ADResult & ADResult < 130) {
            ledCount = 0b00011111;
        } else if (130 <= ADResult & ADResult < 140) {
            ledCount = 0b00001111;
        } else if (140 <= ADResult & ADResult < 150) {
            ledCount = 0b00000111;
        } else if (150 <= ADResult & ADResult < 160) {
            ledCount = 0b00000011;
        } else if (160 <= ADResult & ADResult < 170) {
            ledCount = 0b00000001;
        } else {
            ledCount = 0b00000000;
        }
        
        counter++;
        if (ADResult<=storeVal) {
            storeVal=ADResult;
            LEDout(ledCount);
            counter=0;
        } else {           
            if (storeVal < 100) {
                storeValLedCount = 0b10000000; // storeValLedCount=(ledCount+1)>>1;
            } else if (100 <= storeVal & storeVal < 110) {
                storeValLedCount = 0b01000000;
            } else if (110 <= storeVal & storeVal < 120) {
                storeValLedCount = 0b00100000;
            } else if (120 <= storeVal & storeVal < 130) {
                storeValLedCount = 0b00010000;
            } else if (130 <= storeVal & storeVal < 140) {
                storeValLedCount = 0b00001000;
            } else if (140 <= storeVal & storeVal < 150) {
                storeValLedCount = 0b00000100;
            } else if (150 <= storeVal & storeVal < 160) {
                storeValLedCount = 0b00000010;
            } else if (160 <= storeVal & storeVal < 170) {
                storeValLedCount = 0b00000001;
            } else {
                storeValLedCount = 0b00000000;
            }
            
        if (counter>5) {
            storeValLedCount=storeValLedCount>>1;
            storeVal=storeVal+10;
            counter=0;
        }
            
            LEDout(ledCount+storeValLedCount);
        }                
    }
}
