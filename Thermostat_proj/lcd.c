/********************************************
* Assignment: Lab 8
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>

#include "lcd.h"

void writenibble(unsigned char x);

void init_lcd(void);
void stringout(char * str);
void moveto(unsigned char pos);

void writecommand(unsigned char x);
void writedata(unsigned char x);


/* init_lcd - Do various things to initialize the LCD display */
void init_lcd()
{
    _delay_ms(15);              // Delay at least 15ms
    
    // select command
    PORTB &= (0<<PB0);
    writenibble(0b0011);    // Use writenibble to send 0011
    _delay_ms(5);               // Delay at least 4msec
    
    writenibble(0b0011);    // Use writenibble to send 0011
    _delay_us(120);             // Delay at least 100usec
    
    writenibble(0b0011);    // Use writenibble to send 0011
    
    writenibble(0b0010);    // Use writenibble to send 0010
    _delay_ms(2);
    
    writecommand(0x28);         // Function Set: 4-bit interface, 2 lines
    _delay_ms(2);
    
    writecommand(0x0f);         // Display and cursor on
    _delay_ms(2);
}

/* stringout - Print the contents of the character string "str"
 at the current cursor position. */
void stringout(char *str)
{
    char index = 0;
    while (str[index] != 0) {
        writedata(str[index]);
        index++;
    }
}

/* moveto - Move the cursor to the postion "pos" */
void moveto(unsigned char pos)
{
    writecommand(pos);
}

/* writecommand - Output a byte to the LCD display instruction register. */
void writecommand(unsigned char x)
{
    // RS=0 (PB0)
    PORTB &= (0<<PB0);
    
    // output x to PORTD [7:4]
    writenibble(x>>4);
    writenibble(x);
    _delay_ms(2);
    
}

/* writedata - Output a byte to the LCD display data register */
void writedata(unsigned char x)
{
    // RS=1 (PB0)
    PORTB |= (1<<PB0);
    writenibble(x>>4);
    writenibble(x);
    _delay_ms(2);
}

/* writenibble - Output four bits from "x" to the display */
void writenibble(unsigned char x) // x is 8-bit! Want to send LOWER 4 bits
{
    // want PORTD = ****xxxx
    int maskD = 0b11110000;
    
    PORTD &= (~maskD);
    PORTD |= ((x<<4) & maskD);
    
    // Enable 0-1-0
    PORTB |= (1<<PB1);
    PORTB |= (1<<PB1);
    PORTB &= ~(1<<PB1);
}
