#include <avr/io.h>
#include <string.h>
#include <avr/interrupt.h> 
#include <avr/pgmspace.h>

//versioning numbers & info
#define _ver	0 //every change indicates incompatible change in EEPROM stored data
#define _subver 2
#define ID		"osfw charger"
#define author	"xorly"

#define VENDOR_ID	0x41
#define CHARGER_ID	0xef
#define FW_ID		0x9b

//freq of MCU clocking
#define FOSC 16000000UL

//global macros for bitwise manipulation
#define bit_get(p,m) ((p) & (m))
#define bit_set(p,m) ((p) |= (m))
#define bit_clr(p,m) ((p) &= ~(m))
#define bit_flip(p,m) ((p) ^= (m))
#define bit_con(c,p,m) (c ? bit_set(p,m) : bit_clr(p,m)) //macro for conditional bit set/clr
#define BIT(x) (0x01 << (x))
#define LONGBIT(x) ((unsigned long)0x00000001 << (x))

//simple NOP, belongs to empty loops etc.
#define NOP asm volatile("NOP")

//misc utils lib
#include "misc.c"
//end

//analog measurements lib
#include "analog.c"
//end of ANALOG lib




//UART lib
//TODO: cleanup, comment, shorten, optimize!
#define BAUD 9600UL
#define UART_TX
//#define UART_RX
//#define UA_RX_INT_EN
#include "uart.h"
//end of UART lib


//LCD library settings
#define dataPort	PORTC
#define dataDir		DDRC

#define commandPort	PORTC
#define commandDir	DDRC

#define D4pin 3
#define D5pin 2
#define D6pin 1
#define D7pin 0

#define RSpin 5 //command/data
#define Epin  4

#define Xmax 16
#define Ymax 2
//end of LCD config
#include "hd44780_lib.c"
//end if LCD lib

//STORAGE lib for storing settings in non volatile mem (eeprom)
#include "storage.c"
//end of STORAGE lib

//handlers for menu
int handler_adc_cal();
int handler_supply_voltage();

//menu lib
#include "menu.c"
//end

//contants
#define SAFETY_IDLE			1
#define SAFETY_CONNECT		2
#define SAFETY_CHARGE		3
#define SAFETY_DISCHARGE	4


//buttons
#define BTN_none	0
#define BTN_1		1
#define BTN_2		2
#define BTN_3		3
#define BTN_4		4
#define BTN_ENTER	BTN_4
#define BTN_INC		BTN_3
#define BTN_DEC		BTN_2
#define BTN_CANCEL	BTN_1
volatile uint8_t btn_pressed=BTN_none;

// config
#define pwm_max  800// theoreticaly TODO: set real max value


//globals
uint8_t loop=0;
uint8_t btns_was=15;

void (*slow_periodical_function)();
void (*fast_periodical_function)();

//TODO: something to store global status, eg, power OK, battery polarity OK, and so on

struct {
	unsigned int battery_type:3;
	unsigned int state:2;
	unsigned int error:2;
	unsigned int sampling:1;
} state;

#define ERR_clear		0
#define ERR_badpol		1
#define ERR_conbreak	2

#define STATE_idle		0
#define STATE_discharge	1
#define STATE_charge	2
#define STATE_supply	4

#define BATT_unknown	0
#define BATT_nimh		1
#define BATT_nicd		2
#define BATT_lipo		3
#define BATT_liio		4
#define BATT_acid		5





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
		// 0	neg battery voltage div by 2 :-(
		// 1	balancer port measurement cell 1 (DIV)
		// 2	balancer port measurement cell 2 (DIV)
		// 3	balancer port measurement cell 3 (DIFF AMP)
		// 4	balancer port measurement cell 4 (DIFF AMP)
		// 5	balancer port measurement cell 5 (DIFF AMP)
		// 6	balancer port measurement cell 6 (DIFF AMP)
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
	// 4 - OUT	10-bit PWM OC1B - set power supply voltage for charging (BUCK)
	// 5 - OUT	10-bit PWM OC1A - set voltage for discharging, fot charging (BOOST)
	// 6 - OUT	charging safety circuit
	// 7 - OUT	BP6
	DDRD=0xFD;
	PORTD=0x48;//diarm eerything
}

uint8_t safety_set(uint8_t state)
{
	//active at 0
	//PD3 - connect battery negative wire to ground
	//PD6 - activate charge circuit
	//PA7 - activate discharge circuit
	//status: not tested
	//TODO: schematic check
	//TODO: real test
	switch(state)
	{
		case SAFETY_IDLE:
			PORTA|=0x80; //PA7 to 1
			PORTD|=0x48; //PD6 PD3 to 1
			break;

		case SAFETY_CONNECT:
			PORTA|=0x80; //PA7 to 1
			PORTD&=~0x08; //PD3 to 0
			PORTD|=0x40; //PD6 to 1
			break;

		case SAFETY_CHARGE:
			PORTA|=0x80; //PA7 to 1
			PORTD&=~0x48; //PD6 PD3 to 0
			break;

		case SAFETY_DISCHARGE:
			PORTA&=~0x80; //PA7 to 0
			PORTD&=~0x08; //PD3 to 0
			PORTD|=0x40; //PD6 to 1
			break;

		default:
			return 1; //wrong input param
	}
	return 0;
}

void init_pwm()
{
	//measured original freq 31.25kHz ~ FAST 9bit pwm with 16MHz clock

	//setup
	//TCCR1A=0b10100010;
	TCCR1A=(1<<COM1A1)|(1<<COM1B1)|(1<<WGM11);//clear on compare match (COM1n1) for both, set on BOTTOM; part of 9bit fast config

	//TCCR1B=0b00001000;
	TCCR1B=(1<<WGM12);//part of 9bit fast config

	//set PWM to 0%
	OCR1A=0;
	OCR1B=0;

	//start
	TCCR1B|=(1<<CS10); //no prescaing, enable tick
}


uint16_t pwm_val; //should be read only from outside, write does not do anything
#define get_pwm()	pwm_val
volatile uint8_t set_pwm(uint16_t val)
{
	//2* 9bit gives 1024 levels of settings, keep in mind, that we cant give 100% duty cycle to boost transistor ~ equals shot to ground!
	if(val>pwm_max)
		return 1;// return err, maybe give some beep?
	else
	{
		pwm_val=val;

		if (val>511) //over 9 bits of value
		{
			OCR1A=val-511;//upper bits
			OCR1B=511; //nine bits only
		}
		else
		{
			OCR1A=0;
			OCR1B=val;
		}
	}
	return 0;
}

void init_systick()

{	//start timer0 to emit imterrupt regularly

	TCNT0=0;//clear timer value
	OCR0=157; //calibration value, 16000000/prescaler/OCR0=frequency of interrupts
	TIMSK|=2;//enable compare int.
	TCCR0=(0x08|0x05); // enable CTC (count to compare), set prescaler to 1024 (this starts counter)
}

void beep()
{
	int i;
	for(i=0;i<30;i++)
	{
		bit_set(PORTB,(1<<4));
		_delay_us(500);
		bit_clr(PORTB,(1<<4));
		_delay_us(500);
	}
	
}

#define divider		10 //for better precission and less ripple

void supply_regulation()
{
	int16_t correction=((int16_t)(*val16_p)-(int16_t)adc_measure_raw(ADC_VBATP));
	if (uabs(correction)>divider)
		correction/=divider;
	
	int16_t set=get_pwm()+correction;
	
	if (set<0)
		set=0;
	set_pwm(set);
}

int handler_supply_voltage()
{

	menu_state=MENU_HANDLER;
	uint16_t val=0;
	link16(val);

	hd_clean();
	hd_str("Vsup ");
	safety_set(SAFETY_CHARGE);
	fast_periodical_function=supply_regulation;

	while(btn_pressed!=BTN_CANCEL)
	{
		_delay_us(10000);

		hd_goto(5,0);
		hd_num(measured[MEASURED_ISUP],10,4);

		hd_goto(10,0);
		hd_num(adc_get_cal(val,ADC_CAL[ADC_VBATP]),10,3);

		hd_goto(0,1);
		hd_num(get_pwm(),10,6);

		hd_goto(10,1);
		hd_num(adc_get_cal(measured[MEASURED_VBATP],ADC_CAL[ADC_VBATP]),10,3);

	}
	safety_set(SAFETY_IDLE);
	fast_periodical_function=NULL;
	unlink16();
	return 0;
}

#define edit_blink_period	60
#define cycles_to_default	50

void leave_edit(uint8_t *lister)
{
	hd_goto(15,1);
	hd_str(" ");
	unlink16();
	link8(*lister);
}
int handler_adc_cal()
{
	menu_state=MENU_HANDLER;
	uint8_t i=0;
	uint8_t edit=0;
	uint8_t this_btn=btn_pressed;
	uint8_t prev_btn=this_btn;
	uint16_t original=0;
	uint8_t blink_counter=0;
	uint8_t def_counter=0;
	uint16_t adc_val;

	beep();
	link8(i);

	hd_clean();

	hd_goto(0,0);
	hd_str("IN ** RAWV~REALV");
	hd_goto(0,1);
	hd_str("range(m): CALVA ");

	while(1)//can leave only in non edit mode
	{

		_delay_us(10000);//optional, we dont need to run so fast or regularly

		if (this_btn==BTN_ENTER)
		{
			if (def_counter <= cycles_to_default)
				def_counter++;
			if (def_counter==cycles_to_default) //if held for some time...
			{
				ADC_CAL[i]=pgm_read_word(&ADC_DEF[i]);
				//and because, we probably jumped into edit mode, turn it off
				if (edit)
				{
					edit=0;
					leave_edit(&i);
				}
				beep();
			}
		}
		else
			def_counter=0;
		
		
		
		if(i>=ADC_input_count)
			i=ADC_input_count-1;
		if (prev_btn!=this_btn && this_btn!=BTN_none && i!=5) 
		{
			if (this_btn==BTN_ENTER)
			{
				if(edit)//if ENTER and editing, save value
				{
					edit=0;
					leave_edit(&i);
				}
				else//if ENTER, switch to edit mode;
				{
					original=ADC_CAL[i];
					edit=1;//switch to edit mode
					unlink8();
					link16(ADC_CAL[i]);
				}
			}	
			else if (this_btn == BTN_CANCEL)
			{
				if (edit) //leave edit mode
				{	
					edit=0;
					ADC_CAL[i]=original;
					leave_edit(&i);
				}
				else
					break;
			}

			beep();

		}

		if (edit)
		{
			blink_counter++;
			if(blink_counter==edit_blink_period/2)
			{
				hd_goto(15,1);
				hd_str("E");
			}
			else if (blink_counter==edit_blink_period)
			{
				hd_goto(15,1);
				hd_str(" ");
				blink_counter=0;
			}
		}
		adc_val=adc_measure_raw(i);
		//redraw
		hd_goto(3,0);
		hd_num(i,10,2);
		
		hd_goto(6,0);
		hd_num(adc_val,10,4);

		hd_goto(11,0);
		hd_num(adc_get_cal(adc_val,ADC_CAL[i]),10,5);

		hd_goto(10,1);
		hd_num(ADC_CAL[i],10,5);

		prev_btn=this_btn;
		this_btn=btn_pressed;
		
	}
	s_store_settings();
	unlink8();
	return 0;
}

void draw_menu_entry()
{
	hd_clean();
	hd_str(this_menu_item->text);
	item_changed=0;
	beep();
}

void load_defaults()
{
	int i;
	for(i=0;i<ADC_input_count;i++)
		ADC_CAL[i]=pgm_read_word(&ADC_DEF[i]);
}

int main (void)
{
	uart_init();
	uart_puts(ID);
	uart_puts(" version: ");
	uart_num(_ver,10,1);
	uart_putc('.');
	uart_num(_subver,10,2);
	uart_puts(" is starting...\n");
	
	init_ports();
	adc_init();
	sei(); //enable interrupts
	init_systick();

	hd_init();
	hd_set(POWERED);
	// 4 - OUT	10
	hd_clean();

	hd_str("OS FW ver. ");
	hd_num(_ver,10,1);
	hd_str(".");
	hd_num(_subver,10,1);
	hd_goto(5,1);
	hd_str("xorly");
	_delay_us(1000000);

	init_pwm();
	state.sampling=1;//enable asc sampling
	//set_pwm(100);

	hd_clean();

	s_check_compat();

	if (compatible&0xf0) //if some important filed does not match
	{
		load_defaults();
		// hd_clean();
		// hd_str("rewrite EEPROM?");
		// hd_goto(6,1);
		// hd_num(compatible,16,0);
		// hd_goto(0,1);
		// hd_str("< N");
		// hd_goto(12,1);
		// hd_str("Y >");
		uint8_t btn;
		do
		{
			_delay_us(20000);
			btn=btn_pressed;
			if(btn==BTN_CANCEL)
				break;
			else if(btn==BTN_ENTER)
			{
				beep();
				s_store_id();
				s_store_settings();
				beep();

			}
		}while(btn!=BTN_CANCEL && btn!=BTN_ENTER);
	}
	else
	{
		s_load_settings();
	}
	beep();

	hd_clean();
	menu_init();
	item_changed=1;//first draw
	while(1)
	{
		if(menu_state==MENU_ACTIVE)
		{
			if(item_changed)
			{
				draw_menu_entry();
			}
		}
		else if(menu_state==MENU_EXEC_FUNC)
		{
			this_menu_item->handler_func();
			beep();
			menu_state=MENU_ACTIVE;
			item_changed=1;
		}


	}

	//hd_clean();
}

ISR(TIMER0_COMP_vect)
{
	uint8_t btns_now=(PINB&0x0F);

	loop+=1;
	if (loop>100) //divide by 100
	{


		loop=0;
		//beep();
		if (state.sampling)
		{
			measured[MEASURED_VIN]=adc_measure_raw(ADC_VIN);
			measured[MEASURED_VBATP]=adc_measure_raw(ADC_VBATP);
			measured[MEASURED_VBATN]=adc_measure_raw(ADC_VBATN);
			if(state.battery_type==BATT_liio || state.battery_type==BATT_lipo)
			{
				measured[MEASURED_CELL1]=adc_measure_raw(ADC_CELL1);
				measured[MEASURED_CELL2]=adc_measure_raw(ADC_CELL2);
				measured[MEASURED_CELL3]=adc_measure_raw(ADC_CELL3);
				measured[MEASURED_CELL4]=adc_measure_raw(ADC_CELL4);
				measured[MEASURED_CELL5]=adc_measure_raw(ADC_CELL5);
				measured[MEASURED_CELL6]=adc_measure_raw(ADC_CELL6);
			}
			measured[MEASURED_ISUP]=adc_measure_raw(ADC_ISUP);
			measured[MEASURED_ILOAD]=adc_measure_raw(ADC_ILOAD);
			measured[MEASURED_TEMP]=adc_measure_raw(ADC_TEMP);
			if(measured[MEASURED_VBATP]<measured[MEASURED_VBATN])
			{
				state.error=ERR_badpol;
				uint8_t i;
				for(i=0;i<5;i++)
				{
					beep();
					_delay_us(20000);
				}
			}
			else if (state.error==ERR_badpol)
			{
				state.error=ERR_clear;
			}
		}

		if(slow_periodical_function!=NULL)
		{
			//beep();
			slow_periodical_function();
		}

	}

	if(fast_periodical_function!=NULL)
	{
		//beep();
		fast_periodical_function();
	}

	if (btns_was!=btns_now)//btns changed
	{
		switch(PINB&0x0F)
		{
			case (15-1):
				//BTN1
				btn_pressed=BTN_1;
				if(menu_state==MENU_ACTIVE)
					menu_parent();
				break;

			case (15-2):
				//BTN2
				btn_pressed=BTN_2;
				if(menu_state==MENU_ACTIVE)
					menu_prev();
				else if (menu_state==MENU_HANDLER)
				{
					if (val32_p != NULL && *val32_p>0)
						--*val32_p;
					if (val16_p != NULL && *val16_p>0)
						--*val16_p;
					if (val8_p != NULL && *val8_p > 0)
						--*val8_p;
				}
				//beep();
				break;

			case (15-4):
				//BTN3
				btn_pressed=BTN_3;
				if(menu_state==MENU_ACTIVE)
					menu_next();
				else if (menu_state==MENU_HANDLER)
				{
					if (val32_p != NULL && *val32_p<0xffffffff)
						++*val32_p;
					if (val16_p != NULL && *val16_p<0xffff)
						++*val16_p;
					if (val8_p != NULL && *val8_p < 0xff)
						++*val8_p;
				}
				//beep();
				break;

			case (15-8):
				//BTN4
				btn_pressed=BTN_4;
				if(menu_state==MENU_ACTIVE)
					menu_child();
				break;

			default:
				btn_pressed=BTN_none;

		}
		btns_was=btns_now;
	}
}