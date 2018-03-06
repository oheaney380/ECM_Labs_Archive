#include <xc.h>
#pragma config OSC = IRCIO // internal oscillator
#define _XTAL_FREQ 8000000


// function to setup PWM
void initPWM(void) {
    PTCON0 = 0b00001100; // free running mode, 1:64 prescaler = 32 us
    PTCON1 = 0b10000000; // enable PWM timer
    PWMCON0 = 0b00101111; // PWM0/1 enabled, all independent mode
    PWMCON1 = 0x00; // special features, all 0 (default)
    PTPERL = 0b01110000; // base PWM period low byte
    PTPERH = 0b00000010; // base PWM period high byte
};

 // output correct PWM output for given angle
void angle2PWM(int angle) {
    unsigned int PWM=0;
    //note: the two LSBs in PDC0L have a different function;
    //this mean to you need to shift your on period to the left <<
    //by 2 bits
    angle=angle+90; //Convert to range between 0 and 180 deg
    PWM=(unsigned int)((angle*10.75) + 700)/36;
    
    PDC0L = PWM<<2; // PDC0L bits 7:2 are on period bits 5:0
    PDC0H = PWM>>6; // PDC0H bits 5:0 are on period bits 11:6
};

// function to delay in seconds
//__delay_ms() is limited to a maximum delay of 89ms with an 8Mhz
//clock so you need to write a function to make longer delays
void delay_s(char seconds) {
    unsigned int i=0;
    for (i=1; i<(seconds<<5); i++) {
        __delay_ms(62);
    }
};

void main(void){
    int angle=0;
    OSCCON = 0x72; //8MHz clock
    while(!OSCCONbits.IOFS); //wait until stable

    initPWM(); //setup PWM registers
    while(1){
        for (angle=-90; angle<=90; angle++) {
            angle2PWM(angle); //set servo to -90 deg
            __delay_ms(50);
        }
        
        for (angle=90; angle>=-90; angle--) {
            angle2PWM(angle); //set servo to -90 deg
            __delay_ms(50);
        }
                    //        angle2PWM(90); //set servo to 90 deg
//        delay_s(3);//wait for 1s
//        angle2PWM(-90); //set servo to 90 deg
//        delay_s(3);//wait for 1s
    } 
}
