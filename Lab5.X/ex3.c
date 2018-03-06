#include <xc.h>
#pragma config OSC = IRCIO, WDTEN=OFF // internal oscillator
#define _XTAL_FREQ 8000000


// function to setup PWM
void initPWM(void) {
    PTCON0 = 0b00000000; // free running mode, 1:1 prescaler
    PTCON1 = 0b10000000; // enable PWM timer
    PWMCON0 = 0b01101111; // PWM1 and PWM3 set to output (pin6-4=110), PWM0/1 enabled, all independent mode
    PWMCON1 = 0x00; // special features, all 0 (default)
    PTPERL = 0b11000111; // base PWM period low byte
    PTPERH = 0b00000000; // base PWM period high byte

};

// max speed is 199, lowest speed is 0
void setMotorPWM(unsigned int speed, char direction, char motor) {
    if(motor==1){
        LATBbits.LATB0=direction;
        //flips speed to account for direction change
        if (!direction){
            PDC0L = speed<<2; // PDC0L bits 7:2 are on period bits 5:0
            PDC0H = speed>>6; // PDC0H bits 5:0 are on period bits 11:6
        } else {
            PDC0L = (199-speed)<<2; // PDC0L bits 7:2 are on period bits 5:0
            PDC0H = (199-speed)>>6; // PDC0H bits 5:0 are on period bits 11:6
        }
    } else if (motor==2) {
        //flips speed to account for direction change
        LATBbits.LATB2=direction;
        if (!direction) {
            PDC1L = speed<<2; // PDC0L bits 7:2 are on period bits 5:0
            PDC1H = speed>>6; // PDC0H bits 5:0 are on period bits 11:6 
        } else {
            PDC1L = (199-speed)<<2; // PDC0L bits 7:2 are on period bits 5:0
            PDC1H = (199-speed)>>6; // PDC0H bits 5:0 are on period bits 11:6 
        }        
    }    
};

// function to delay in seconds
//__delay_ms() is limited to a maximum delay of 89ms with an 8Mhz
//clock so you need to write a function to make longer delays
void delay_s(char seconds) {
    unsigned int i=0;
    unsigned int j=0;
    for (i=1; i<seconds; i++) {
        for (j=1; j<16; j++) {
            __delay_ms(62);
        }
    }
};

void main(void){
    unsigned int ADResult=0;
    OSCCON = 0x72; // 8MHz clock
    while(!OSCCONbits.IOFS); // wait until stable
    
    // set bits as outputs
    TRISBbits.RB0=0;
    TRISBbits.RB1=0;
    TRISBbits.RB2=0;
    TRISBbits.RB3=0;
    
    TRISAbits.RA3=1;
    ANSEL0bits.ANS3=1;
    /* Initialise ADC */
    ADCON0=0b00001101; // Single shot, ADC port channel 3 (AN3), Enable ADC
    ADCON1=0b00000000; // Use Internal Voltage Reference
    // (Vdd and Vss)
    ADCON2=0b10101011; // Right justify result and timing settings
    
    initPWM(); //setup PWM registers
    while(1){
        // fast, forward
        delay_s(5);
        // setting first motor
        setMotorPWM(199,0,1);
        // setting second motor
        setMotorPWM(199,0,2);
        
        
        // slow, forward
        delay_s(5);
        // setting first motor
        setMotorPWM(100,0,1);
        // setting second motor
        setMotorPWM(100,0,2);
        
        // slow, backward
        delay_s(5);
        // setting first motor
        setMotorPWM(100,1,1);
        // setting second motor
        setMotorPWM(100,1,2);
        
        // fast, backward
        delay_s(5);
        // setting first motor
        setMotorPWM(199,1,1);
        // setting second motor
        setMotorPWM(199,1,2);

    } 
}
