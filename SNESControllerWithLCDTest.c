#define F_CPU 8000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"


volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
uint16_t data = 0x0000; //16 bits of controller data
uint16_t mem  = 0x0000; //16 bits of saved memory

// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks


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

void TimerOn() {
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s

	// AVR output compare register OCR1A.
	OCR1A = 125;	// Timer interrupt will be generated when TCNT1==OCR1A
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt

	//Initialize avr counter
	TCNT1=0;

	_avr_timer_cntcurr = _avr_timer_M;
	// TimerISR will be called every _avr_timer_cntcurr milliseconds

	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff() {
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR() {
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect) {
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	if (_avr_timer_cntcurr == 0) { // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

void displayButton(uint16_t data, uint16_t memory){
	if (data == memory){
		
	}
	else if ((data & 0x0001) == 0x0001){
		LCD_ClearScreen();
		LCD_DisplayString(1,"B");
	}
	else if ((data & 0x0002) == 0x0002){
		LCD_ClearScreen();
		LCD_DisplayString(1,"Y");
	}
	else if (data == 0x0004){
		LCD_ClearScreen();
		LCD_DisplayString(1,"SELECT");
	}
	else if (data == 0x0008){
		LCD_ClearScreen();
		LCD_DisplayString(1,"START");
	}
	else if ((data & 0x0010) == 0x0010){
		LCD_ClearScreen();
		LCD_DisplayString(1,"UP");
	}
	else if (data == 0x0020){
		LCD_ClearScreen();
		LCD_DisplayString(1,"DOWN");
	}
	else if (data == 0x0040){
		LCD_ClearScreen();
		LCD_DisplayString(1,"LEFT");
	}
	else if (data == 0x0080){
		LCD_ClearScreen();
		LCD_DisplayString(1,"RIGHT");
	}
	else if (data == 0x0100){
		LCD_ClearScreen();
		LCD_DisplayString(1,"A");
	}
	else if (data == 0x0200){
		LCD_ClearScreen();
		LCD_DisplayString(1,"X");
	}
	else if (data == 0x0400){
		LCD_ClearScreen();
		LCD_DisplayString(1,"L");
	}
	else if (data == 0x0800){
		LCD_ClearScreen();
		LCD_DisplayString(1,"R");
	}
	memory = data;
}

int main(void)
{
	DDRD = 0xFF; PORTD = 0x00; // LCD control lines
	DDRC = 0xFF; PORTC = 0x00; // LCD data lines
	DDRB = 0x03; PORTB = 0xFC; // B0 and B1 are output and the rest are input
	LCD_init();
	mem = 0x1111;
	TimerSet(100);
	TimerOn();
	while(1) {
		while(!TimerFlag);
		TimerFlag = 0;
		SNESController();
		displayButton(data, mem);
	}
}