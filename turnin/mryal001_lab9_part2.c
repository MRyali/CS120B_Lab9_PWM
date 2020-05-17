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
#include "timer.h"
#include "io.h"
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

enum States {Start, push, inc, incRelease, dec, decRelease, on, off, powerOff} state;

unsigned char button0; //inc
unsigned char button1; //dec
unsigned char button2; //off

const double notes[8] = {261.63, 293.66, 329.63, 349.23, 392.00, 440.00, 493.88, 523.25}; //array of frequencies
unsigned char i = 0; //index for array

void Tick() {
	switch(state) {
        	case Start:
            		state = off;
            		break;
        	case push: //wait for button push
			if (button0) { //increase
				if (i < 7) {
					i++;
				}
				state = inc;
            		}
            		else if (button1){ //decrease
				if (i > 0) {
					i--;
				}
                		state = dec;
            		}
            		else if (button2){ //turn off
				state = powerOff;
            		}
            		else {
                		state = push;
           		}
            		break;
        	case inc:
			/*
			if (i < 7) { //next frequency
				i++;
			}*/
			state = incRelease;
			break;
		case incRelease:
			if (!button0) {
				state = push;
			}
			else {
				state = incRelease;
			}
			break;
		case dec:
			/*
			if (i > 0) {
				i--;
			}*/
			state = decRelease;
			break;
		case decRelease:
			if (!button1) {
				state = push;
			}
			else {
				state = decRelease;
			}
			break;
		case off:
			if (button2) {
				state = on;
			}
			else {
				state = off;
			}
			break;
		case on:
			if (!button2) {
				state = push;
			}
			else {
				state = on;
			}
			break;
		case powerOff: 
			if (!button2) {
				state = off;
			}
			else {
				state = powerOff;
			}
			break;
        	default:
            		state = Start;
            		break;
	}
    	switch(state) {
        	case Start:
            		break;
        	case push:
            		break;
		case inc:
			set_PWM(notes[i]);
			break;
		case incRelease:
			break;
		case dec:
			set_PWM(notes[i]);
			break;
		case decRelease:
			break;
       		case off:
			PWM_off();
			break;
		case powerOff:
			break;
        	case on:
            		PWM_on();
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
	i = 0;

    	while (1) {
        	button0 = ~PINA & 0x01;
        	button1 = ~PINA & 0x02;
        	button2 = ~PINA & 0x04;

        	Tick();
    	}
    	return 1;
}
