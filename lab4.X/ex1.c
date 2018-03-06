#include <xc.h>
#pragma config MCLRE=OFF, LVP=OFF, OSC = IRCIO, WDTEN = OFF //internal oscillator, WDT off

// don?t forget to put function prototypes at the start of your program
// precise delays can be produced using the __delay_us() and __delay_ms()
// macro functions. For these to work you must write the line
#define _XTAL_FREQ 8000000 //i.e. for an 8MHz clock frequency
// near the top of your main.c file so the functions can calculate how
// long to make the delay for different clock frequencies.

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
//function to send four bits to the LCD
//void LCDout(unsigned char number){
////set data pins using the four bits from number
// //toggle the enable bit to send data
//    LCD_DB4 = (number<<7)>>7;
//    LCD_DB5 = (number<<6)>>7;
//    LCD_DB6 = (number<<5)>>7;
//    LCD_DB7 = (number<<4)>>7;
//    E_TOG();
//    __delay_us(5); // 5us delay
//    
//}

void LCDout(unsigned char number)
{
    if(number & 0b0001){ LCD_DB4 = 1;} else { LCD_DB4 = 0;}
    if(number & 0b0010){ LCD_DB5 = 1;} else { LCD_DB5 = 0;}
    if(number & 0b0100){ LCD_DB6 = 1;} else { LCD_DB6 = 0;}
    if(number & 0b1000){ LCD_DB7 = 1;} else { LCD_DB7 = 0;}
    E_TOG();
    __delay_us(5);
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
    SendLCD(0b00001110,0); //Display on
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

void main (void) {
    
    //near the start of your main function
    OSCCON = 0x72; //8MHz clock ? why 0x72? Check the data sheet!
    while(!OSCCONbits.IOFS); //Wait for OSC to become stable
    
    LCD_Init(); //Initialize the LCD
    SetLine(1); //Set Line 1
    SendLCD(0x48,1); //H
    SendLCD(0x65,1); //E
    SendLCD(0x6C,1); //L
    SendLCD(0x6C,1); //L
    SendLCD(0x6F,1); //O
    SendLCD(0x20,1); //(space)
    SendLCD(0x57,1); //W
    SendLCD(0x6F,1); //O
    SendLCD(0x72,1); //R
    SendLCD(0x6C,1); //L
    SendLCD(0x64,1); //D
    SendLCD(0x21,1); //!
    while(1);  
}