#pragma once

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>

#include "descriptors.h"
#include <LUFA/Platform/Platform.h>

typedef enum {
  USBEMANI_COMMAND_RESET = 0xF5,
} USBEMANI_COMMAND;

typedef struct {
   int16_t   Wheel;
   int16_t   Z;
   int16_t   RZ;
  uint16_t   Button;
    int8_t   LX;
    int8_t   LY;
    int8_t   RX;
    int8_t   RY;
   int16_t   Slider;
   int16_t   Dial;
} USBemani_Input_t;

typedef struct {
  uint16_t  Lights;
  uint8_t   Command;
  uint8_t   Data;
} USBemani_Output_t;

void SetupHardware(void);
void SetupEffects(void);
void HID_Task(void);

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);
void EVENT_USB_Device_StartOfFrame(void);

void ProcessOutputReport(USBemani_Output_t *Report);
void CreateInputReport(USBemani_Input_t *Report);

void BuildInputReport(USBemani_Input_t *Report);

#include "input.h"
#include "input_list.h"
#include "output_list.h"
#include "color.h"
#include "color_provider.h"
#include "effect.h"
#include "effect_deferer.h"
#include "rgb.h"
#include "ps2.h"
#include "config.h"
