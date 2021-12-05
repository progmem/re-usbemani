#pragma once
#include "usbemani.h"

typedef struct OutputList_t OutputList_t;

struct OutputList_t {
  OutputList_t *parent;

  uint16_t *input;
  uint16_t  mask;
  uint16_t  value;
};

void OutputList_Register(uint16_t *input, uint16_t mask, uint16_t value);
uint16_t OutputList_Build(void);

void OutputList_TimeoutAt(uint16_t at);
void OutputList_ResetTimer(void);
void OutputList_TimerTick(void);
uint16_t OutputList_Timer(void);