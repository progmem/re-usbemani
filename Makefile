MCU          = atmega32u4
ARCH         = AVR8
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = 2
TARGET       = build/usbemani
SRC          = $(TARGET).c descriptors.c $(LUFA_SRC_USB) color.c color_provider.c config.c effect.c effect_deferer.c input.c ps2.c rgb.c
LUFA_PATH    = ./lufa/LUFA
CC_FLAGS     = -DGLOBAL_BRIGHTNESS=128 -DUSE_LUFA_CONFIG_HEADER -Ilufa_config/ -Werror -Wall
LD_FLAGS     =

# Default target
all: build

build:
	mkdir -p build

flash: usbemani.hex
	avrdude -P /dev/tty.usbmodem* -c avr109 -p m32u4 -U flash:w:build/usbemani.hex:i

flash_eep: usbemani.eep
	avrdude -P /dev/tty.usbmodem* -c avr109 -p m32u4 -U eeprom:w:build/usbemani.eep:i


# Include LUFA-specific DMBS extension modules
DMBS_LUFA_PATH ?= $(LUFA_PATH)/Build/LUFA
include $(DMBS_LUFA_PATH)/lufa-sources.mk
include $(DMBS_LUFA_PATH)/lufa-gcc.mk

# Include common DMBS build system modules
DMBS_PATH      ?= $(LUFA_PATH)/Build/DMBS/DMBS
include $(DMBS_PATH)/core.mk
include $(DMBS_PATH)/cppcheck.mk
include $(DMBS_PATH)/doxygen.mk
include $(DMBS_PATH)/dfu.mk
include $(DMBS_PATH)/gcc.mk
include $(DMBS_PATH)/hid.mk
include $(DMBS_PATH)/avrdude.mk
include $(DMBS_PATH)/atprogram.mk
