#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"
#include "timer.h"

unsigned char data = 0x00;
unsigned char memory = 0x00;

void USARTGET(){
	if (USART_HasReceived(0)) {
		data = USART_Receive(0);
		PORTB = data;
		//USART_Flush(0);
	}
}


int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0xFF; PORTB = 0x00;
	initUSART(0); // initializes USART0(USART will be used on RX0 and TX0
	USART_Flush(0);
	memory = 0x11;
	TimerSet(50);
	TimerOn();
	while(1) {
		PORTA = 0xAA;
		while(!TimerFlag);
		TimerFlag = 0;
		USARTGET();
	}
}