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
  uint8_t  pin;
  uint8_t *data;
  uint8_t  bitmask;
  uint16_t index;
  uint16_t size;
  uint16_t ticks;
  RGB_Flags state;
} transmit;

void RGB_Init(RGB_PIN pin, uint8_t quantity) {
  if (framebuffer.data) free(framebuffer.data);

  framebuffer.size  = quantity;
  framebuffer.data  = calloc(quantity, sizeof(RGB_Color_t));

  transmit.pin    = pin;
  transmit.size   = quantity * 24;
  transmit.ticks  = 0;

  // Make sure we can output a signal
  DDRC  |=  pin,
  PORTC &= ~pin;

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
  TCNT1L = 127,
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
  asm(
  "cbi 0x08, 7\n\t"
  "cbi 0x08, 6\n\t"
  );

  // Finish transmission
  if (transmit.index >= transmit.size) {
    TCCR1B &= ~(1 << CS10);
    transmit.state &= ~(RGB_SYNCED);
    return;
  }

  register uint8_t bit_test = (*transmit.data & transmit.bitmask);

  if (transmit.bitmask & 0x01)
    transmit.data++;

  transmit.index++;
  transmit.bitmask = bitmasks[transmit.index & 0x07];

  if(transmit.pin == RGB_C7) {
    if (bit_test)
      asm(
        "sbi 0x08, 7\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        // "cbi 0x08, 7\n\t"
      );
    else
      asm(
        "sbi 0x08, 7\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "nop\n\t"
        "cbi 0x08, 7\n\t"
      );
    return;
  }

  if (bit_test)
    asm(
      "sbi 0x08, 6\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      // "cbi 0x08, 6\n\t"
    );
  else
    asm(
      "sbi 0x08, 6\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "nop\n\t"
      "cbi 0x08, 6\n\t"
    );
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

//// Screen-clearing functions
uint8_t fade_rate = 3;
uint8_t fade_mask = 0x03;
void  (*framebuffer_clear_func)(uint8_t, uint8_t) = RGB_ClearRange;

void RGB_SetProcessFrame(void (*clear_func)) {
  framebuffer_clear_func = clear_func;
}

void RGB_ProcessFrame(void) {
  if(framebuffer_clear_func)
    framebuffer_clear_func(0, framebuffer.size);
}

void RGB_SetFadeRate(uint8_t rate) {
  if (!rate) rate = 1;

  fade_rate = rate;
  fade_mask = rate-1;
}

void RGB_FadeRange(uint8_t index, uint8_t size) {
  uint8_t rate = fade_rate;
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
    int8_t rate = fade_rate - (Util_Random() & fade_mask);
    if (rate < 0) rate = 0;
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