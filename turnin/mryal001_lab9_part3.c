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
#endif

// 0.954 hz is lowest frequency possible with this function,
// based on settings in PWM_on()
// Passing in 0 as the frequency will stop the speaker from generating sound
void set_PWM(double frequency) {
	static double current_frequency; // Keeps track of the currently set frequency
	// Will only update the registers when the frequency changes, otherwise allows
	// music to play uninterrupted.
	if (frequency != current_frequency) {
		if (!frequency) { TCCR0B &= 0x08; } //stops timer/counter
		else { TCCR0B |= 0x03; } // resumes/continues timer/counter

		// prevents OCR3A from overflowing, using prescaler 64
		// 0.954 is smallest frequency that will not result in overflow
		if (frequency < 0.954) { OCR0A = 0xFFFF; }

		// prevents OCR0A from underflowing, using prescaler 64					// 31250 is largest frequency that will not result in underflow
		else if (frequency > 31250) { OCR0A = 0x0000; }

		// set OCR3A based on desired frequency
		else { OCR0A = (short)(8000000 / (128 * frequency)) - 1; }

		TCNT0 = 0; // resets counter
		current_frequency = frequency; // Updates the current frequency
	}
}

void PWM_on() {
	TCCR0A = (1 << COM0A0 | 1 << WGM00);
	// COM3A0: Toggle PB3 on compare match between counter and OCR0A
	TCCR0B = (1 << WGM02) | (1 << CS01) | (1 << CS00);
	// WGM02: When counter (TCNT0) matches OCR0A, reset counter
	// CS01 & CS30: Set a prescaler of 64
	set_PWM(0);
}

void PWM_off() {
	TCCR0A = 0x00;
	TCCR0B = 0x00;
}

enum States{Start, off, play} state;

unsigned char button0;

const double notes[26] = {329.63, 329.63, 329.63, 329.63, 329.63, 329.63, 329.63, 392.00, 261.63, 293.66, 329.63, 349.23, 349.23, 349.23, 349.23, 349.23, 329.63, 329.63, 329.63, 329.63, 329.63, 293.66, 293.66, 329.63, 293.66, 392.00 };
unsigned char i = 0x00;
unsigned char j = 0x00;

void button_Tick(){
	buttonPress = ~PINA & 0x01;
	switch(state){ // Transitions
		case off:
			if(button0){
				i = 0;
				j = 0;
				state = play;
			}
			else
				state= off;
			break;
		case play:
			if(j < 2){
				state = play;
			}
			else{
				state = off;
			}
			if(i > 25){
				j++;
			}
			i++;
			break;
	}
	switch(state){ // State actions
		case off:
			set_PWM(0); //turn off
			break;
		case play:
			set_PWM(notes[i]); //play note
			set_PWM(0); //end laying note
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
        Tick();
    }
    return 1;
}
