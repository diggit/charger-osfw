charger-OsFw
============

We (hope not only me) are trying to write opensource firmware for battery pack chargers based on AVR atmega32. 
Those charger are clones or very similar: Turnigy Accucell, imax B6 and many noname ones.

### How to recognize clone?
Those chargers have 16x2 character display, 4 buttons, integrated ballancer for lixx cells, anoying buzzer,...

### Why reinvent wheel?
Well, original firmware is not so bad, but have some limitations. This charger is nice piece of hardware (I mean design, not quality). Original one, has limited calibration count to 1, not official specification of serial data stream and more.
What about conveting your charger into simple power supply, constant current load or just battery monitor?

### How can we do it?
Whole charger is controlled by single, well known MicroCharm,ontroller Unit (MCU) AVR ATmega32. It is:
* 8bit RISC MCU
* running on 16MHz
* 32kiB program FLASH
* 2kiB SRAM
* on chip 10bit ADC
* and many other peripherials (timers, UART, SPI, i2c, WDT, PWM, EEPROM,...)

To program this chip, you only need: AVR toolchain to compile code and link binaries **(avr-gcc)**, hardware programmer (eg. **usbasp** or arduino), software for downloading program to chip flash **(avrdude)**.

### progress?
We are at the beginning. There is lot of work to do. 
That mean, you should not use code here if you are not keen developer and don't understand this code. You can fry your charger easily.

####TODO:
| what														| who			| status
|---------------------------------|---------|--------
| func. to controll display				| diggit	| DONE
| properly initialize harware			| diggit	| IP
| decide, how to implement menu		|					|
| func. to gather ADC vals				|					|
| func to control DC-DC						|					|
| func to manage balancer					|					|
|	 | |
| exhausting testig								| all devs| ...



**You should know**, that we are not responsible for any harm or device lose by following this repository in any way!
