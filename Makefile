MCU          = atmega32u4
ARCH         = AVR8
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = 2
TARGET       = build/usbemani
SRC          = $(TARGET).c descriptors.c $(LUFA_SRC_USB) rgb.c color.c color_provider.c effect.c effect_deferer.c input.c ps2.c
LUFA_PATH    = ./lufa/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -Ilufa_config/ -Werror -Wall
LD_FLAGS     =

# RGB data is available on pin C7.
# PS2 acknowledgement is available on pin C6.
# Uncomment this line if you need to swap (RGB/C6, PS2/C7).
# CC_FLAGS    := $(CC_FLAGS) -DUSBEMANI_C6_RGB

# Default target
all: build

build:
	mkdir -p build

flash: usbemani.hex
	avrdude -P /dev/tty.usbmodem* -c avr109 -p m32u4 -U flash:w:build/usbemani.hex:i

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
