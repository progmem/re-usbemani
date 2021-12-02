#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "color.h"

typedef enum {
  RGB_C6 = 0x00,
  RGB_C7 = 0x01,
  RGB_NC = 0xFF,
} RGB_PIN;

void RGB_Init(RGB_PIN pin, uint8_t quantity);
void RGB_Transmit(void);
bool RGB_Ready(void);
void RGB_Sync(void);

void RGB_SetRange(RGB_Color_t color, uint8_t index, uint8_t size);

void RGB_AddRange(RGB_Color_t color, uint8_t index, uint8_t size);

void RGB_FadeRange(uint8_t rate, uint8_t index, uint8_t size);
void RGB_FadeRangeRandom(uint8_t index, uint8_t size);

void RGB_ClearRange(uint8_t index, uint8_t size);

uint16_t  RGB_GetTicks(void);
uint16_t *RGB_PtrTicks(void);