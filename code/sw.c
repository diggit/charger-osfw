#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h> 

#define _ver 0
#define _subver 1

#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clr(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_con(c,p,m) (c ? bit_set(p,m) : bit_clr(p,m)) //macro for conditional bit set/clr
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))

#define NOP asm volatile("NOP")

//LCD library settings
#define dataPort	PORTC
#define dataDir		DDRC

#define commandPort	PORTC
#define commandDir	DDRC

#define D4pin 3
#define D5pin 2
#define D6pin 1
#define D7pin 0

#define RSpin 5 /c/ommand/data
#define Epin  4

#define Xmax 16
#define Ymax 2

#include "hd44780_lib.c"

//globals
uint8_t loop=0;
uint8_t btns_was=15;

char* MENU []={"SETTINGS","DIAGNOSTICS","LiIo","LiPo","NiMh","NiCd","PWR SUPPLY","PWR LOAD"};
char uint8_t []=

volatile uint8_t menu_level=0,menu_item=0;
volatile uint8_t* variable=&menu_item;

void init_ports()
{
	//http://www.rcgroups.com/forums/showpost.php?p=17083296&postcount=3

	//PORTA
	// 0  - IN	ADC0 - filtered, divided measurement of the battery + voltage
	// 1  - IN	ADC1 - filtered, divided measurement of the battery - voltage
	// 2  - IN	ADC2
	// 3  - IN	ADC3
	// 4  - IN	ADC4 - filtered, divided measurement of Vin
	// 5  - OUT	BP3
	// 6  - IN	ADC6 - COM from '4051 analog mux
		// 8 analog inputs selected from ADC6 using PORTB5-7:
		// balancer port measurement?
		// balancer port measurement?
		// balancer port measurement?
		// balancer port measurement?
		// balancer port measurement?
		// balancer port measurement?
		// balancer port measurement?
		// temp sensor
	// 7  - OUT DIScharging safety circuit
	DDRA=0xA0; 
	PORTA=0x80;//block DIScharging


	//PORTB
	// 0  - IN	button 0 (Batt Type),	active LOW
	// 1  - IN	button 1 (Dec),			active LOW
	// 2  - IN	button 2 (Inc),			active LOW
	// 3  - IN	button 3 (Enter),		active LOW
	// 4  - OUT	speaker
	//5~7 - OUT	4051 address pins
	DDRB=0xF0;


	//PORTC
	//0~3 - OUT	LCD, pin order reversed (HD44780, 4-bit mode)
	// 4  - OUT	LCD E pin (clock)
	// 5  - OUT	LCD command/data pin
	// 6  - OUT	BP1
	// 7  - OUT	BP2
	DDRC=0xFF;
	PORTC=0;


	//PORTD
	// 0 - OUT	BP4
	// 1 - OUT	USART out TXD (pin not configured as out)
	// 2 - OUT	BP5
	// 3 - OUT	battery neg. end connection safety circ.
	// 4 - OUT	10-bit PWM OC1B - set power supply voltage for charging
	// 5 - OUT	10-bit PWM OC1A - set voltage for discharging
	// 6 - OUT	charging safety circuit
	// 7 - OUT	BP6
	DDRD=0xFD;
	PORTD=0x48;
}

void init_systick()
{
	//start timer0 to emit imterrupt regularly

	TCNT0=0;//clear timer value
	OCR0=157; //calibration value, 16000000/prescaler/OCR0=frequency of interrupts
	TIMSK|=2;//enable compare int.
	TCCR0=(0x08|0x05); // enable CTC (count to compare), set prescaler to 1024 (this starts counter)
}

void beep()
{
	int i;
	for(i=0;i<100;i++)
	{
		bit_set(PORTB,(1<<4));
		_delay_us(500);
		bit_clr(PORTB,(1<<4));
		_delay_us(500);
	}
	
}
int main (void)
{
	

	int i;
	init_ports();
	sei(); //enable interrupts
	init_systick();

	hd_init();
	hd_set(POWERED);
	hd_clean();

	hd_str("OS FW ver. ");
	hd_num(_ver,2);
	hd_str(".");
	hd_num(_subver,2);
	hd_goto(5,1);
	hd_str("xorly");
	_delay_us(1000000);
	beep();
	hd_clean();

	uint8_t menu_item_was=1;
	
	while(1)
	{
		if(menu_item!=menu_item_was)
		{
			hd_clean();
			hd_str(MENU[menu_item]);
			menu_item_was=menu_item;
			beep();
		}
	}
  	return 0;
}

ISR(TIMER0_COMP_vect)
{
	uint8_t btns_now=(PINB&0x0F);

	loop+=1;
	if (loop>100) //divide by 100
	{
		loop=0;
		//beep=1;
	}

	if (btns_was!=btns_now)//btns changed
	{
		if((PINB&0x0F)!=15)
		{
			//when any key pressed
			switch(PINB&0x0F)
			{
				case (15-1):
					//BTN1
					break;

				case (15-2):
					//BTN2
					*variable-=1;
					//beep();
					break;

				case (15-4):
					//BTN3
					*variable+=1;
					//beep();
					break;

				case (15-8):
					//BTN4
					break;

			}
		}
		btns_was=btns_now;
	}
}