/*	Author: Mayur Ryali
 *  Partner(s) Name:
 *	Lab Section: 21
 *	Assignment: Lab #9  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */

 // Demo: https://drive.google.com/open?id=11QJ_jR-TBbEZ4gJWRhjr5M2b_3eTsGvy

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; } //stops timer/counter
		else { TCCR3B |= 0x03; } // resumes/continues timer/counter

		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR3A = 0xFFFF; }

		// prevents OCR0A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR3A = 0x0000; }

		// set OCR3A based on desired frequency
		else { OCR3A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT3 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	// COM3A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

enum States {Start, wait, c4, d4, e4} state;

unsigned char button0;
unsigned char button1;
unsigned char button2;

void Tick() {
    switch(state) {
        case Start:
            state = wait;
            break;
        case wait: //wait for button push
            if (button0 && !button1 && !button2) {
                state = c4;
            }
            else if (button1 && !button0 && !button2){
                state = d4;
            }
            else if (button2 && !button0 && !button1){
                state = e4;
            }
            else {
                state = wait;
            }
            break;
        case c4:
            if (button0 && !button1 && !button2) { //button is held
                state = c4;
            }
            else { //otherwise wait for new button press
                state = wait;
            }
            break;
        case d4:
            if (button1 && !button0 && !button2) { //button is held
                state = d4;
            }
            else { //otherwise wait for new button press
                state = wait;
            }
            break;
        case e4:
            if (button2 && !button0 && !button1) { //button is held
                state = e4;
            }
            else { //otherwise wait for new button press
                state = wait;
            }
            break;
        default:
            state = Start;
            break;
    }
    switch(state) {
        case Start:
            break;
        case wait:
            set_PWM(0);
            break;
        case c4:
            set_PWM(261.63);
            break;
        case d4:
            set_PWM(293.66);
            break;
        case e4:
            set_PWM(329.63);
            break;
        default:
            break;
    }
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF; // input
	DDRB = 0xFF; PORTB = 0x00; // output

   	PWM_on();
	state = Start;

    	while (1) {
        	button0 = ~PINA & 0x01;
        	button1 = ~PINA & 0x02;
        	button2 = ~PINA & 0x04;

        	Tick();
    	}
    	return 1;
}
