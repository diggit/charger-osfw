#ifndef __STORAGE_LIB
	
	#include <avr/eeprom.h>	
	
	//address defines
	//identification
	#define EADDR8_FW_ID			(uint8_t*)0
	#define EADDR8_VENDOR_ID		(uint8_t*)1
	#define EADDR8_CHARGER_ID		(uint8_t*)2
	#define EADDR8_VER				(uint8_t*)3
	#define EADDR8_SUBVER			(uint8_t*)4
	//#define EADDR8_			(uint8_t*)5	

	#define EADDR16_ADC_CAL			(uint16_t*)6//13 * 16b
	//#define EADDR8_
	
	#define s_load_array(A,S,B) eeprom_read_block(A,S,B)
	#define s_update_array(A,D,B) eeprom_write_block(A,D,B)
	
	uint8_t compatible=0;

	uint8_t s_check_compat()
	{
		uint8_t flag=1;

		//most inportant values
		if(eeprom_read_byte(EADDR8_FW_ID)!=FW_ID)//must match!
		{
			flag|=0x80;
			hd_num(eeprom_read_byte(EADDR8_FW_ID),16,2);
			hd_num(FW_ID,16,3);
		}
		
		if(eeprom_read_byte(EADDR8_VENDOR_ID)!=VENDOR_ID)
		{
			flag|=0x40;
			hd_goto(0,1);
			hd_num(eeprom_read_byte(EADDR8_VENDOR_ID),16,2);
			hd_num(VENDOR_ID,16,3);
		}

		if(eeprom_read_byte(EADDR8_CHARGER_ID)!=CHARGER_ID)
			flag|=0x20;

		//when does not match, caligration values should NOT be used from eeprom
		if(eeprom_read_byte(EADDR8_VER)!=_ver)
			flag|=0x10;

		//less important values
		if(eeprom_read_byte(EADDR8_SUBVER)!=_subver)//just improved version
			flag|=0x08;

		//if(eeprom_read_byte()!=)
		compatible=flag|0x01;
		return flag;
	}

	void s_store_id()
	{
		eeprom_write_byte(EADDR8_FW_ID,FW_ID);
		eeprom_write_byte(EADDR8_VENDOR_ID,VENDOR_ID);
		eeprom_write_byte(EADDR8_CHARGER_ID,CHARGER_ID);
		eeprom_write_byte(EADDR8_VER,_ver);
		eeprom_write_byte(EADDR8_SUBVER,_subver);
	}

	void s_store_settings()
	{
		//ADC_CAL calibration constants of adc
		s_update_array(ADC_CAL,EADDR16_ADC_CAL,(ADC_input_count-1)*2);
	}

	void s_load_settings()
	{
		s_load_array(ADC_CAL,EADDR16_ADC_CAL,(ADC_input_count-1)*2);
	}

	void s_store_all()
	{

	}

	#define __STORAGE_LIB
#endif