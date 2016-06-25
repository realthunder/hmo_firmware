ARDUINO_LIBS = EEPROM Servo SPI FrequencyTimer2
OTHER_LIBS = bitlash
ARDUINO_SKETCHBOOK = ../
EXTRA_FLAGS = -DIR_RAWBUF=256 -DBITLASH_INTERNAL -DBITLASH_PROMPT='">\r\n"' -DBITLASH_TXEN=bitlash_txen \
			  -DSHELL_REBOOT -DSHELL_EEPROM
ARDUINO_DIR=../../ide

ISP_PROG = stk500v1
ISP_PORT = /dev/ttyUSB0
AVRDUDE_ISP_BAUDRATE = 19200

ifeq ($(board),)

USER_LIBS = $(OTHER_LIBS)
OPTIONS = $(EXTRA_FLAGS) -DLAYOUT_US_ENGLISH -DUSB_SERIAL -DSHELL_BAUDRATE=384000

include ../Teensy.mk

else

ARDUINO_LIBS += $(OTHER_LIBS)
EXTRA_FLAGS += -DTINY_BUILD -DUSER_FUNCTIONS

ifeq ($(board),pro2)
    override board = pro5v328
	EXTRA_FLAGS += -DBOARD_PRO2
endif
ifeq ($(board),nano3)
    override board = nano328
endif
BOARD_TAG = $(board)

include ../Arduino-Makefile/Arduino.mk

endif
