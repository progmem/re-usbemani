#include "ps2.h"

// Stores a constructed packet for the PS2.
uint16_t Data = 0;
// When set, ignore all data until chip select goes high again
// Shoutouts to @nicolasnoble for the insight on how the PS1 handles chip select
uint8_t QuietTime = 0;
// List of available PS2 inputs.
PS2_InputList_t *PS2Input = NULL;
// Current PS2 state.
void (*PS2Handler)(uint8_t) = NULL;



void PS2_Acknowledge(void) {
  // Burn a few cycles before acknowledging
  asm volatile(
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
  );
  DDRC  |=  0x40;
  // 40 cycles of delay should give us the same delay as a real PS1 controller
  asm volatile(
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
    "nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n"
  );
  DDRC  &= ~0x40;
}

void PS2_Listen(uint8_t in);
void PS2_Addressed(uint8_t in);
void PS2_HeaderFinished(uint8_t in);
void PS2_LowerSent(uint8_t in);

// Idle state.
void PS2_Listen(uint8_t in) {
  // Report as a digital controller when addressed
  if (in == 0x01) {
    PS2_Acknowledge();
    SPDR = ~(0x41);
    PS2Handler = PS2_Addressed;
    return;
  // Otherwise, ignore all incoming traffic until our task performs a reset
  } else {
    DDRB &= ~0x08;
    QuietTime = 1;
  }
}

// When polling is requested, begin responding
void PS2_Addressed(uint8_t in) {
  if (in == 0x42) {
    SPDR = ~(0x5A);
    PS2Handler = PS2_HeaderFinished;
    PS2_Acknowledge();
  }
}

// After end-of-header sent, send the first byte
void PS2_HeaderFinished(uint8_t in) {
  uint8_t *data = (uint8_t *)&Data;

  SPDR = ~(*data);
  PS2Handler = PS2_LowerSent;
  PS2_Acknowledge();
}

// After first byte sent, send the second and go back to listening.
void PS2_LowerSent(uint8_t in) {
  uint8_t *data = (uint8_t *)&Data + 1;

  SPDR = ~(*data);
  PS2Handler = PS2_Listen;
  PS2_Acknowledge();
}

void PS2_Init(void) {
  cli();

  PORTC &= ~0x40;
  PS2_Acknowledge();
  // Set MISO as an output pin
  DDRB |= 0x08;
  // Setup data on falling edge, sample on rising edge (SPI mode 3)
  SPCR  = (1 << CPOL) | (1 << CPHA)
  // Transmit LSB first
        | (1 << DORD)
  // Enable interrupts for SPI
        | (1 << SPIE)
  // Enable SPI
        | (1 << SPE);

  // Set the first byte up
  SPDR = 0x00;
  PS2Handler = PS2_Listen;
  sei();
}

// Update the stored data packet
void PS2_Task(void) {
  // If chip select is disabled (high), quiet time is over. Reset state.
  if (PINB & 0x01) {
    QuietTime = 0;
    DDRB |= 0x08;
    SPDR  = 0x00;
  }

  PS2_InputList_t *map = PS2Input;
  uint16_t new_data = 0;

  while(map) {
    if (!map->input || (*map->input & map->mask))
      new_data |= map->buttons;

    map = map->parent;
  }

  Data = ~new_data;
}

void PS2_MapInput(uint16_t *input, uint16_t mask, PS2_INPUT buttons) {
  PS2_InputList_t *child = calloc(1, sizeof(PS2_InputList_t));

  child->input    = input,
  child->mask     = mask,
  child->buttons  = buttons,
  child->parent   = PS2Input;

  PS2Input = child;
}

void PS2_AlwaysInput(PS2_INPUT buttons) {
  PS2_InputList_t *child = calloc(1, sizeof(PS2_InputList_t));

  child->input    = NULL,
  child->mask     = 0,
  child->buttons  = buttons,
  child->parent   = PS2Input;

  PS2Input = child;
}

// When a transfer is complete, determine what to do next
ISR(SPI_STC_vect) {
  // If chip select is still enabled, stay in quiet mode if set.
  if (QuietTime) return;

  uint8_t input = SPDR;
  // If our current input packet is polling the controller, re-enable writes and listen
  if (input == 0x01)
    PS2Handler = PS2_Listen;

  PS2Handler(input);
}
