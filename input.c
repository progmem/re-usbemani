#include "input.h"

#define SETUP_PIN_MASK(x) (1 << (x & 0x07))

uint8_t process_output_loading_data = 0;
static inline void process_output_direct(void);
static inline void process_output_latch(void);

const uint8_t rotary_lookup[6][4] = {
  {0x3 , 0x2, 0x1,  0x0},  {0x23, 0x0, 0x1, 0x0},
  {0x13, 0x2, 0x0,  0x0},  {0x3 , 0x5, 0x4, 0x0},
  {0x3 , 0x3, 0x4,  0x10}, {0x3 , 0x5, 0x3, 0x20},
};

Input_Encoders_t _io_rotary = {
  .active = 0,
};

Input_Buttons_t _io_buttons = {
  .active = 0,
};

Input_Analog_t _io_analog = {
  .active = 0,
};

Output_Pins_t _io_outputs = {
  .active = 0,
  .latch_group = PIN_NC,
};

uint16_t _io_ticks = 0;
uint8_t  raw_input[3];

static inline void process_rotary(void) {
  static uint8_t active = 0;
  register uint8_t pin_state = 0;

  Input_Rotary_t *enc = &(_io_rotary.encoders[active]);

  if (enc->hold)
    enc->hold--;
  else
    enc->direction = 0;

  if (raw_input[enc->group[0]] & enc->mask[0])
    pin_state |= 1 << 0;
  if (raw_input[enc->group[1]] & enc->mask[1])
    pin_state |= 1 << 1;

  enc->state = rotary_lookup[enc->state & 0xf][pin_state];

  uint8_t result = enc->state & (INPUT_ROTARY_CW | INPUT_ROTARY_CCW);
  if (result) {
    enc->direction  = result;
    enc->hold       = enc->max_hold;
    if (result == INPUT_ROTARY_CW) {
      enc->position++;
      if (enc->position == enc->max_position)
        enc->position = 0;
      enc->position16 += enc->increment16;
      return;
    } else {
      enc->position--;
      if (enc->position == -1)
        enc->position = enc->max_position - 1;
      enc->position16 -= enc->increment16;
      return;
    }
  }

  active++;
  if (active >= _io_rotary.active) active = 0;
}

static inline void process_analog(void) {
  static uint8_t index = 0;

  // Only run if analog is enabled and we are not currently running a conversion
  uint8_t flags = ADCSRA & ((1 << ADSC) | (1<<ADEN));
  if (flags != (1<<ADEN)) return;

  // Read the top 8 bits and store in the currently-polled analog value
  _io_analog.raw[index] = ADCH;

  // Switch pins
  ++index; if (index >= _io_analog.active) index = 0;
  ADMUX  = (1<<ADLAR) | (1<<REFS0) | (_io_analog.mask[index] & 0x07);
  ADCSRB = (_io_analog.mask[index] & 0x20);

  // Begin conversion of the next pin
  ADCSRA |= (1<<ADSC);
}

static inline void process_output_direct() {
  DDRB  |=  _io_outputs.precalc_ddr[0],
  DDRD  |=  _io_outputs.precalc_ddr[1],
  DDRF  |=  _io_outputs.precalc_ddr[2],
  PORTB = (PORTB & ~_io_outputs.precalc_low[0]) | _io_outputs.precalc_high[0],
  PORTD = (PORTD & ~_io_outputs.precalc_low[1]) | _io_outputs.precalc_high[1],
  PORTF = (PORTF & ~_io_outputs.precalc_low[2]) | _io_outputs.precalc_high[2];
}

static inline void process_output_latch() {
  DDRB  |=  _io_outputs.precalc_ddr[0],
  DDRD  |=  _io_outputs.precalc_ddr[1],
  DDRF  |=  _io_outputs.precalc_ddr[2],
  PORTB = (PORTB & ~_io_outputs.precalc_low[0]) | _io_outputs.precalc_high[0],
  PORTD = (PORTD & ~_io_outputs.precalc_low[1]) | _io_outputs.precalc_high[1],
  PORTF = (PORTF & ~_io_outputs.precalc_low[2]) | _io_outputs.precalc_high[2];

  switch(_io_outputs.latch_group) {
    case 0x02:
      DDRF  |=  _io_outputs.latch_mask,
      PORTF |=  _io_outputs.latch_mask;
      PORTF &= ~_io_outputs.latch_mask;
      break;
    case 0x01:
      DDRD  |=  _io_outputs.latch_mask,
      PORTD |=  _io_outputs.latch_mask;
      PORTD &= ~_io_outputs.latch_mask;
      break;
    default:
      DDRB  |=  _io_outputs.latch_mask,
      PORTB |=  _io_outputs.latch_mask;
      PORTB &= ~_io_outputs.latch_mask;
  }

  DDRB  &= ~_io_outputs.precalc_ddr[0],
  DDRD  &= ~_io_outputs.precalc_ddr[1],
  DDRF  &= ~_io_outputs.precalc_ddr[2],
  PORTB |=  _io_outputs.precalc_low[0],
  PORTD |=  _io_outputs.precalc_low[1],
  PORTF |=  _io_outputs.precalc_low[2];
}

void setup_pin(INPUT_PIN_INDEX pin) {
  switch(pin & 0x18) {
    case 0x10:  // PORTF
      DDRF  &= ~SETUP_PIN_MASK(pin);
      PORTF |=  SETUP_PIN_MASK(pin);
      break;
    case 0x08:  // PORTD
      DDRD  &= ~SETUP_PIN_MASK(pin);
      PORTD |=  SETUP_PIN_MASK(pin);
      break;
    default:    // PORTB
      DDRB  &= ~SETUP_PIN_MASK(pin);
      PORTB |=  SETUP_PIN_MASK(pin);
  }
}

void Input_RegisterButton(INPUT_PIN_INDEX pin) {
  setup_pin(pin);

  _io_buttons.group[_io_buttons.active] = (pin >> 3);
  _io_buttons.mask[_io_buttons.active]  = (1 << (pin & 0x07));

  ++_io_buttons.active;
}

void Input_RegisterAnalog(ANALOG_PIN_INDEX pin, ANALOG_SHOULD_INVERT invert) {
  // Enable analog subsystem, if it isn't already.
  if (pin & 0x20)
    DIDR2 |= (1 << (pin & 0x07));
  else
    DIDR0 |= (1 << (pin & 0x07));

  _io_analog.mask[_io_analog.active] = pin;
  _io_analog.raw[ _io_analog.active] = 0;

  if (invert)
    _io_analog.invert |= (1 << _io_analog.active);

  _io_analog.active++;
}

void Input_RegisterRotary(INPUT_PIN_INDEX pin1, INPUT_PIN_INDEX pin2, uint16_t ppr, uint16_t hold) {
  setup_pin(pin1);
  setup_pin(pin2);

  Input_Rotary_t *enc = &(_io_rotary.encoders[_io_rotary.active]);
  enc->group[0] = (pin1 >> 3);
  enc->group[1] = (pin2 >> 3);

  enc->mask[0] = (1 << (pin1 & 0x07));
  enc->mask[1] = (1 << (pin2 & 0x07));

  enc->position     = ppr >> 1;
  enc->max_position = ppr << 1;
  enc->max_hold     = hold;

  enc->increment16  = 65536 / enc->max_position;

  ++_io_rotary.active;
}

void Input_RotaryLogicalTarget(uint8_t index, uint16_t logical_max, uint16_t logical_per_rotation) {
  if (!logical_max || !logical_per_rotation) return;

  Input_Rotary_t *enc = &(_io_rotary.encoders[index]);

  if (logical_max == logical_per_rotation)
    enc->increment16  = 65536 / enc->max_position;
  else
    enc->increment16 = ((65536 / logical_max) * logical_per_rotation) / enc->max_position;
}

void Input_RotaryHold(uint8_t index, uint16_t hold) {
  Input_Rotary_t *enc = &(_io_rotary.encoders[index]);
  enc->max_hold = hold;
}

void Input_AnalogDigitalThresholds(uint8_t index, uint8_t trigger, uint8_t release) {
  _io_analog.trigger[index] = trigger,
  _io_analog.release[index] = release;
}

void Output_RegisterLatch(INPUT_PIN_INDEX pin) {
  if ((pin & OUT_INV) == OUT_INV) {
    _io_outputs.invert = 0xFFFF;
    pin ^= OUT_INV;
  }

  _io_outputs.latch_group = (pin >> 3);
  _io_outputs.latch_mask  = (1 << (pin & 0x07));
}

void Output_RegisterPin(INPUT_PIN_INDEX pin) {
  if (_io_outputs.latch_group != 0xFF) return;

  if (pin & OUT_INV) {
    _io_outputs.invert |= (1 << _io_outputs.active);
    pin &= ~(OUT_INV);
  }

  setup_pin(pin);

  _io_outputs.group[_io_outputs.active] = (pin >> 3);
  _io_outputs.mask[_io_outputs.active]  = (1 << (pin & 0x07));

  _io_outputs.active++;
}

void InputOutput_Begin(INPUT_FREQUENCY freq) {
  // Do an initial read
  raw_input[0] = PINB,
  raw_input[1] = PIND,
  raw_input[2] = PINF;
  process_rotary();

  for (uint8_t i = _io_rotary.active; i < 5; i++) {
    Input_Rotary_t *enc = &(_io_rotary.encoders[i]);
    enc->position = 0;
  }

  // Setup digital interrupt
  TCCR0A  = 0,
  TCCR0B  = 0,
  TCNT0   = 0,
  OCR0A   = freq,
  TCCR0A |= (1 << WGM01),
  TCCR0B |= (1 << CS01) | (1 << CS00),
  TIMSK0 |= (1 << OCIE0A);

  // Setup analog
  if (_io_analog.active) {
    // Enable ADC
    ADCSRA  |= (1<<ADEN);
    // Set prescaler
    ADCSRA  |= (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
    // Left-shift data
    // Use VCC
    ADMUX   |= (1<<ADLAR) | (1<<REFS0);
    // Clear multiplexers
    ADMUX   &= ~((1<<MUX0) | (1<<MUX1) | (1<<MUX2));
    ADCSRB  &= ~( 1<<MUX5);
    // Enable capture
    ADCSRA  |=  (1<<ADSC);
  }

  sei();
}

void Input_Task(void) {
  _io_buttons.data = 0;

  for (uint8_t i = 0; i < _io_buttons.active; i++) {
    if (raw_input[_io_buttons.group[i]] & _io_buttons.mask[i])
      continue;
    _io_buttons.data |= (1 << i);
  }
}

void Analog_Task(void) {
  for (uint8_t i = 0; i < _io_analog.active; i++) {
    uint16_t mask = (1 << i);

    _io_analog.value[i] = _io_analog.raw[i];

    // Invert the value if requested
    if (_io_analog.invert & mask)
      _io_analog.value[i] = 255 - _io_analog.raw[i];

    // Perform analog-to-input conversion
    if (_io_analog.value[i] > _io_analog.trigger[i])
      _io_analog.digital |=   mask;
    if (_io_analog.value[i] < _io_analog.release[i])
      _io_analog.digital &= ~(mask);
  }
}

void Rotary_Task(void) {
/*  for (uint8_t i = 0; i < _io_rotary.active; i++) {
    Input_Rotary_t *enc = &(_io_rotary.encoders[i]);
  }*/
}

void Output_Task(void) {
  process_output_loading_data = 1;
  Input_Buttons_t *access = NULL;

  // Use either the buttons (latches) or output (direct) for our calculations
  if (_io_outputs.active)
    access = (void*)&_io_outputs;
  else if (_io_outputs.latch_group != 0xFF)
    access = (void*)&_io_buttons;
  // If output hasn't been setup, exit.
  if (!access) return;

  uint16_t data = _io_outputs.data ^ _io_outputs.invert;

  for (uint8_t i = 0; i < _io_buttons.active; i++) {
    uint8_t group = access->group[i];
    uint8_t mask  = access->mask[i];

    _io_outputs.precalc_ddr[group]  |=   mask;
    _io_outputs.precalc_high[group] &= ~(mask);
    _io_outputs.precalc_low[ group] &= ~(mask);

    if(data & (1 << i)) {
      _io_outputs.precalc_high[group] |= mask;
    } else {
      _io_outputs.precalc_low[ group] |= mask;
    }
  }
  process_output_loading_data = 0;
}

uint16_t Input_Ticks(uint8_t index) {
  if(index)
    return (_io_ticks % index);
  return _io_ticks;
}

uint16_t Input_GetButtons(void) {
  return _io_buttons.data; }

uint16_t Input_GetRotaryPhysicalPosition(uint8_t index) {
  return _io_rotary.encoders[index].position; }

uint16_t Input_GetRotaryLogicalPosition(uint8_t index) {
  return _io_rotary.encoders[index].position16; }

uint16_t Input_GetRotaryMaximum(uint8_t index) {
  return _io_rotary.encoders[index].max_position; }

uint16_t Input_GetRotaryDirection(uint8_t index) {
  return _io_rotary.encoders[index].direction; }

uint16_t Input_GetAnalog(uint8_t index) {
  return _io_analog.value[index]; }

uint16_t Input_GetAnalogDigital(void) {
  return _io_analog.digital; }


uint16_t*Input_PtrButtons(void) {
  return &(_io_buttons.data); }

uint16_t*Input_PtrRotaryPhysicalPosition(uint8_t index) {
  return &(_io_rotary.encoders[index].position); }

uint16_t*Input_PtrRotaryLogicalPosition(uint8_t index) {
  return &(_io_rotary.encoders[index].position16); }

uint16_t*Input_PtrRotaryDirection(uint8_t index) {
  return &(_io_rotary.encoders[index].direction); }

uint16_t*Input_PtrAnalog(uint8_t index) {
  return &(_io_analog.value[index]); }

uint16_t*Input_PtrAnalogDigital(void) {
  return &(_io_analog.digital); }

uint8_t  Input_CountButtons(void) {
  return _io_buttons.active; }
uint8_t  Input_CountRotary(void) {
  return _io_rotary.active; }
uint8_t  Input_CountAnalog(void) {
  return _io_analog.active; }

uint16_t Output_Get(void) {
  return _io_outputs.data; }

uint16_t*Output_Ptr(void) {
  return &(_io_outputs.data); }

void Output_Set(uint16_t data) {
  _io_outputs.data  = data; }

__attribute__((weak)) void Input_ExecuteOnInterrupt(void) { }

// Interrupt that processes inputs
ISR(TIMER0_COMPA_vect) {
  static volatile uint8_t ticks = 0;

  PORTC &= ~(0x80 | 0x40);
  // Read pins
  raw_input[0] = PINB,
  raw_input[1] = PIND,
  raw_input[2] = PINF;

  // Update rotary encoders every cycle
  PORTE |= 0x40;
  process_rotary();
  PORTE &= ~0x40;
  ticks++;

  // Handle outputs
  if (!process_output_loading_data && (ticks & 0x3f) == 0x3e) {
    if (_io_outputs.active)
      process_output_direct();
    else if (_io_outputs.latch_group != 0xFF)
      process_output_latch();
  }

  // Handle additional interrupts
  if ((ticks & 0x3f) == 0x3f) {
    Input_ExecuteOnInterrupt();
    _io_ticks++;
  }

  process_analog();
}
