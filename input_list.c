#include "input_list.h"
InputList_t *axis = NULL, *button = NULL;
USBemani_Input_t Report;

void InputList_RegisterAxis(uint16_t *input, uint16_t mask, int8_t *dest, int8_t value) {
  InputList_t *child = calloc(1, sizeof(InputList_t));

  child->input      = input;
  child->mask       = mask;

  child->axis       = dest;
  child->axis_value = value;
  child->parent     = axis;
  axis = child;
}

void InputList_RegisterButton(uint16_t *input, uint16_t mask, uint16_t *dest, uint16_t value) {
  InputList_t *child = calloc(1, sizeof(InputList_t));

  child->input       = input;
  child->mask        = mask;

  child->buttons     = dest;
  child->button_mask = value;
  child->parent      = button;
  button = child;
}

USBemani_Input_t *InputList_BuildReport(void) {
  InputList_t *map;

  // Clear report
  memset(&Report, 0, sizeof(USBemani_Input_t));

  Report.Slider  = Input_GetRotaryLogicalPosition(0);
  Report.Dial    = Input_GetRotaryLogicalPosition(1);
  Report.Wheel   = Input_GetRotaryLogicalPosition(2);
  Report.Z       = Input_GetRotaryLogicalPosition(3);
  Report.RZ      = Input_GetRotaryLogicalPosition(4);

  // Axis
  map = axis;
  while(map) {
    if (!map->input || (*map->input & map->mask))
      (*map->axis) = map->axis_value;
    map = map->parent;
  }

  // Buttons
  map = button;
  while(map) {
    if (!map->input || (*map->input & map->mask))
      (*map->buttons) |= map->button_mask;
    map = map->parent;
  }

  return &Report;
}