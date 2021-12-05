#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>

typedef enum {
  PIN_B0, // 0
  PIN_B1,
  PIN_B2,
  PIN_B3,
  PIN_B4,
  PIN_B5,
  PIN_B6,
  PIN_B7,
  PIN_D0, // 0x08
  PIN_D1,
  PIN_D2,
  PIN_D3,
  PIN_D4,
  PIN_D5,
  PIN_D6,
  PIN_D7,
  PIN_F0, // 0x10
  PIN_F1,
  PIN_F2,
  PIN_F3,
  PIN_F4,
  PIN_F5,
  PIN_F6,
  PIN_F7,
  OUT_INV   = 0x40, // Invert data, for EEPROM config packing
  OUT_LATCH = 0x80, // Output pin as latch, for EEPROM config packing
  PIN_NC  = 0xFF,
} INPUT_PIN_INDEX;

typedef enum {
  ANALOG_F0,
  ANALOG_F1,
  ANALOG_F4 = 0x04,
  ANALOG_F5,
  ANALOG_F6,
  ANALOG_F7,
  ANALOG_D4 = 0x20,
  ANALOG_D6,
  ANALOG_D7,
  ANALOG_B4,
  ANALOG_B5,
  ANALOG_B6,
  ANALOG_INV  = 0x80, // Invert, for EEPROM config packing
  ANALOG_NC   = 0xFF,
} ANALOG_PIN_INDEX;

typedef enum {
  ANALOG_NORMAL,
  ANALOG_INVERT = 0x80,
} ANALOG_SHOULD_INVERT;

typedef enum {
  INPUT_FREQ_1KHZ  = 249,
  INPUT_FREQ_2KHZ  = 124,
  INPUT_FREQ_4KHZ  = 62,
  INPUT_FREQ_8KHZ  = 31,
  INPUT_FREQ_16KHZ = 15,
} INPUT_FREQUENCY;

typedef enum {
  INPUT_ROTARY_CW   = 0x0010,
  INPUT_ROTARY_CCW  = 0x0020,
  INPUT_NULL        = 0xFFFF,
} INPUT_ROTARY_DIRECTION;


typedef struct {
  uint8_t                 group[2];
  uint8_t                 mask[2];
  uint8_t                 state;
  INPUT_ROTARY_DIRECTION  direction;
  uint16_t                position;
  uint16_t                max_position;
  uint16_t                position16;
  uint16_t                increment16;
  uint16_t                hold;
  uint16_t                max_hold;
} Input_Rotary_t;

typedef struct {
  uint8_t         active;
  Input_Rotary_t  encoders[5];
} Input_Encoders_t;

typedef struct {
  uint8_t   active;
  uint16_t  data;
  uint8_t   group[16];
  uint8_t   mask[16];
} Input_Buttons_t;

typedef struct {
  uint8_t   active;
  uint8_t   raw[12]; // Raw reads
  uint16_t  value[12]; // Converted reads
  uint16_t  invert;
  uint8_t   trigger[12];
  uint8_t   release[12];
  ANALOG_PIN_INDEX mask[12];
  uint16_t digital;
} Input_Analog_t;

typedef struct {
  uint8_t   active;
  uint16_t  data;
  uint8_t   group[16];
  uint8_t   mask[16];
  uint8_t   latch_group;
  uint8_t   latch_mask;
  uint8_t   precalc_ddr[3];
  uint8_t   precalc_low[3];
  uint8_t   precalc_high[3];
  uint16_t  invert;
} Output_Pins_t;

void Input_RegisterButton(INPUT_PIN_INDEX pin);
void Input_RegisterAnalog(ANALOG_PIN_INDEX pin, ANALOG_SHOULD_INVERT invert);
void Input_RegisterRotary(INPUT_PIN_INDEX pin1, INPUT_PIN_INDEX pin2, uint16_t ppr, uint16_t hold);

void Output_RegisterPin(INPUT_PIN_INDEX pin);
void Output_RegisterLatch(INPUT_PIN_INDEX pin);

void InputOutput_Begin(INPUT_FREQUENCY freq);
void Input_Task(void);
void Analog_Task(void);
void Rotary_Task(void);
void Output_Task(void);

uint16_t Input_Ticks(uint8_t index);
uint16_t Input_GetButtons(void);
uint16_t Input_GetRotaryPhysicalPosition(uint8_t index);
uint16_t Input_GetRotaryLogicalPosition(uint8_t index);
uint16_t Input_GetRotaryMaximum(uint8_t index);
uint16_t Input_GetRotaryDirection(uint8_t index);
uint16_t Input_GetAnalog(uint8_t index);
uint16_t Input_GetAnalogDigital(void);

void Input_RotaryLogicalTarget(uint8_t index, uint16_t logical_max, uint16_t logical_per_rotation);
void Input_RotaryHold(uint8_t index, uint16_t hold);

void Input_AnalogDigitalThresholds(uint8_t index, uint8_t trigger, uint8_t release);

uint16_t*Input_PtrButtons(void);
uint16_t*Input_PtrRotaryPhysicalPosition(uint8_t index);
uint16_t*Input_PtrRotaryLogicalPosition(uint8_t index);
uint16_t*Input_PtrRotaryDirection(uint8_t index);
uint16_t*Input_PtrAnalog(uint8_t index);
uint16_t*Input_PtrAnalogDigital(void);

uint8_t  Input_CountButtons(void);
uint8_t  Input_CountRotary(void);
uint8_t  Input_CountAnalog(void);

uint16_t Output_Get(void);
void     Output_Set(uint16_t data);
uint16_t*Output_Ptr(void);
__attribute__((weak)) void Input_ExecuteOnInterrupt(void);

/*
IO_Init

IOPointer_Button
IOPointer_RotaryPositionPhysical
IOPointer_RotaryPositionLogical
IOPointer_RotaryDirection
IOPointer_Output

Button_Add
Button_Get

Rotary_Add
Rotary_ConfigureLogical
Rotary_Direction
Rotary_PositionPhysical
Rotary_PositionLogical

Output_Get
Output_Set
*/
