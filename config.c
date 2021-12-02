#include "config.h"
#include "eeprom.h"

//// Device Configuration
inline void _device_input(void) {
  ConfigDevice_Input_t input;
  eeprom_read_block((void *)&input, (void *)&(eeprom.Device.Input), sizeof(input));

  for (uint8_t i = 0; i < 16; i++) {
    if (input.pin[i] == PIN_NC) return;
    Input_RegisterButton(input.pin[i]);
  }
}

inline void _device_output(void) {
  ConfigDevice_Output_t output;
  eeprom_read_block((void *)&output, (void *)&(eeprom.Device.Output), sizeof(output));

  if (output.pin[0] & OUT_LATCH) {
    Output_RegisterLatch(output.pin[0] ^ OUT_LATCH);
    return;
  }

  for (uint8_t i = 0; i < 16; i++) {
    if (output.pin[i] == PIN_NC) return;
    Output_RegisterPin(output.pin[i]);
  }
}

inline void _device_encoder(void) {
  ConfigDevice_Encoder_t encoder;

  for (uint8_t i = 0; i < 5; i++) {
    eeprom_read_block((void *)&encoder, (void *)&(eeprom.Device.Encoder[i]), sizeof(encoder));

    if (encoder.pin_a == PIN_NC) return;
    Input_RegisterRotary(encoder.pin_a, encoder.pin_b, encoder.ppr, 500);
  }
}

inline void _device_analog(void) {
  ConfigDevice_Analog_t analog;
  eeprom_read_block((void *)&analog, (void *)&(eeprom.Device.Analog), sizeof(analog));

  for (uint8_t i = 0; i < 12; i++) {
    if (analog.pin[i] == ANALOG_NC) return;
    Input_RegisterAnalog(analog.pin[i], analog.pin[i] & ANALOG_INV);
    Input_AnalogDigitalThresholds(i, 128, 64); // Safe threshold
  }
}

inline void _device_ps2(void) {
  ConfigDevice_PS2_t ps2;
  eeprom_read_block((void *)&ps2, (void *)&(eeprom.Device.PS2), sizeof(ps2));

  if (ps2.pin == PS2_NC) return;
  PS2_Init(ps2.pin, ps2.invert);
}

inline void _device_rgb(void) {
  ConfigDevice_RGB_t rgb;
  eeprom_read_block((void *)&rgb, (void *)&(eeprom.Device.RGB), sizeof(rgb));

  if (rgb.pin == RGB_NC) return;
  RGB_Init(rgb.pin, rgb.quantity);
}

//// User Configuration
void _user_ps2map(void) {
  ConfigUser_PS2Map_t ps2map;

  for (uint8_t i = 0; i < 12; i++) {
    eeprom_read_block((void *)&ps2map, (void *)&(eeprom.User.PS2Map[i]), sizeof(ps2map));
    uint16_t *ptr    = NULL;
    uint16_t  source = ps2map.source & 0xF0;
    uint16_t  mask   = 0;

    // Stop processing
    if (source == CONFIG_DATASOURCE_NC)
      break;

    // Analog buttons
    if (source & CONFIG_DATASOURCE_ANALOG) {
      ptr  = Input_PtrAnalogDigital();
      mask = 1 << (ps2map.source & 0x0F);
    }
    // Rotary encoder
    else if (source & CONFIG_DATASOURCE_ENCODER) {
      uint8_t enc =  ps2map.source & 0x07;
      ptr  = Input_PtrRotaryDirection(enc);
      mask = (ps2map.source & 0x18) << 1;
    }
    // Digital buttons
    else if (source & CONFIG_DATASOURCE_DIGITAL) {
      ptr   = Input_PtrButtons();
      mask  = 1 << (ps2map.source & 0x0F);
    }

    PS2_MapInput(ptr, mask, ps2map.output);
  }
}

// Check if EEPROM is valid
bool _eeprom_valid(void) {
  return true;
}

// Reset the EEPROM to a safe state (effectively wiping the configuration clean)
void _eeprom_reset(void) {
}

void Config_LoadFromEEPROM(void) {
  // If EEPROM is considered dirty, don't use it.
  if (!_eeprom_valid())
    return;

  //// Device
  // IO
  _device_input();
  _device_output();
  _device_encoder();
  _device_analog();
  InputOutput_Begin(INPUT_FREQ_8KHZ);
  Output_Task();
  // PS2
  _device_ps2();
  // RGB
  _device_rgb();

  //// User
  // USB Mapping
  // _user_usb();
  // PS2 Mapping
  _user_ps2map();
}
