#  +-----------------------------------+
#  | Jetpack Cognition Lab, Inc.       |
#  | Supreme Machines Sensorimotor     |
#  | Author: Matthias Kubisch          |
#  | kubisch@informatik.hu-berlin.de   |
#  | Last Update: January 2021         |
#  +-----------------------------------+


# flasher Settings
PROGTYPE=arduino
PROGPORT=/dev/ttyUSB0
PROGRATE=19200
AVRDUDE=avrdude -c $(PROGTYPE) -P $(PROGPORT) -b $(PROGRATE) -p $(MCU) -v

# supported devices:
# +---------------------+-------------+
# | DEVICE              | MCU         |
# +---------------------+-------------+
# | SENSORIMOTOR_REV1_1 | atmega328p  |
# | SENSORIMOTOR_REV1_2 | atmega328pb |
# +---------------------+-------------+

DEVICE=SENSORIMOTOR_REV1_1
MCU=atmega328p

# clock frequency in Hz
F_CPU=16000000UL


# +-FUSES-------------------------------------------------------------------------+
# |                                                                               |
# |         SUT0-----+ +-----CKSEL3                                               |
# |         SUT1----+| |+----CKSEL2                                               |
# |        CKOUT---+|| ||+---CKSEL1                                               |
# |       CKDIV8--+||| |||+--CKSEL0                                               |
# |               |||| ||||                                                       |
# |  lfuse: 0xCF: 1100.1111                                                       |
# |                                                                               |
# |        CKSEL3..1       = 111 : >8 Mhz external oscillator                     |
# |        CKSEL0, SUT1..0 = 100 : Ceramic Resonator, slowly rising power         |
# |        CKOUT           = 0   : no clock output                                |
# |                                                                               |
# |  hfuse: 0xDD set bootloader size 512 words (start adress 0x3E00), no BOOTRST  |
# |                                                                               |
# |  efuse: 0xFD: xxxx.x101 Brownout detection min:2V5 typ:2V7 max:2V9            |
# |                                                                               |
# |  fuses explained: http://www.ladyada.net/learn/avr/fuses.html                 |
# |  calculator: http://www.engbedded.com/fusecalc                                |
# |  atmega328p defaults: lfuse: 0x62, hfuse: 0xD9, efuse 0x07 (0xFF)             |
# |                                                                               |
# +-------------------------------------------------------------------------------+

LFUSE=0xCF
HFUSE=0xDD
EFUSE=0xFD
# FIXME: EF: 0xFD will not work on atmega328pb

FUSES = -u -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

# Define according to bootloader size settings.
# Note the following is a byte address of the 0x3e00 word address
BOOTADDR=0x7c00

# Bootloader baudrate 1MBAUD, same as sensorimotor default.
BAUDRATE=1000000UL

# compiler settings
CC=avr-gcc
OBJDUMP=avr-objdump
OBJCOPY=avr-objcopy --strip-all
#SIZE=avr-size --format=sysv --mcu=$(MCU)

# bootloader
BOOT=boot
BOOT_HEX=boot.hex
BOOT_OBJ=boot.o
UART_OBJ=uart.o
COM_OBJ=com.o

# example firmware
PRG=main
BIN=main.bin
SRC=main.c
OBJ=$(SRC:%.c=%.o)

OPTIMIZE = -Os
CFLAGS += -Wall -pedantic $(OPTIMIZE) -mmcu=$(MCU) -DF_CPU=$(F_CPU) -DBAUD_RATE=$(BAUDRATE) -D$(DEVICE)
LDFLAGS += -Wl,--section-start=.text=$(BOOTADDR)

# further optimize size if possible
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,-gc-sections  -Wl,--relax


EEPROM=eeprom.hex

.PHONY: all
all: $(BOOT_HEX) $(PRG) $(BIN)
#	$(SIZE) $(BOOT_HEX)

$(BOOT): $(BOOT_OBJ) $(UART_OBJ) $(COM_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(BOOT_HEX): $(BOOT)
	$(OBJCOPY) -j .text -j .data -S -O ihex $^ $@

$(PRG): $(OBJ) $(UART_OBJ) $(COM_OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(BIN): $(PRG)
	$(OBJCOPY) -R .fuse -R .eeprom -R .lock -R .signature -S -O binary $^ $@

.PHONY: fuse
fuse:
	$(AVRDUDE) $(FUSES)

.PHONY: flash
flash: $(BOOT_HEX)
	$(AVRDUDE) -e -U flash:w:$^

.PHONY: test
test:
	$(AVRDUDE) -v

.PHONY: install
install: flash fuse

.PHONY: fusedump
fusedump:
	$(AVRDUDE) -U lfuse:r:-:h -U hfuse:r:-:h -U efuse:r:-:h

.PHONY: flashdump
flashdump:
	$(AVRDUDE) -U flash:r:flash.bin:r

.PHONY: eepromdump
eepromdump:
	$(AVRDUDE) -U eeprom:r:$(EEPROM):i

.PHONY: clean
clean:
	rm -f $(COM_OBJ) $(UART_OBJ) $(BOOT) $(BOOT_HEX) $(BOOT_OBJ) $(BIN) $(PRG) $(OBJ) $(EEPROM)

