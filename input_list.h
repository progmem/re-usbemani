#pragma once
#include "usbemani.h"

typedef struct InputList_t InputList_t;

struct InputList_t {
  InputList_t *parent;

  uint16_t *input;
  uint16_t  mask;

  union {
    struct {   int8_t *axis;      int8_t axis_value;  };
    struct { uint16_t *buttons; uint16_t button_mask; };
  };
};

void InputList_RegisterAxis(  uint16_t *input, uint16_t mask,   int8_t *dest,   int8_t value);
void InputList_RegisterButton(uint16_t *input, uint16_t mask, uint16_t *dest, uint16_t value);

USBemani_Input_t *InputList_BuildReport(void);