#ifndef __UART_LIB  //not yet included

	#ifndef FOSC
		#define FOSC 16000000UL
	#endif

	#ifndef BAUD
		#define BAUD 9600UL
	#endif

	#define ubrr_val (FOSC/(16*BAUD)-1)



	#if (defined UART_RX || defined UART_TX)
		#ifdef UART_RX
			unsigned char uart_getc( void );
			void uart_flush(void);
			#ifndef UA_RX_INT_EN
				#define UA_RX_INT_EN 0
			#endif
		#endif

		#ifdef UART_TX
			void uart_putc(char data );
			void uart_puts(char *str);
			//void uart_num(int32_t num,uint8_t base, uint8_t min_length);
			#include "misc.c" //toolf for number conversion to array of chars
			#define uart_num(V,B,M) uart_puts(itoa(V,B,M))
		#endif
	#else
		#error "not RX nor TX enabled! Do not import or define at least one of: UART_RX, UART_RX."
	#endif
	
	void uart_init(void);

	#include "uart.c"

	#define __UART_LIB
#endif
