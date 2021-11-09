#pragma once
#include <stdlib.h>
#include "color.h"

// Base/shared definition
// Provides the following:
// * A provider function to call, returning an HSV color
// * A union of 4 bytes, allowing:
//    * One free byte of padding and an HSV color
//    * Two 16-bit values (useful for providing higher-precision hue)
//    * Four 8-bit values
#define BASE \
  HSV_Color_t (*provider)(void *); \
  union {                 \
    struct {              \
      uint8_t   padding;  \
      HSV_Color_t color;  \
    };                    \
    struct {              \
      uint16_t data16[2]; \
    };                    \
    struct {              \
      uint8_t data8[4];   \
    };                    \
  };

// The basic ColorProvider template
typedef struct {
  BASE
} ColorProvider_t;

// The basic ColorProvider template
typedef struct {
  BASE
  uint8_t delta;  // Amount to cycle by
} ColorProvider_Cycle_t;

// Maps hue to an input
typedef struct {
  BASE
  uint16_t *input;  // Pointer to input
  uint16_t indexer; // Value to multiply input by
} ColorProvider_HueVariable_t;

// Contains a rainbow of color based on the incoming input
typedef struct {
  BASE
  uint16_t *input;     // Pointer to input
  uint16_t  initial;   // When starting a new chain, the value to multiply with
  uint16_t  increment; // When continuing a new chain, the value to add
  uint8_t   quantity;  // How many colors to yield
  uint8_t   pos;       // Which color we're producing
} ColorProvider_RainbowVariable_t;

// Produces an RGB color
RGB_Color_t ColorProvider_Get(ColorProvider_t *self);

ColorProvider_t *ColorProvider_Hue(uint8_t hue);
ColorProvider_t *ColorProvider_HueCycle(uint8_t hue, uint8_t delta);
ColorProvider_t *ColorProvider_HueRandom(void);
ColorProvider_t *ColorProvider_RainbowVariable(uint8_t quantity, uint16_t *input, uint16_t input_max);
