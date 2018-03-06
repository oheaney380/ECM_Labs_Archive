#include <xc.h>
#include <string.h>
#include <stdio.h>
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



//variables
unsigned char alive=0;
unsigned char location=0;

//function to shorten string
int str_cut(char *str, int begin, int len)
{
    int l = strlen(str);

    if (len < 0) len = l - begin;
    if (begin + len > l) len = l - begin;
    memmove(str + begin, str + begin + len, l - len + 1);

    return len;
}

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

// High priority interrupt routine
void interrupt InterruptHandlerHigh ()
{
    if (INTCONbits.INT0IF) {
        unsigned char A=0;
        unsigned char B=0;
        A = PORTCbits.RC3;
        __delay_us(50); //delay 50us
        B = PORTCbits.RC3;
        if (A&B) {
            alive=1; // change direction
            location=!location;
        }
        INTCONbits.INT0IF = 0; //clear the interrupt flag
    }
}

void main (void) {
    
    // variables
    char buf[40]; //buffer for characters for LCD
    char buf2[40]; //buffer for characters for LCD
    char lvl[40]; //buffer for characters for LCD
    char lvl2[40]; //buffer for characters for LCD
    char kitty[40]; //buffer for characters for LCD  
    unsigned char storeval=0;
    unsigned int LDRresult=0;
    unsigned int int_part=0;
    unsigned int frac_part=0;
    unsigned char i=0;
    unsigned char x=0;
    TMR0L=0; //start from 0
    
    //near the start of your main function
    OSCCON = 0x72; //8MHz clock ? why 0x72? Check the data sheet!
    while(!OSCCONbits.IOFS); //Wait for OSC to become stable
    
    //AD Initialisation
    LATC=0; //set the output data latch levels to 0 on all pins
    LATD=0; 
    TRISAbits.RA3 = 1;
    ANSEL0bits.ANS3 = 1; // Set pin AN3 to analogue input
    
    TRISC=0b00001000; //set the data direction registers to output 0-6, input 7
    TRISD=0;
    // Enable interrupts
    INTCONbits.GIEH = 1; // Global Interrupt Enable bit
    // Set the button on RC3 to trigger an
    // interrupt. It is always high priority
    INTCONbits.INT0IE = 1; //INT0 External Interrupt Enable bit
    
    /* Initialise ADC */
    ADCON0=0b00001101; // Single shot, ADC port channel 3 (AN3), Enable ADC
    ADCON1=0b00000000; // Use Internal Voltage Reference
    // (Vdd and Vss)
    ADCON2=0b10101011; // Right justify result and timing settings
    
    LCD_Init(); //Initialise the LCD
        
    while(1) {
        
        while(!alive) {
            SetLine(1); //Set Line 1
            sprintf(buf,"Press to (>^.^)>");
            SendLCD(0b00000010,0); // move cursor to home
            __delay_ms(2);
            LCD_String(buf); // output string to LCD

            SetLine(2); //Set Line 2
            sprintf(buf," <(^.^)>  Start");
            LCD_String(buf); // output string to LCD
            for (i=1; i<15; i++) { // compound delay
                __delay_ms(50);
            }
            
            SetLine(1); //Set Line 1
            sprintf(buf,"Press to ^(^.^)^");
            SendLCD(0b00000010,0); // move cursor to home
            __delay_ms(2);
            LCD_String(buf); //output string to LCD

            SetLine(2); //Set Line 2
            sprintf(buf,"  (^.^)   Start");
            LCD_String(buf); // output string to LCD
            for (i=1; i<15; i++) { // compound delay
                __delay_ms(50);
            }
            
            SetLine(1); //Set Line 1
            sprintf(buf,"Press to <(^.^<)");
            SendLCD(0b00000010,0); // move cursor to home
            __delay_ms(2);
            LCD_String(buf); //output string to LCD
            
            SetLine(2); //Set Line 2
            sprintf(buf," (>^.^<)  Start");
            LCD_String(buf); // output string to LCD
            for (i=1; i<15; i++) { // compound delay
                __delay_ms(50);
            }
        }

        SendLCD(0b00000001,0); //Clear Display
        __delay_us(50); //Delay to let display clearing finish
        SendLCD(0b00000010,0); // move cursor to home
        __delay_ms(2);
        
        sprintf(lvl, "               <        <     <     << "); // why must do?
        sprintf(lvl2,"       <            <      <     <     ");
        sprintf(kitty,"O");
        
        if(alive) {
            for (x=0; x<40; x++) {
                
                if (location) {
                    *lvl=strcat("D",lvl);
                } else {
                    *lvl2=strcat("D",lvl2);
                }
                
                SetLine(1); //Set Line 1
                LCD_String(lvl); //output string to LCD
                SetLine(2); //Set Line 2
                LCD_String(lvl2); //output string to LCD
                                
                str_cut(lvl,0,1);
                str_cut(lvl2,0,1);
                
                for (i=1; i<10; i++) { // Compound delay for delay
                    __delay_ms(50);
                }
                
                // Collision detection
                if (location) {
                    if (x==15 | x==24 | x==30 | x==36 | x==37) {
                        SendLCD(0b00000001,0); //Clear Display
                        __delay_us(50); //Delay to let display clearing finish
                        SendLCD(0b00000010,0); // move cursor to home
                        __delay_ms(2);
                        LCD_String("You Died!"); //output string to LCD
                        for (i=1; i<50; i++) { // Compound delay for delay
                            __delay_ms(50);
                        }
                        break;
                    }                    
                } else {
                    if (x==7 | x==20 | x==32 | x==27 | x==33){
                        SendLCD(0b00000001,0); //Clear Display
                        __delay_us(50); //Delay to let display clearing finish
                        SendLCD(0b00000010,0); // move cursor to home
                        __delay_ms(2);
                        LCD_String("You Died!"); //output string to LCD
                        for (i=1; i<50; i++) { // Compound delay for delay
                            __delay_ms(50);
                        }
                        break;
                    }                    
                }
            }
            
            // Win Condition
            if (x==40) {
                SendLCD(0b00000001,0); //Clear Display
                __delay_us(50); //Delay to let display clearing finish
                SendLCD(0b00000010,0); // move cursor to home
                __delay_ms(2);
                LCD_String("You Won!"); //output string to LCD
                for (i=1; i<50; i++) { // Compound delay for delay
                    __delay_ms(50);
                }               
            }
            alive=0;
        }
    }
}