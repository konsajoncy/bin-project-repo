/*
 * main.c
 *
 *  Created on: 12 wrz 2017
 *      Author: Dunajski
 */
// ta atmega sie budzi po impulsie(STROB) drugiej
// po przebudzeniu mija 10 sekund
// a potem znowu idzie spac
// przerwanie na INT0; low lvl

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>


ISR(INT0_vect)
{
	MCUCR&=~(1<<SE);
	_delay_ms(500);
	PORTA^=(1<<PA0);
	_delay_ms(500);
	PORTA^=(1<<PA0);
	_delay_ms(500);
	PORTA^=(1<<PA0);
	PORTA|=(1<<PA0);
	_delay_ms(10000);
	PORTA&=~(1<<PA0);
	asm(" mov r0,r11; ldi r16,(1<<SE) ; out MCUCR, r16 ;sleep ;");  // wstawka do usypiania
}

void sleep_mode_init(void);
void ext_int_init(void);
int main(void)
{
	DDRD&=~(1<<PD2); // INT0 jako wejscie
	PORTD|=(1<<PD2); // pull up internal
	DDRA|=(1<<PA0); //LEd bieda debadzer
	sei();
	ext_int_init();  // inicjalizacja przerwañ
	sleep_mode_init(); // inicjalizacja rodzaju uspienia
	asm(" mov r0,r11; ldi r16,(1<<SE) ; out MCUCR, r16 ;sleep ;");  // wstawka do usypiania

	while(1);

	return 0;
}

void sleep_mode_init(void)
{
	//MCUCR&=~(1<<SM0)|~(1<<SM1)|~(1<<SM2);  // idle mode
	MCUCR|=(1<<SM1);  // power down mode
}

void ext_int_init(void)
{
	MCUCR&=~(1<<ISC00)|~(1<<ISC01); // low level
	//MCUCR|=(1<<ISC00)|(1<<ISC01); //
	GICR|=(1<<INT0); // odblokuj przerwania z zewnatrz
}
