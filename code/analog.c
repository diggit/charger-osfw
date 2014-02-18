//how many should be value sampled, then averaged
#define ADC_SAMPLES			5

//numbers of analog inputs,(on ADC6 i multiplexor)
#define ADC_VBATP			0
#define ADC_VBATN			1
#define ADC_ILOAD			2
#define ADC_ISUP			3
#define ADC_VIN				4 //note: ADC5 is used as digitial pin...
#define ADC_EXMUX			6
//muxed
#define ADC_VBATN2			ADC_EXMUX			
#define ADC_CELL1			ADC_EXMUX+1
#define ADC_CELL2			ADC_EXMUX+2
#define ADC_CELL3			ADC_EXMUX+3
#define ADC_CELL4			ADC_EXMUX+4
#define ADC_CELL5			ADC_EXMUX+5
#define ADC_CELL6			ADC_EXMUX+5
#define ADC_TEMP			ADC_EXMUX+7

#define ADC_input_count		ADC_EXMUX+8


//milivolts at 1024 (calculated)	
#define ADC_CAL_VBATP		31300 //cca 31V at max
#define ADC_CAL_VBATN		0	
#define ADC_CAL_ILOAD		0	
#define ADC_CAL_ISUP		5700 //cca 5.7A at max measureable (may not, probably wont't survive such current!)
#define ADC_CAL_VIN			18935 //cca 18.9V at input max measureable			

#define ADC_CAL_VBATN2		0	
#define ADC_CAL_CELL1		5595 
#define ADC_CAL_CELL2		5190	
#define ADC_CAL_CELL3		4885	
#define ADC_CAL_CELL4		0//must be measured	
#define ADC_CAL_CELL5		0//must be measured	
#define ADC_CAL_CELL6		0//must be measured	
#define ADC_CAL_TEMP		0//tbd	
//ADC END

//array with calibration values, position equals to constants 
PROGMEM const uint16_t ADC_DEF[ADC_input_count]={
	ADC_CAL_VBATP,
	ADC_CAL_VBATN,
	ADC_CAL_ILOAD,
	ADC_CAL_ISUP,
	ADC_CAL_VIN,
	0, //this is because of skipped input number 5
	ADC_CAL_VBATN2,
	ADC_CAL_CELL1,
	ADC_CAL_CELL2,
	ADC_CAL_CELL3,
	ADC_CAL_CELL4,
	ADC_CAL_CELL5,
	ADC_CAL_CELL6,
	ADC_CAL_TEMP
};

uint16_t ADC_CAL[ADC_input_count]={};


#define MEASURED_VBATP	0
#define MEASURED_VBATN	1
#define MEASURED_ILOAD	2
#define MEASURED_ISUP	3
#define MEASURED_VIN	4

#define MEASURED_CELL1	5
#define MEASURED_CELL2	6
#define MEASURED_CELL3	7
#define MEASURED_CELL4	8
#define MEASURED_CELL5	9
#define MEASURED_CELL6	10
#define MEASURED_TEMP	11

volatile uint16_t measured[12];

//IO defs, where multiplexer is located
#define EXMUX_PORT			PORTB
#define EXMUX				5		//bit 7~5, means shl by 5

//METHODS

//setup adc, one shot, slow, 
void adc_init()
{
	//setup 10bit low sample rate, ext. ref.(def),

	ADMUX=0;//some default

	//enable ACD, 128 prescaler
	ADCSRA=BIT(ADEN)|BIT(ADPS0)|BIT(ADPS1)|BIT(ADPS2);
}

//gel value from ADC, takes input number as arg., returns 0~1023, 1024 means error
uint16_t adc_measure_raw(uint8_t input)
{
	uint16_t adc_raw=0;
	if(input<=ADC_input_count && input !=5 ) //valid input only (ADC5 id digital IO)
	{
		//MUX corret input
		if(input<ADC_EXMUX)//inernal MUX
		{
			ADMUX=input;
		}
		else //external mux, ADC >=ADC_EXMUX
		{
			ADMUX=ADC_EXMUX; //external MUX on input 6
			EXMUX_PORT=(( input- ADC_EXMUX )<<EXMUX);
		}

		//measure, blocking :-(
		uint8_t sample;
		for(sample=0;sample<ADC_SAMPLES;sample++)
		{
			ADCSRA|=(1<<ADSC);//convert
			while(!(ADCSRA & (1<<ADIF)))//wait, until conversion complete
				NOP;

			adc_raw+=ADC;

		}
		adc_raw/=ADC_SAMPLES;
		return adc_raw;
	}
	return 1024;//ERR, wrong input
}

uint16_t adc_get_cal(uint16_t raw , uint16_t calval)
{
	
	return (uint32_t)calval*(uint32_t)raw/1024;
}

//returns calibrated value of input
uint16_t  adc_measure_m(uint8_t input)
{
	return adc_get_cal(adc_measure_raw(input),ADC_CAL[input]);
}

uint16_t  adc_measure_m_buffered(uint8_t position_in_array)
{
	if(position_in_array<=4)
		return adc_get_cal(measured[position_in_array],ADC_CAL[position_in_array]);
	else
		return adc_get_cal(measured[position_in_array],ADC_CAL[position_in_array+2]);
}