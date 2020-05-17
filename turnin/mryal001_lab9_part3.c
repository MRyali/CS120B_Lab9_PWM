/*	Author: Mayur Ryali
 *  Partner(s) Name:
 *	Lab Section: 21
 *	Assignment: Lab #9  Exercise #3
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

enum States{Start, off, play, wait, repeat, release} state;

unsigned char button0;

const double notes[32] = {523.25, 440.00, 349.23, 523.25, 440.00, 349.23, 523.25, 440.00,
	493.88, 392.00, 329.63,493.88, 392.00, 329.63, 493.88, 392.00,
	493.88, 392.00, 329.63,493.88, 392.00, 329.63, 493.88, 392.00,
	440.00, 349.23, 293.66, 440.00, 349.23, 293.66, 440.00, 349.23 };
unsigned char i = 0;


void Tick(){
	switch(state) {
		case Start:
			state = off;
			break;
		case off:
			if(button0){
				i = 0;
				state = play;
			}
			else
				state= off;
			break;
		case play:
			if (i < 31) {
				i++;
				state = play;
			}
			else {
				state = wait;
			}
			break;
		case wait:
			if (!button0) {
				state = repeat;
			}
			else {
				state = wait;
			}
			break;
		case repeat:
			i = 0;
			if (button0) {
				state = release;
			}
			else {
				state = wait;
			}
			break;
		case release:
			if (!button0) {
				state = play;
			}
			else {
				state = release;
			}
			break;
		default:
			state = Start;
			break;
	}
	switch(state){
		case Start:
			break;
		case off:
			set_PWM(0); //turn off
			break;
		case play:
			set_PWM(notes[i]); //play note
			break;
		case wait:
			set_PWM(0);
			break;
		case repeat:
			set_PWM(0);
			break;
		case release:
			break;
		default:
			break;
	}
}


int main(void) {
    	DDRA = 0x00; PORTA = 0xFF; // input
	DDRB = 0xFF; PORTB = 0x00; // output

	TimerSet(215);
	TimerOn();

   	PWM_on();
	state = Start;

    	while (1) {
        	button0 = ~PINA & 0x01;
        	Tick();

		while(!TimerFlag) {}
		TimerFlag = 0;
    	}
    	return 1;
}
