#pragma once
#include <stdlib.h>
#include <stdint.h>

typedef struct {
  union { uint8_t g, green; };
  union { uint8_t r, red; };
  union { uint8_t b, blue; };
} RGB_Color_t;

typedef struct {
  union { uint8_t h, hue; };
  union { uint8_t s, sat, saturation; };
  union { uint8_t v, val, value; };
} HSV_Color_t;

RGB_Color_t HSV_ToRGB(HSV_Color_t hsv);

void HSV_Brighten(HSV_Color_t *color, uint8_t value);
void HSV_Dim(HSV_Color_t *color, uint8_t value);

uint16_t Util_Random(void);