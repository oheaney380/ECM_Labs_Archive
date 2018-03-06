#include <xc.h>
#include <string.h>
#include <stdio.h>
#include "Motor_Control.h"
#pragma config OSC = IRCIO, WDTEN=OFF // internal oscillator
#define _XTAL_FREQ 8000000

// define bits
#define LCD_E LATCbits.LATC0
#define LCD_DB4 LATCbits.LATC1
#define LCD_DB5 LATCbits.LATC2
#define LCD_DB6 LATDbits.LATD0
#define LCD_DB7 LATDbits.LATD1
#define LCD_RS LATAbits.LATA6

//function to toggle enable bit on then off
void E_TOG(void){
//don?t forget to put a delay between the on and off
//commands! 5us will be plenty.
    LCD_E=1;
    __delay_us(5); // 5us delay ? remember to define _XTAL_FREQ
    LCD_E=0;
}

// function to send four bits to the LCD
void LCDout(unsigned char number){
//set data pins using the four bits from number
//toggle the enable bit to send data
    LCD_DB4 = (number<<7)>>7;
    LCD_DB5 = (number<<6)>>7;
    LCD_DB6 = (number<<5)>>7;
    LCD_DB7 = (number<<4)>>7;
    E_TOG();
    __delay_us(5); // 5us delay
    
}

//function to send data/commands over a 4bit interface
void SendLCD(unsigned char Byte, char type){
 // set RS pin whether it is a Command (0) or Data/Char (1)
 // using type as the argument
    LCD_RS=type;
 // send high bits of Byte using LCDout function
    LCDout((Byte&0xF0)>>4);
    __delay_us(10); // 10us delay
 // send low bits of Byte using LCDout function
    LCDout(Byte&0x0F);
    __delay_us(50); // 10us delay
}
void LCD_Init(void){
 // set initial LAT output values (they start up in a random state)
    LATA=0;
    LATC=0;
    LATD=0;
 // set LCD pins as output (TRIS registers)
    TRISAbits.RA6=0;
    TRISCbits.RC0=0;
    TRISCbits.RC1=0;
    TRISCbits.RC2=0;
    TRISDbits.RD0=0;
    TRISDbits.RD1=0;
 // Initialisation sequence code - see the data sheet
    __delay_ms(15); //delay 15mS
    LCDout(0b0011); //send 0b0011 using LCDout
    __delay_ms(5); //delay 5ms
    LCDout(0b0011); //send 0b0011 using LCDout
    __delay_us(200); //delay 200us
    LCDout(0b0011); //send 0b0011 using LCDout
    __delay_us(50); //delay 50us
    LCDout(0b0010); //send 0b0010 using LCDout set to four bit mode
    __delay_us(50); //delay 50us
 // now use SendLCD to send whole bytes ? send function set, clear
 // screen, set entry mode, display on etc to finish initialisation
    SendLCD(0b00101000,0); //Function set - 2-line display, 5x10 dots
    __delay_us(50); //delay 50us
    SendLCD(0b00001000,0); //Display off
    __delay_us(50); //delay 50us
    SendLCD(0b00000001,0); //Display Clear
    __delay_ms(5); //delay 50us
    SendLCD(0b00000110,0); //Entry Mode Set
    __delay_us(50); //delay 50us
    SendLCD(0b00001100,0); //Display on
    __delay_us(50); //delay 50us
}
//function to put cursor to start of line
void SetLine (char line) {
    if (line==1) {
        SendLCD(0x80,0); //Send 0x80 to set line to 1 (0x00 ddram address)
    } else if (line==2) {
        SendLCD(0xC0,0); //Send 0xC0 to set line to 2 (0x40 ddram address)
    }
 __delay_us(50); // 50us delay
}

void LCD_String(char *string){
    //While the data pointed to isn?t a 0x00 do below
    while(*string != 0){
    //Send out the current byte pointed to
    // and increment the pointer
    SendLCD(*string++,1);
    __delay_us(50); //so we can see each character
    //being printed in turn (remove delay if you want
    //your message to appear almost instantly)
    }
}

void sendCharToSerial(char charToSend){
 while (!PIR1bits.TXIF); // wait for flag to be set
 TXREG=charToSend; //transfer char H to transmitter
}

char getCharSerial(void){
while (!PIR1bits.RCIF); //wait for the data to arrive
    return RCREG; //return byte in RCREG
}

void Serial_String(char *string){
    //While the data pointed to isn?t a 0x00 do below
    while(*string != 0){
    //Send out the current byte pointed to
    // and increment the pointer
    sendCharToSerial(*string++);
    __delay_us(50); //so we can see each character
    //being printed in turn (remove delay if you want
    //your message to appear almost instantly)
    }
}
    
void main(void){
    // variables
    char letter=0;
    char buf[40] ={0};
    unsigned char i=0;
    
    TRISC = 0b11000000; //set data direction registers
                        //both need to be 1 even though RC6
                        //is an output, check the datasheet!

    OSCCON = 0b11110010; // 8MHz clock
    while(!OSCCONbits.IOFS); // wait until stable
    
    SPBRG=205; //set baud rate to 9600
    SPBRGH=0;
    BAUDCONbits.BRG16=1; //set baud rate scaling to 16 bit mode
    TXSTAbits.BRGH=1; //high baud rate select bit
    RCSTAbits.CREN=1; //continous receive mode
    RCSTAbits.SPEN=1; //enable serial port, other settings default
    TXSTAbits.TXEN=1; //enable transmitter, other settings default
    
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
    
    LCD_Init(); //Initialise the LCD
    
    while(1){
        letter=getCharSerial();
        if(letter==0xF0){
            goForward(&motorL, &motorR);
        } else if(letter==0xF1){
            turnLeft(&motorL, &motorR);
        } else if(letter==0xF2){
            turnRight(&motorL, &motorR);
        } else if(letter==0xF3){
            stop(&motorL, &motorR);
        }      
    }
}
