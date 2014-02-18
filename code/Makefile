PRG            = sw
OBJ            = sw.o
MCU_TARGET     = atmega32
OPTIMIZE       = -O2

# You should not have to change anything below here.

CC             = avr-gcc

# Override is only needed by avr-lib build system.

override CFLAGS = -g -Wall $(OPTIMIZE) -mmcu=$(MCU_TARGET) $(DEFS)

OBJCOPY        = avr-objcopy
OBJDUMP        = avr-objdump

all: hex

$(PRG).elf: $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o $(PRG).elf $(PRG).hex

hex:  $(PRG).hex

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@
	avr-size -C --mcu=$(MCU_TARGET) $(PRG).elf

install: load

load: $(PRG).hex
	avrdude -p m32 -c usbasp -U flash:w:$< 

