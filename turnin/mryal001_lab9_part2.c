/*	Author: Mayur Ryali
 *  Partner(s) Name:
 *	Lab Section: 21
 *	Assignment: Lab #9  Exercise #2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
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

enum States {Start, push, release, inc, dec, on, off} state;

unsigned char button0; //inc
unsigned char button1; //dec
unsigned char button2; //off

const double notes[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25};
unsigned char i = 0; //index
unsigned char power = 0; //0 means off, 1 means on
unsigned char s = 0; //tracks increase (1), decrease (2), off (3), or on (4)

void Tick() {
	switch(state) {
        	case Start:
            		state = Wait;
            		break;
        	case push: //wait for button push
			if (button0 && power == 1) {
				s = 1;
                		state = release;
            		}
            		else if (button1 && power = 1){
				s = 2;
                		state = release;
            		}
            		else if (button2){
				if (power == 0) {
					s = 4;
                			state = release;
				}
				else {
					s = 3;
                			state = release;
				}
            		}
            		else {
                		state = wait;
           		}
            		break;
		case release:
			if (s == 1 && !button0) {
				state = inc;
			}
			else if (s == 2 && !button1) {
				state = dec;
			}
			else if (s == 3 && !button2) {
				state = off;
			}
			else if (s == 4 && !button2) {
				state = on;
			}
			else {
				state = release;
			}
			break;
        	case inc:
			s = 0;
			state = push;
			break;
		case dec:
			s = 0;
			state = push;
			break;
		case off:
			s = 0;
			power = 0;
			state = push;
			break;
		case on:
			s = 0;
			power = 1;
			state = push;
			break;
        	default:
            		state = Start;
            		break;
	}
    	switch(state) {
        	case Start:
            		break;
        	case push:
            		set_PWM(0);
            		break;
        	case release:
            		set_PWM(261.63);
            		break;
		case inc:
			i++;
			if (i > 7) {
				i = 7;
			}
			set_PWM(notes[i]);
			break;
		case dec:
			i--;
			if (i < 0) {
				i = 0;
			}
			set_PWM(notes[i]);
			break;
       		 case off:
			PWM_off();
			set_PWM(0);
			break;
        	case on:
            		PWM_on();
			set_PWM(notes[i]);
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
	power = 1;

    	while (1) {
        	button0 = ~PINA & 0x01;
        	button1 = ~PINA & 0x02;
        	button2 = ~PINA & 0x04;

        	Tick();
    	}
    	return 1;
}
