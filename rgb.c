#include "rgb.h"

typedef enum {
  RGB_DIRTY   = 0x01,
  RGB_SYNCED  = 0x02,
} RGB_Flags;

const uint8_t bitmasks[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

struct {
  uint8_t      size;
  RGB_Color_t *data;
} framebuffer;

struct {
  uint8_t *data;
  uint8_t  bitmask;
  uint16_t index;
  uint16_t size;
  uint16_t ticks;
  RGB_Flags state;
} transmit;

void RGB_Init(uint8_t quantity) {
  if (framebuffer.data) free(framebuffer.data);

  framebuffer.size  = quantity;
  framebuffer.data  = calloc(quantity, sizeof(RGB_Color_t));

  transmit.size  = quantity * 24;
  transmit.ticks = 0;

  // Make sure we can output a signal
  DDRC  |=  0x80,
  PORTC &= ~0x80;

  //// Timer 1
  // Clear registers and counters
  TIMSK1 &= ~((1 << OCIE1A) | (1 << OCIE1B) | (1 << OCIE1C)),
  TCCR1A = 0,
  TCCR1B = 0,

  // Count up to OCR1A in CTC mode.
  TCCR1B |= (1 << WGM12),

  // Ping as frequently as we can
  OCR1AL = 127,
  OCR1AH = 0;

  // Enable interrupts
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void RGB_Transmit(void) {
  // Don't transmit if RGB's disabled or no payload has been set.
  if (!framebuffer.size) return;
  // Don't transmit if already transmitting.
  if (TCCR1B & (1 << CS10)) return;
  // Don't transmit unless a sync has been latched in
  if (!(transmit.state & RGB_SYNCED)) return;

  // Reset timer
  TCNT1L = 57,
  TCNT1H = 0;

  transmit.data     = (uint8_t *)framebuffer.data;
  transmit.bitmask  =  bitmasks[0];
  transmit.index    = 0;

  // Count at clock frequency.
  TCCR1B |= (1 << CS10);
  transmit.ticks++;
}

void RGB_Sync(void) {
  transmit.state |= RGB_SYNCED;
}

bool RGB_Ready(void) {
  if (transmit.state & RGB_SYNCED) return false;
  return !(TCCR1B & (1 << CS10));
}

uint16_t RGB_GetTicks(void) {
  return transmit.ticks;
}

uint16_t *RGB_PtrTicks(void) {
  return &transmit.ticks;
}

ISR(TIMER1_COMPA_vect) {
  // Finish transmission
  if (transmit.index >= transmit.size) {
    TCCR1B &= ~(1 << CS10);
    transmit.state &= ~(RGB_SYNCED);
    return;
  }

  uint8_t bit_test = (*transmit.data & transmit.bitmask);

  asm("sbi 0x08, 7");
  if (bit_test)
    asm("nop\nnop\nnop\nnop");
  asm("cbi 0x08, 7");

  if (transmit.bitmask & 0x01)
    transmit.data++;

  transmit.index++;
  transmit.bitmask = bitmasks[transmit.index & 0x07];
}

void RGB_SetRange(RGB_Color_t color, uint8_t index, uint8_t size) {
  if (index >= framebuffer.size) return;
  if ((index + size) > framebuffer.size) size = framebuffer.size - index;

  register uint8_t r = color.r, g = color.g, b = color.b;
  RGB_Color_t *dest = &framebuffer.data[index];

  while(size) {
    dest->r = r, dest->g = g, dest->b = b;
    dest++;
    size--;
  }
}

#define ALWAYS_INLINE __attribute__ ((always_inline)) static inline
ALWAYS_INLINE uint8_t qadd8(uint8_t i, uint8_t j) {
  asm volatile(
    "add %0, %1    \n\t"
    "brcc L_%=     \n\t"
    "ldi %0, 0xFF  \n\t"
    "L_%=: "
    : "+a" (i)
    : "a"  (j)
  );
  return i;
}

void RGB_AddRange(RGB_Color_t color, uint8_t index, uint8_t size) {
  if (index >= framebuffer.size) return;
  if ((index + size) > framebuffer.size) size = framebuffer.size - index;

  // register uint8_t r, g, b;
  RGB_Color_t *dest = &framebuffer.data[index];

  while(size) {
    dest->r  = qadd8(dest->r >> 0, color.r >> 1);
    dest->g  = qadd8(dest->g >> 0, color.g >> 1);
    dest->b  = qadd8(dest->b >> 0, color.b >> 1);
    /*if (r & 0xFF00) r = 255;
    if (g & 0xFF00) g = 255;
    if (b & 0xFF00) b = 255;*/
    // dest->r = r, dest->g = g, dest->b = b;
    ++dest;
    --size;
  }
}

void RGB_FadeRange(uint8_t rate, uint8_t index, uint8_t size) {
  if (index >= framebuffer.size) return;
  if ((index + size) > framebuffer.size) size = framebuffer.size - index;

  register uint16_t r, g, b;
  RGB_Color_t *dest = &framebuffer.data[index];

  while(size) {
    r  = (dest->r << rate) - (dest->r);
    g  = (dest->g << rate) - (dest->g);
    b  = (dest->b << rate) - (dest->b);
    dest->r = r >> rate, dest->g = g >> rate, dest->b = b >> rate;
    ++dest;
    --size;
  }
}

void RGB_FadeRangeRandom(uint8_t index, uint8_t size) {
  if (index >= framebuffer.size) return;
  if ((index + size) > framebuffer.size) size = framebuffer.size - index;

  register uint16_t r, g, b;
  RGB_Color_t *dest = &framebuffer.data[index];

  while(size) {
    uint8_t rate = 5 - (Util_Random() & 0x03);

    r  = (dest->r << rate) - (dest->r);
    g  = (dest->g << rate) - (dest->g);
    b  = (dest->b << rate) - (dest->b);
    dest->r = r >> rate, dest->g = g >> rate, dest->b = b >> rate;
    ++dest;
    --size;
  }
}


void RGB_ClearRange(uint8_t index, uint8_t size) {
  if (index >= framebuffer.size) return;
  if ((index + size) > framebuffer.size) size = framebuffer.size - index;

  RGB_Color_t *dest = &framebuffer.data[index];

  while(size) {
    dest->r = 0, dest->g = 0, dest->b = 0;
    ++dest;
    --size;
  }
}