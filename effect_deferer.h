#pragma once
#include "effect.h"

#define Effect_AutoQueue(x) Effect_Defer(x, NULL, 0)

typedef struct Effect_Deferer_t Effect_Deferer_t;
struct Effect_Deferer_t {
  Effect_t *effect; // The effect to draw
  uint16_t *value;  // The value to test
  uint16_t  mask;   // The mask to test the value with
  Effect_Deferer_t *next;
};

void Effect_Defer(Effect_t *effect, uint16_t *value, uint16_t mask);
void Effect_QueueDeferred(void);