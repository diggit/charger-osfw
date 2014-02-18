/*simple HD44780 library
 * author:	Patrik Bachan
 * contact:	patrikbachan@gmail.com
 * license:	as-is
*/

#ifndef Xmax
# warning "number of characters not defined, using 16 (Xmax)"
# define Xmax 16
#endif

#ifndef Ymax
# warning "number of lines not defined, using 2 (Ymax)"
# define Ymax 2
#endif

#ifndef _NOP
	#define _NOP asm volatile("NOP")
#endif

#ifndef dataPort
# error "dataPort was not defined (eg. PORTB)"
#else
#define DPORT	dataPort
#endif

#ifndef dataDir
# error "dataDir was not defined (eg. DDRB)"
#else
#define DDDR	dataDir
#endif


#ifndef commandPort
# error "comandPort was not defined (eg. PORTC)"
#else
#define CPORT	commandPort
#endif

#ifndef commandDir
# error "commandDir was not defined (eg. DDRC)"
#else
#define CDDR	commandDir
#endif


#if !(defined D4pin && defined D5pin && defined D6pin && defined D7pin)
# error "one or more data pins (D4pin, D5pin, D6pin, D7pin) were not defined (eg 1,2,3,4...)"
#else
#define D4		(1<<D4pin)
#define D5		(1<<D5pin)
#define D6		(1<<D6pin)
#define D7		(1<<D7pin)
#endif

#if !(defined RSpin && Epin)
# error "one or more data pins (D4pin, D5pin, D6pin, D7pin) were not defined (eg 1,2,3,4...)"
#else
#define RS	(1<<RSpin)
#define E	(1<<Epin)
#endif

#include "misc.c"

#define CMDinit	0b00111000
#define CMDset	0b00001111
#define CMDddram	0x80

#define POWERED	4
#define CURSOR	2
#define BLINK	1


volatile uint8_t xpos=0,ypos=0;

void _delay_us(uint32_t delay)//takes 8 cycles, at 16MHz means half of micro second (freq*time)/repeated
{
	delay=delay<<1;//multiply by 2 to get 1uS delay
	while(delay--) {_NOP;_NOP;}
};//dont try to optimize, it's calibrated! (gcc -O2)


void tick(uint16_t delay)
{
	CPORT|=E;
	_delay_us(delay);
	CPORT&=~E;
	_delay_us(delay);
}

void transmit(uint8_t data)
{	
	int8_t i;
	for(i=4;i>=0;i-=4)
	{
		DPORT&=~(D4|D5|D6|D7);//vycisti port
		if((data>>i)&1)
			DPORT|=D4;
		if((data>>i)&2)
			DPORT|=D5;
		if((data>>i)&4)
			DPORT|=D6;
		if((data>>i)&8)
			DPORT|=D7;
		tick(1);
		_delay_us(50);//maybe remove???
	}
	DPORT&=~(D4|D5|D6|D7);//vycisti port
}

void hd_clean()
{
	transmit(0x01);
	_delay_us(1700);
	xpos=0;
	ypos=0;
}

#define hd_num(V,B,M) hd_str(itoa(V,B,M))


void hd_goto(uint8_t X,uint8_t Y)
{
	if  (!(X<Xmax && Y<Ymax))//check for valid coordinates
	return;
	if(Y==0)
		transmit(CMDddram|X);
	else if(Y==1)
		transmit(CMDddram|0x40|X);
	xpos=X;
	ypos=Y;
}

void hd_init()
{
	DDDR|=(D4|D5|D6|D7);
	CDDR|=(RS|E);
	
	DPORT&=~(D4|D5|D6|D7);//vycisti port
	DPORT|=(D4|D5);//posle se cislo 3 (cast CMDinit)
	tick(1);
	_delay_us(50);
	
	transmit(CMDinit);
	//transmit(CMDset);
	transmit(2);
	_delay_us(2000);
	//transmit(0x80|0x40);//to dram
	//bit_set(CPORT,RS);
	//transmit('R');
	//DPORT&=~(D4|D5|D6|D7);//vycisti port
	//bit_clr(CPORT,RS);
	hd_goto(0,0);
	
}

void hd_set(uint8_t state)
{
	transmit(0x08|(state&7));
}


void hd_newline()
{
	ypos++;
	if(!(ypos<Ymax))
		ypos=0;
	hd_goto(0,ypos);
	//hd_clean();
	//hd_num(ypos,10,0);

}


void hd_write(uint8_t C)
{
	if(!(xpos<Xmax))//newline
	{
		hd_newline();
	}
	CPORT|=RS;
	transmit(C);
	CPORT&=~RS;
}

void hd_str(char *str)
{
	for(; *str != '\0'; str++)// while character on given address is not EQ \0, send it, increase address and iterate!
	{
		if  (*str == '\n')
			hd_newline();
		else
			hd_write(*str);//send character which is located on address called str here, *str gives valu (char) stored there
	}
}

