/*
 * main.c
 *
 *  Created on: 23 mar 2017
 *      Author: Wonszu
 */
#include <avr/io.h>
#define F_CPUX 8000000L
#include <util/delay.h>
#define CMP_REGISTER (F_CPUX/(1000L*256))
#include <avr/interrupt.h>

volatile int n=0;

//częstotliwość zegara mikrokotrolera
#define BAUDRATE 115200L //prędkość transmisji w bodach
#define BAUD_REG ((F_CPU/(16*BAUDRATE))-1) //dzielnika cz. UBRR
//---------------------------------------------------------------
void uart_init(void) //Inicjowanie portu UART
{
	UBRRH=(BAUD_REG>>8); //dzielnk czestotliwosci transmisji
	UBRRL=BAUD_REG;
	//Format ramki danych: 8data,1stopbit
	UCSRC=(1<<URSEL)| (3<<UCSZ0);
	UCSRB=(1<<RXEN)| (1<<TXEN); //zezwolenie na odbior i nadawanie
	UCSRB |=(1<<RXCIE); //zezwolenie na przerwanie odb.
}


#define FIFO_L 128 //dlugosc kolejek FIFO
struct {
	unsigned char wi; //indeks odczytu
	unsigned char ri; //indeks zawpisu
	unsigned char buff[FIFO_L];
}
	InputFifo ={0,0}, //kolejeka wejściowa FIFO
	OutputFifo={0,0}; //kolejeka wyjściowa FIFO

ISR(USART_RXC_vect) /*VECTOR(11), USART, RxComplete*/
{
	InputFifo.buff[InputFifo.wi++]=UDR; //umieszczenie danej w kolejce
	if (InputFifo.wi == FIFO_L) InputFifo.wi = 0;
}
/*
 * jo, tu mam taką złotą myśl, że sprawdzacie czy index tablicy dochodzi do wartości, którą ją inicjujecie
 * czyli już o jeden za daleko. W związku z tym (o ile będzie takie miejsce w pamięci) nadpiszecie sobie jakąś
 * inną zmienną. Poza tym, odczytując tę tablicę nie macie pojęcia które dane są najświeższe. To, że w następnej 
 * funkcji odczytujecie po kolei, to nie znaczy, że nie tracicie danych, gdy częstotliwość odbioru jest większa.
 */

//----------------------------------------------------
// zwraca: 0 -gdy bufor odbiornika pusty
// 1 -gdy pobrany znak umieszczony w '*p_dada'
unsigned char GetFromSerial(unsigned char *p_dada)
{
	if (InputFifo.ri == InputFifo.wi) return 0;

	else {
		*p_dada = InputFifo.buff[InputFifo.ri++];
		if (InputFifo.ri == FIFO_L) InputFifo.ri=0;
		return 1;
	}
}

ISR(USART_UDRE_vect) /*VECTOR(12), USART Data Register Empty*/
{
	if (OutputFifo.wi == OutputFifo.ri)
	{
		UCSRB &= ~(1<<UDRIE); //bufor FIFO pusty - wylaczenie przerwania
	}
	else {
		UDR = OutputFifo.buff[OutputFifo.ri++]; //dana z bufora do rejestru UDR
		if (OutputFifo.ri == FIFO_L) OutputFifo.ri = 0;
	}
}
//----------------------------------------------------------------
void PutToSerial(unsigned char data)
{
	OutputFifo.buff[OutputFifo.wi++] = data;

	if (OutputFifo.wi == FIFO_L ) OutputFifo.wi = 0;
	UCSRB |=(1<<UDRIE); // wlaczenie przerwan
}



void StrToSerial(char *msg)
{
	while(*msg!=0) PutToSerial(*msg++);
}



ISR(TIMER0_COMP_vect) //obsluga przerwania od TIMER0
{
	if(PINA & (1<<PA1))
		n++;
	/*if(n>2)
		PORTD|=(1<<PD6);
	if(n<2)
		PORTD&=~(1<<PD6);
		*/
}
//// Przerwanie co 250 us
void InitTimer0(void)
{
	OCR0=CMP_REGISTER -1 ;
	TCCR0= (1<<WGM01) | (1<<CS01) | (1<<CS00); // CTC / PRESC 64
	TIMSK= (1<<OCIE0); //zezwolenie na przerwania
}
void zmierz_zapelnienie(void)
{
	n=0;
	PORTA|=(1<<PA0);
	_delay_us(10);
	PORTA&=~(1<<PA0);
	_delay_ms(1000);
}

void GSM_init(void)
{
	StrToSerial("AT+CREG?");
}

int main(void)
{
	uart_init();
	InitTimer0();
	sei();
	unsigned char dana;
	DDRA|=(1<<PA0); // ultradzwiek TRIG
	DDRA&=~(1<<PA1); // wejscie ultradzwiek ECHO
	PORTA|=(1<<PA1); // pull up ECHO
	DDRD|=(1<<PD6); // LED
	//GSM_init();
	while(1)
	{
		zmierz_zapelnienie();
		_delay_ms(10000);
		GSM_init();
		while( GetFromSerial( &dana))
		{
			if(dana=='K')PORTD|=(1<<PD6);
		}

	}
}

