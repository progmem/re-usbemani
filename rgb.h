#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "color.h"

typedef enum {
  RGB_C6 = 0x40,
  RGB_C7 = 0x80,
  RGB_NC = 0xFF,
} RGB_PIN;

void RGB_Init(RGB_PIN pin, uint8_t quantity);
void RGB_Transmit(void);
bool RGB_Ready(void);
void RGB_Sync(void);

void RGB_SetRange(RGB_Color_t color, uint8_t index, uint8_t size);
void RGB_AddRange(RGB_Color_t color, uint8_t index, uint8_t size);

void RGB_ClearRange(uint8_t index, uint8_t size);

void RGB_SetFadeRate(uint8_t rate);
void RGB_FadeRange(uint8_t index, uint8_t size);
void RGB_FadeRangeRandom(uint8_t index, uint8_t size);

void RGB_SetProcessFrame(void (*clear_func));
void RGB_ProcessFrame(void);
uint16_t  RGB_GetTicks(void);
uint16_t *RGB_PtrTicks(void);