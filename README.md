charger-OsFw
============

We (hope not only me) are trying to write opensource firmware for battery pack chargers based on AVR atmega32. 
Those charger are clones or very similar: Turnigy Accucell, imax B6 and many noname ones.

### How to recognize clone?
Those chargers have 16x2 character display, 4 buttons, integrated ballancer for lixx cells, anoying buzzer,...

### Why reinvent wheel?
Well, original firmware is not so bad, but have some limitations. Original FW: can be calibrated only once, has not official specification of serial data stream and more. This charger is nice piece of hardware (I mean design, not quality - sometimes really sucks).
What about conveting your charger into simple power supply, constant current load or just battery monitor?

### How can we do it?
Whole charger is controlled by single, well known MicroController Unit (MCU) AVR ATmega32. It is:
* 8bit RISC MCU
* running at 16MHz
* 32kiB program FLASH
* 2kiB SRAM
* on chip 10bit ADC
* and many other peripherials (timers, UART, SPI, i2c, WDT, PWM, EEPROM,...)

To program this chip, you only need: AVR toolchain to compile code and link binaries **(avr-gcc)**, hardware programmer (eg. **usbasp** or arduino), software for downloading program to chip flash **(avrdude)**.

### progress?
We are at the beginning. There is lot of work to do. 
That mean, you should not use code here if you are not keen developer and don't understand this code. You can fry your charger easily.

####TODO:
| what				  | who 	| status
|---------------------------------|-------------|--------
| func. to controll display	  | diggit	| DONE		|
| properly initialize harware	  | diggit	| DONE		|
| decide, how to implement menu   | diggit	| DONE		|
| func. to gather ADC vals	  | diggit	| DONE		|
| claibration of ADC		  | diggit	| DONE for voltage, current cal. should be improved|
| func to control DC-DC		  | diggit	| DONE		|
| func to manage balance	  | diggit	| waiting	|
| button status reading		  | diggit	| DONE		|
| algorithm for charging	  | diggit	| waiting	|
| power supply alg.		  | diggit	| IP - dummy voltage supply is working	|
| uart output			  | diggit	| waiting	|
|	 ||
| exhausting testig								| all devs| ...

I am moving forward slowly, studying on university is really time consuming.


**You should know**, that we are not responsible for any harm or device lose by following this repository in any way!
