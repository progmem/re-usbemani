#include "output_list.h"

OutputList_t *output = NULL;
uint16_t timeout_at = 0;
uint16_t timeout;

void OutputList_Register(uint16_t *input, uint16_t mask, uint16_t value) {
  OutputList_t *child = calloc(1, sizeof(OutputList_t));

  child->input  = input;
  child->mask   = mask;
  child->value  = value;
  child->parent = output;
  output = child;
}

uint16_t OutputList_Build(void) {
  uint16_t ret = 0;
  OutputList_t *map = output;

  while(map) {
    if (!map->input || (*map->input & map->mask))
      ret |= map->value;
    map = map->parent;
  }

  return ret;
}

void OutputList_TimeoutAt(uint16_t at) {
  timeout_at = at; }

void OutputList_ResetTimer(void) {
  timeout = timeout_at; }

void OutputList_TimerTick(void) {
  if (timeout) --timeout; }

uint16_t OutputList_Timer(void) {
  return timeout; }