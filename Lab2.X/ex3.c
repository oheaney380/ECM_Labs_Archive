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
    int a=1; //declare a as an integer
    unsigned char A; //declare a variable
    unsigned char B; //declare a variable
    unsigned char reading; //declare a variable
    unsigned char LEDCount=0; //declare a variable
    unsigned char buffer=0;
    int timeCount=0; //declare a variable
    
    LATC=0; //set the output data latch levels to 0 on all pins
    LATD=0; 
    TRISC=0b00001000; //set the data direction registers to output 0-6, input 7
    TRISD=0;
    while (1){        
        A = PORTCbits.RC3;
        delay(2);
        B = PORTCbits.RC3;
        reading = (A+B)>>1;
        
        if (reading) {
            timeCount++;
            if (timeCount==1) {
                buffer++;
            } else if (timeCount>75) {
                buffer++;
                delay(25);
            }
        } else {
            timeCount = 0;
        }
        
        if (buffer==10) {
            buffer=0;
            if (LEDCount==0){
                LEDCount=1;
            } else {
                LEDCount=LEDCount<<1;
            }            
        }
              
        LEDout(LEDCount);
    }
}
