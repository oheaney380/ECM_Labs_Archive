#include <xc.h>
#pragma config OSC = IRCIO, WDTEN=OFF // internal oscillator
#define _XTAL_FREQ 8000000

struct DC_motor {
    char power; //motor power, out of 100
    char direction; //motor direction, forward(1), reverse(0)
    unsigned char *dutyLowByte; //PWM duty low byte address
    unsigned char *dutyHighByte; //PWM duty high byte address
    char dir_pin; // pin that controls direction on PORTB
    int PWMperiod; //base period of PWM cycle
}; //struct motor

// function to setup PWM
void initPWM(void) {
    PTCON0 = 0b00000000; // free running mode, 1:1 prescaler
    PTCON1 = 0b10000000; // enable PWM timer
    PWMCON0 = 0b01101111; // PWM1 and PWM3 set to output (pin6-4=110), PWM0/1 enabled, all independent mode
    PWMCON1 = 0x00; // special features, all 0 (default)
    //PWMperiod is 199
    PTPERL = 0b11000111; // base PWM period low byte
    PTPERH = 0b00000000; // base PWM period high byte 
    PDC0L = 0<<2;
    PDC0H = 0>>6;
    PDC1L = 0<<2;
    PDC1H = 0>>6;
}

// function to set PWM from the values in the motor structure
// function to set PWM from the values in the motor structure
void setMotorPWM(struct DC_motor *m)
{
    int PWMduty; //tmp variable to store PWM duty cycle
    if (m->direction){ //if forward
    // low time increases with power
    PWMduty=m->PWMperiod - ((int)(m->power)*(m->PWMperiod))/100;
    }
    else { //if reverse
    // high time increases with power
    PWMduty=((int)(m->power)*(m->PWMperiod))/100;
    }
    PWMduty = (PWMduty << 2); // two LSBs are reserved for other things
    *(m->dutyLowByte) = PWMduty & 0xFF; //set low duty cycle byte
    *(m->dutyHighByte) = (PWMduty >> 8) & 0x3F; //set high duty cycle byte

    if (m->direction){ // if direction is high,
    LATB=LATB | (1<<(m->dir_pin)); // set dir_pin bit in LATB to high
    } else { // if direction is low,
    LATB=LATB & (~(1<<(m->dir_pin))); // set dir_pin bit in LATB to low
    }
}

void setSingleMotorFullSpeed(struct DC_motor *m)
{
 for (m->power; (m->power)<100; (m->power)++){ //increase motor power until 100
    setMotorPWM(m); //pass pointer to setMotorSpeed function (not &m here)
    __delay_ms(5); //delay of 5 ms (500 ms from 0 to 100 full power)
 }
}

void stopSingleMotor(struct DC_motor *m)
{
 for (m->power; (m->power)>0; (m->power)--){ //increase motor power until 100
    setMotorPWM(m); //pass pointer to setMotorSpeed function (not &m here)
    __delay_ms(5); //delay of 5 ms (500 ms from 0 to 100 full power)
 }
}

void fullSpeed(struct DC_motor *m_L, struct DC_motor *m_R)
{
    while(m_L->power<100 || m_R->power<100){
        if(m_L->power<100){
            m_L->power++;            
        }
        if(m_R->power<100){
            m_R->power++;            
        }
        setMotorPWM(m_L);
        setMotorPWM(m_R);
        __delay_us(50);
    }    
}

void stop(struct DC_motor *m_L, struct DC_motor *m_R)
{
    while(m_L->power>0 || m_R->power>0){
        if(m_L->power>0){
            m_L->power--;            
        }
        if(m_R->power>0){
            m_R->power--;            
        }
        setMotorPWM(m_L);
        setMotorPWM(m_R);
        __delay_us(50);
    }
}

void turnLeft(struct DC_motor *m_L, struct DC_motor *m_R)
{
    m_L->direction=0;
    m_R->direction=1;
    fullSpeed(m_L, m_R);
}

void turnRight(struct DC_motor *m_L, struct DC_motor *m_R)
{
    m_L->direction=1;
    m_R->direction=0;
    fullSpeed(m_L, m_R);
}

// function to delay in seconds
//__delay_ms() is limited to a maximum delay of 89ms with an 8Mhz
//clock so you need to write a function to make longer delays
void delay_s(char seconds) {
    unsigned int i=0;
    unsigned int j=0;
    for (i=1; i<=seconds; i++) {
        for (j=1; j<=20; j++) {
            __delay_ms(50);
        }
    }
}

void main(void){
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
    
    struct DC_motor motorL, motorR; //declare two DC_motor structures
    motorL.power=0; //zero power to start
    motorL.direction=1; //set default motor direction
    motorL.dutyLowByte=(unsigned char *)(&PDC0L); //store address of PWM duty low byte
    motorL.dutyHighByte=(unsigned char *)(&PDC0H); //store address of PWM duty high byte
    motorL.dir_pin=0; //pin RB0/PWM0 controls direction
    motorL.PWMperiod=199; //store PWMperiod for motor
    //same for motorR but different PWM registers and direction pin
    motorR.power=0; //zero power to start
    motorR.direction=1; //set default motor direction
    motorR.dutyLowByte=(unsigned char *)(&PDC1L); //store address of PWM duty low byte
    motorR.dutyHighByte=(unsigned char *)(&PDC1H); //store address of PWM duty high byte
    motorR.dir_pin=2; //pin RB0/PWM0 controls direction
    motorR.PWMperiod=199; //store PWMperiod for motor
    
    while(1){
        
        delay_s(5);
        fullSpeed(&motorL, &motorR);
        
        delay_s(5);
        stop(&motorL, &motorR);
        
        delay_s(5);
        turnLeft(&motorL, &motorR);
        
        delay_s(5);
        turnRight(&motorL, &motorR);
        
    } 
}
