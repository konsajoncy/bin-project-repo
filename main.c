/*
 * main.c
 *
 *  Created on: 23 mar 2017
 *      Author: Wonszu
 */
#include <avr/io.h>
#define F_CPU 8000000L
#include <util/delay.h>
#define CMP_REGISTER (F_CPU/(1000L*256))
#include <avr/interrupt.h>

int zmienna=0;
unsigned char k[4];
volatile int n=0;
unsigned char LED_TAB[4];
unsigned char LED_N[11];


struct pole_bitowe
{
	volatile	uint8_t :5;
	volatile	unsigned char przyciski:2;
};

#define keyh ((struct pole_bitowe*)&PIND)->przyciski
#define keyh_dir ((struct pole_bitowe*)&DDRD)->przyciski
#define keyh_pullup ((struct pole_bitowe*)&PORTD)->przyciski
volatile unsigned char key=255;


ISR(TIMER0_COMP_vect) //obsluga przerwania od TIMER0
{
	unsigned char tmpD=0;


	if(PINA & (1<<PA1))
		n++;



	switch(zmienna){
		case 0:
				//PORTD=LED_TAB[0];
				tmpD=(unsigned char)~(0x08);
				PORTC=LED_N[k[0]]; //wypisz 2
				zmienna=1;
				break;

		case 1:
				//PORTD=LED_TAB[1];
				tmpD=(unsigned char)~(0x04);
				PORTC=LED_N[k[1]]; //wypisz 0
				zmienna=2;
				break;

		case 2:
				//PORTD=LED_TAB[2];
				tmpD=(unsigned char)~(0x02);
				PORTC=LED_N[k[2]]; //wypisz 1
				zmienna=3;
				break;

		case 3:
				//PORTD=LED_TAB[3];
				tmpD=(unsigned char)~(0x01);
				PORTC=LED_N[k[3]]; //wypisz 7
				zmienna=0;
				break;


	}

	PORTD= (PORTD |0x0F ) & tmpD;

}

void Led_update(int n){

			k[0]=(n%10000-n%1000)/1000;
			k[1]=(n%1000-n%100)/100;
			k[2]=(n%100-n%10)/10;
			k[3]=n%10;
}

void InitTimer0(void)
{
	OCR0=CMP_REGISTER -1 ;
	TCCR0= (1<<WGM01) | (1<<CS01) | (1<<CS00);
	TIMSK= (1<<OCIE0); //zezwolenie na przerwania
}

int main(void)
{
	InitTimer0();
	DDRD=0x8F;
	PORTD=0xEF;
	PORTD=0x6F;

	DDRC=0xFF;
	PORTC=0xFF;

	LED_N[0]=0xC0; //wypisz 0
	LED_N[1]=0xF9; //wypisz 1
	LED_N[2]=0xA4; //wypisz 2
	LED_N[3]=0xB0; //wypisz 3
	LED_N[4]=0x99; //wypisz 4
	LED_N[5]=0x92; //wypisz 5
	LED_N[6]=0x82; //wypisz 6
	LED_N[7]=0xF8; //wypisz 7
	LED_N[8]=0x80; //wypisz 8
	LED_N[9]=0x90; //wypisz 9
	LED_N[10]=0x86; //wypisz error


	sei();
	DDRA|=(1<<PA0);
	DDRA&=~(1<<PA1); // wejscie
	PORTA|=(1<<PA1); // pull up R
	DDRD&=~((1<<PD5)|(1<<PD6)); // wejscie
	PORTD|=(1<<PD5)|(1<<PD6); // pull up R
	DDRD|=(1<<PD7);

	while(1)
	{
		n=0;



			PORTA|=(1<<PA0);
			_delay_us(10);
			PORTA&=~(1<<PA0);
			_delay_ms(1000);




		Led_update(n);
		_delay_ms(1);

		if(n>=9999)
					{
						n=6;
					}
	}
}

