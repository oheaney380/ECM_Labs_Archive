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
    unsigned char i; //declare a variable
    
    LATC=0; //set the output data latch levels to 0 on all pins
    LATD=0;
    TRISC=0; //set the data direction registers to output on all pins
    TRISD=0;
    while (1){
        for (i = 1; i < 8 ; i++){ // increment i
            a=a<<1;
            LEDout(a); //output a on the LED array in binary
            // Delay so human eye can see change
            delay(10); //delay of length 100            
        }
        for (i = 1; i < 8 ; i++){ // increment i
            a=a>>1;
            LEDout(a); //output a on the LED array in binary
            // Delay so human eye can see change
            delay(10); //delay of length 100
        }
    }
}
