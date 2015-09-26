/********************************************
 Ann-Chie (Angela) Shao
 annchies@usc.edu
 Assignment: Project
 ********************************************/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include "ds1631.h"

#define FOSC 16000000           // clock frequency
#define BAUD 9600               // baud rate used
#define MYUBRR FOSC/16/BAUD-1   // value for UBRR0

volatile unsigned char state=00, but_H=0, but_L=0, state_H=0, state_L=0;
volatile unsigned char A=0, B=0, changed = 0;
volatile int cnt=0;

// for remote sensor ISR
volatile char remote[5], allRec = 0;   // 4byte buffer of input; allRec - all char received
volatile int numRec = 0, recInt = 0;    // how many data char received; int value

char rx_char();
void tx_char(char ch);

int main(void) {
    // configure PD[7:4], PB[1:0], PB[5:4] (LED) as outputs
    // enable pull-up R in PC[5:4] (I2C) & PC[2,3] (encoder)
    DDRD |= ((1<<DD7) | (1<<DD6) | (1<<DD5) | (1<<DD4));
    DDRB |= ((1<<DD5) | (1<<DD4) | (1<<DD1) | (1<<DD0));
    PORTC |= ((1<<PC5) | (1<<PC4) | (1<<PC3) | (1<<PC2));
    PORTD |= ((1<<PD3) | (1<<PD2)); // for buttons

    // enable tristate to read from Rx (which shares PD0 with USB)
    DDRB |= (1<<DD3);
    PORTB &= ~(1<<PB3);
    
    UCSR0B |= (1 << TXEN0 | 1 << RXEN0 | 1 << RXCIE0); // Enable RX and TX & ISR
    UCSR0C = (3 << UCSZ00);
    UBRR0 = MYUBRR;
    
    PCICR |= (1<<PCIE1);    // enable pin change interrupt for Port C
    PCMSK1 |= ((1<<PCINT10) | (1<<PCINT11)); // initialize PC[3:2] to generate interrupt
    
    sei();  // enable global interrupt
    
    init_lcd();
    
    // initialize DS1631 (temperature sensor)
    unsigned char t[2];
    ds1631_init();
    ds1631_conv();
    
    char setting[30];
    char display[10];
    
    // initial LCD screen
    writecommand(0x01); // clear
    snprintf(setting, 30, "Low:");
    moveto(0xc0);
    stringout(setting);
    snprintf(setting, 30, "High:");
    moveto(0xc8);
    stringout(setting);
    snprintf(setting, 30, "Temp:");
    moveto(0x80);
    stringout(setting);
    snprintf(setting, 30, "Rmt:");
    moveto(0x88);
    stringout(setting);
    
    int low=50, high=55;
    int num, num_bef = 0;
    
    while(1) {
        // detect button
        if ((PIND & (1<<PD2)) == 0) {
            but_L = 1;
            but_H = 0;
        } else if ((PIND & (1<<PD3)) == 0) {
            but_L = 0;
            but_H = 1;
        }
        
        // when encoder is turned
        if (changed) {
            if (but_L) {
                cnt = low;
                
                if (cnt >= high) { // check range
                    snprintf(display, 6, "%d", high);
                    moveto(0xc4);
                    stringout(display);
                } else {
                    snprintf(display, 6, "%d", cnt);
                    moveto(0xc4);
                    stringout(display);
                }
                low = cnt;
            } else {
                cnt = high;
                
                if (cnt <= low) { // check range
                    snprintf(display, 6, "%d", low);
                    moveto(0xcd);
                    stringout(display);
                } else {
                    snprintf(display, 6, "%d", cnt);
                    moveto(0xcd);
                    stringout(display);
                }
                high = cnt;
            }
            changed = 0;
        }
        
        // convert temp input to decimal (by *100 to make it a whole #); display
        ds1631_temp(t);     // read data
        
        int whole = t[0] * 100;  // multiply whole # by 10
        int dec;
        if (t[1] == 0x00) {       // find decimal
            dec = 0;
        } else {
            dec = 50;
        }
        num = whole + dec;      // num = actual_num * 10
        
        int F_100 = 9*num/5 + 3200;
        int F = F_100/100;         // convert F to actual temp in F
        
        snprintf(display, 6, "%d", F);
        moveto(0x85);
        stringout(display);
        
        // see if LED needs to light up
        if (F > high) {
            state_H = 1;
        } else if (F < low) {
            state_L = 1;
        } else {
            state_H = 0;
            state_L = 0;
        }
        
        // light up LED if needed
        if (state_H) {
            PORTB |= (1<<PB5);
        } else if (state_L) {
            PORTB |= (1<<PB4);
        } else {
            PORTB &= ~(1<<PB5);
            PORTB &= ~(1<<PB4);
        }
        
        // calculate ch[] to transmit
        char ch[4];
        if (F >= 0) {
            ch[0] = '+';
        } else {
            ch[0]= '-';
            F += 2*F; // change to positive
        }
        if (F >= 100) {
            ch[1] = F/100 + '0';
            F -= F/100 * 100;
        } else {
            ch[1] = '0';
        }
        if (F >= 10) {
            ch[2] = F/10 + '0';
            F -= F/10 * 10;
        } else {
            ch[2] = '0';
        }
        if (F >= 0) {
            ch[3] = F + '0';
        } else {
            ch[3] = '0';
        }
        
        // if temp sensor data changes, transmit to remote unit
        if (num_bef != num) {
            tx_char(ch[0]);
            tx_char(ch[1]);
            tx_char(ch[2]);
            tx_char(ch[3]);
            
            num_bef = num;
        }
        
        // calculate decimal value (bc received buffer is in ascii)
        recInt = 100*(remote[1]-'0');
        recInt += 10*(remote[2]-'0');
        recInt += remote[3]-'0';
     
        if (remote[1] == '-') { // if negative
            recInt *= -1;
        }
        
        // when data from remote is received; display
        if (allRec) {
            snprintf(display, 6, "%d", recInt);
            moveto(0x8c);
            stringout(display);
            
            allRec = 0; // reset allRec
        }
    }
    
    return 0;   /* never reached */
}

ISR (USART_RX_vect) { // called when Rx has data
    char ch = UDR0;
    
    if (ch == '-' || (ch == '+')) {
        numRec = 0;
        remote[0] = ch;
    } else {
        remote[numRec] = ch;
    }
    
    numRec++;
    
    if (numRec == 4) // if all 4 char are received
        allRec = 1;
}

void tx_char(char chArr) {
    // Wait for transmitter data register empty
    while ((UCSR0A & (1<<UDRE0)) == 0) {}
    UDR0 = chArr;
}

ISR (PCINT1_vect) { // called if bits of pin PC[5:4] changes
    changed = 1;
    
    // retrieve input
    A = (PINC & (1<<PC2));
    B = (PINC & (1<<PC3));
    
    // change state according to input
    if (A) { // if A=....1... = true = non zero number
        if (state == 00) {
            state = 01;
            cnt++;
        } else if (state == 10) {
            state = 11;
            cnt--;
        }
    } else { // check if A=00000000
        if (state == 01) {
            state = 00;
            cnt--;
        } else if (state == 11) {
            state = 10;
            cnt++;
        }
    }
    if (B) {
        if (state == 00) {
            state = 10;
            cnt--;
        } else if (state == 01) {
            state = 11;
            cnt++;
        }
    } else {
        if (state == 10) {
            state = 00;
            cnt++;
        } else if (state == 11) {
            state = 01;
            cnt--;
        }
    }
}

