/********************************************
*
*  Name: Ann-Chie (Angela) Shao
*  Section: Lab W 3:30-5:00
*  Assignment: Lab 7 
*
********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h> // for interrupts

volatile unsigned char adcIn = 0;               // volatile variable for ADC input 
volatile unsigned char state, state_bef;        // states {0, 1, 2}
volatile unsigned char ten=0, one=0, tenth=0, ten_b, one_b, tenth_b;   // time variables

void init_adc(void);                // input A0, 8-bit, AVCC, prescalar 2^7, ADC interrupt
void init_lcd(void);
void init_timer1(unsigned short);
void stringout(char * str);
void moveto(unsigned char pos);

void writecommand(unsigned char x);
void writedata(unsigned char x);
void writenibble(unsigned char x);

int main(void) {
    // configure PD [7:4] and PD [1:0] as outputs
    DDRD |= 0b11110000;
    DDRB |= 0b00000011;
    
    sei();                  // enable global interrupts
    
    // initialize ADC, LCD, timer
    init_lcd();             
    writecommand(0x01);     // clear LCD
    init_timer1(6250);      // set modulus/max value to 6250
    init_adc();      
    
    // display 00.0  
    char display[5];
    snprintf(display, 5, "%d%d.%d", ten, one, tenth);
    moveto(0x80);
    stringout(display); 
    
    ADCSRA |= (1<<ADSC);    // start 1st conversion
    
    //TCCR1B |= (1<<CS12);    // set prescalar 256 and start counter

    state = 0;              // set initial state
    while (1) {        
        ADCSRA |= (1<<ADSC);    // start next conversion
    }   
    
    return 0;   /* never reached */
}

ISR (TIMER1_COMPA_vect) { // 16-bit timer1; update time
    tenth++; // change time
    if (tenth > 9) { 
        tenth = 0;
        one++;
    }    
    if (one > 9) { 
        one = 0;
        ten++;
    }
    if (ten > 5) { 
        ten = 0;
        one = 0;
        tenth = 0;
    }
    
    if (state==0) {         // if S0
        // don't change display / do nothing
    } else if (state==1) {  // if S1
        // change display
        if ((ten_b!=ten) || (one_b!=one) || (tenth_b!=tenth)) { // if time changes
            char display[5];
            snprintf(display, 5, "%d%d.%d", ten, one, tenth);
            moveto(0x80);
            stringout(display);
        }
    } else {                // if S2
        // don't change display / do nothing
    }
    
    ten_b=ten; one_b=one; tenth_b=tenth; // update the last ten, one, tenth
}

ISR (ADC_vect) { // update state 
    adcIn = ADCH;           // read ADC result
      
    if(adcIn != 255) {      // if some button is pressed
        if ((adcIn > 45) && (adcIn < 60)) { // if UP (Start_Stop)
            if (state==0) {          // if S0, change to S1
                state=1; 
                TCCR1B |= (1<<CS12);    // start counter set (prescalar = 256)
            } else if (state==1) {   // if S1, change to S0
                state=0;
                TCCR1B &= ~(1<<CS12);   // stop counter
            } else {                   // if S2, change to S1
                state=1;
            }
        } else if ((adcIn > 95) && (adcIn < 110)) { // if DOWN (Lap_Reset)
            if (state == 0) {       // if S0, reset                
                ten=0; one=0; tenth=0;
                char display[5];
                snprintf(display, 5, "%d%d.%d", ten, one, tenth);
                moveto(0x80);
                stringout(display);
            } else if (state == 1) {       // if S1, change to S2
                state = 2;
            } else if (state == 2) {       // if S2, change to S1
                state = 1;
            }
        } 
    }     
    ADCSRA |= (1<<ADSC);
}


/* initialize ADC */
void init_adc() {
    ADMUX |= 0;             // select input channel (A0)
    ADMUX |= (1<<REFS0);    // select high V ref
    ADMUX |= (1<<ADLAR);    // select 8-bit (1<<ADLAR)
    ADCSRA |= 7;            // select prescaler value (2^7)
    ADCSRA |= (1<<ADEN);    // enable ADC module (1<<ADEN)       
    
    ADCSRA |= (1<<ADIE);    // enable ADC interrupts
}

/* initialize timer */
void init_timer1(unsigned short max) {
    TCCR1B |= (1<<WGM12);   // set to CTC (clear timer on compare) mode
    TIMSK1 |= (1<<OCIE1A);  // enable timer interrupt
    OCR1A = max;            // load MAX count/modulus
}

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
