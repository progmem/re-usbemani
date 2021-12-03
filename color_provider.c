#include "color_provider.h"
#define CALLER (HSV_Color_t (*)(void *))
#define STRUCT (ColorProvider_t *)

#ifndef GLOBAL_BRIGHTNESS
#define GLOBAL_BRIGHTNESS 255
#endif

RGB_Color_t ColorProvider_Get(ColorProvider_t *self) {
  return HSV_ToRGB(self->provider(self));
}

// Hue: Output a fixed color
HSV_Color_t _call_ColorProvider_Hue(ColorProvider_t *self) {
  return self->color;
}
ColorProvider_t *ColorProvider_Hue(uint8_t hue) {
  ColorProvider_t *result = calloc(1, sizeof(ColorProvider_t));
  result->provider = CALLER _call_ColorProvider_Hue;
  result->color.hue = hue;
  result->color.sat = 255;
  result->color.val = GLOBAL_BRIGHTNESS;
  return STRUCT result;
}

// HueCycle: Output a color, cycling hue with each call
HSV_Color_t _call_ColorProvider_HueCycle(ColorProvider_Cycle_t *self) {
  self->color.hue += self->delta;
  return self->color;
}
ColorProvider_t *ColorProvider_HueCycle(uint8_t hue, uint8_t delta) {
  ColorProvider_Cycle_t *result = calloc(1, sizeof(ColorProvider_Cycle_t));
  result->provider = CALLER _call_ColorProvider_HueCycle;
  result->color.hue = hue - delta; // First call will land on target hue
  result->color.sat = 255;
  result->color.val = GLOBAL_BRIGHTNESS;
  result->delta     = delta;
  return STRUCT result;
}

// HueRandom: Output a random hue with each call.
// WARNING: Use with a one-shot effect to avoid rapid flashing!
HSV_Color_t _call_ColorProvider_HueRandom(ColorProvider_Cycle_t *self) {
  self->data16[0] ^= Util_Random();
  return self->color;
}
ColorProvider_t *ColorProvider_HueRandom(void) {
  ColorProvider_t *result = calloc(1, sizeof(ColorProvider_t));
  result->provider = CALLER _call_ColorProvider_HueRandom;
  result->color.sat = 255;
  result->color.val = GLOBAL_BRIGHTNESS;
  return STRUCT result;
}

// Rainbow: Output a quantity of colors based on an input range
HSV_Color_t _call_ColorProvider_RainbowVariable(ColorProvider_RainbowVariable_t *self) {
  if (--self->pos) {
    self->data16[0] += self->increment;
  } else {
    self->pos = self->quantity;
    self->data16[0] = ((uint16_t)(*self->input) * self->initial);
  }
  return self->color;
}
ColorProvider_t *ColorProvider_RainbowVariable(uint8_t quantity, uint16_t *input, uint16_t input_max) {
  ColorProvider_RainbowVariable_t *result = calloc(1, sizeof(ColorProvider_RainbowVariable_t));
  result->provider = CALLER _call_ColorProvider_RainbowVariable;
  result->color.hue = 0;
  result->color.sat = 255;
  result->color.val = GLOBAL_BRIGHTNESS;

  result->input   = input;
  result->initial = 0xFFFF / input_max;
  result->pos     = 1;

  result->increment = 0xFFFF / quantity;
  result->quantity  = quantity;
  return STRUCT result;
}
