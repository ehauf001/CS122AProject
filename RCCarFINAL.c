#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"
#include "timer.h"

unsigned char data = 0x00;

void set_PWM(double frequency){
	static double current_frequency;
	if (frequency != current_frequency){
		if (!frequency) {TCCR3B &= 0x08; }
		else {TCCR3B |= 0x03;}
		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency >31250) {OCR3A = 0x0000; }
		else {OCR3A = (short)(8000000/(128 * frequency)) - 1;}
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off(){
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

void USARTGET(){
	if (USART_HasReceived(0)) {
		data = USART_Receive(0);
		PORTA = data;
		if ((data & 0x10) == 0x10){
			set_PWM(261.63);
		}
		else{
			set_PWM(0);
		}
		USART_Flush(0);
	}
}

int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	initUSART(0); // initializes USART0(USART will be used on RX0 and TX0
	USART_Flush(0);
	PWM_on();
	TimerSet(50);
	TimerOn();
	while(1) {
		while(!TimerFlag);
		TimerFlag = 0;
		USARTGET();
	}
}
