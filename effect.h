#pragma once
#include <stdlib.h>
#include <stdbool.h>
#include "color.h"
#include "color_provider.h"
#include "rgb.h"

typedef struct Effect_t Effect_t;

struct Effect_t {
  void (*draw)(Effect_t *);   // Drawing handler, accepts itself
  ColorProvider_t *provider;  // Color provider
  uint8_t index;              // First LED to draw
  uint8_t size;               // How many LEDs to draw
  union {                     // Data cache, subject to clearing!
    struct { uint16_t cache;     };
    struct { uint8_t  cache8[2]; };
  };
  Effect_t *next;             // Effect to draw after this one
};

// Add effect to the front of the queue
void Effect_Shift(Effect_t *effect);
// Add effect to the back of the queue
void Effect_Push( Effect_t *effect);
// Run through all queued effects
void Effect_Run(void);

// Call the color provider once per call
Effect_t *Effect_Single(ColorProvider_t *color, uint8_t index, uint8_t size);
// Call the color provider once per pixel
Effect_t *Effect_Multi(ColorProvider_t *color, uint8_t index, uint8_t size);
// Call the color provider once. Uses the called color until released.
Effect_t *Effect_Press(ColorProvider_t *color, uint8_t index, uint8_t size);
// Call the color provider once. Draws exactly once until reset.
Effect_t *Effect_SingleShot(ColorProvider_t *color, uint8_t index, uint8_t size);
// Call the color provider once. Spawns an effect on press. Lights on hold.
Effect_t *Effect_Splash(ColorProvider_t *color, uint8_t index, uint8_t size);
