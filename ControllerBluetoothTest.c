#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usart_ATmega1284.h"
#include "timer.h"


uint16_t data = 0x0000; //16 bits of controller data
uint16_t mem  = 0x0000; //16 bits of saved memory
unsigned char button = 0x00;


void SNESController(){
	data = 0x0000;
	//latch controls
	PORTB |= 0x01;
	_delay_us(12);
	PORTB &= 0xFE;
	_delay_us(6);

	//clock and data
	for(int i = 0; i < 16; i++){
		data |= ( (((~PINB) & 0x04)>>2) << i );
		PORTB |= 0x02;
		_delay_us(6);
		PORTB &= 0xFD;
		_delay_us(6);
	}
}

void USARTSEND(uint16_t data, uint16_t mem){
	if (data != mem){
		button = 0x11;
		if (data == 0x0000){
			button = 0x00;
		}
		else if (data == 0x0010){
			//button = 0xAA;
			//button = 0x01;
		}
		else if (data == 0x0020){
			//button = 0x55;
			button = 0x02;
		}
		else if (data == 0x0040){
			//button = 0x99;
			button = 0x03;
		}
		else if (data == 0x0080){
			//button = 0x66;
			button = 0x04;
		}
		if (USART_IsSendReady(0)){
			USART_Send(button, 0);
		}
	}
	mem = data;
}

int main(void)
{
	DDRB = 0x03; PORTB = 0xFC; // B0 and B1 are output and the rest are input
	
	initUSART(0); // initializes USART0(USART will be used on RX0 and TX0
	//USART_Flush(0);
	mem = 0x1111;
	TimerSet(50);
	TimerOn();
	while(1) {
		while(!TimerFlag);
		TimerFlag = 0;
		SNESController();
		USARTSEND(data, mem);
	}
}