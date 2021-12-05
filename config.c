#include "config.h"
#include "eeprom.h"

//// Datasource Divider
typedef struct {
  uint16_t *source;
  union { uint16_t mask; uint16_t max; };
} _datasource;

_datasource _datasource_parse(CONFIG_DATASOURCE ds) {
  uint8_t source = ds & CONFIG_DATASOURCE_SOURCE;
  uint8_t index  = ds & CONFIG_DATASOURCE_INDEX;

  _datasource ret = { .mask = 1 << index };

  switch (source) {
    case CONFIG_DATASOURCE_DIGITAL:
      ret.source = Input_PtrButtons();
      break;
    case CONFIG_DATASOURCE_ENCODER:
      ret.source = Input_PtrRotaryPhysicalPosition(index);
      ret.max    = Input_GetRotaryMaximum(index);
      break;
    case CONFIG_DATASOURCE_ENCODER_CW:
      ret.source = Input_PtrRotaryDirection(index);
      ret.mask   = INPUT_ROTARY_CW;
      break;
    case CONFIG_DATASOURCE_ENCODER_CCW:
      ret.source = Input_PtrRotaryDirection(index);
      ret.mask   = INPUT_ROTARY_CCW;
      break;
    case CONFIG_DATASOURCE_ENCODER_DIR:
      ret.source = Input_PtrRotaryDirection(index);
      ret.mask   = INPUT_ROTARY_CW | INPUT_ROTARY_CCW;
      break;
    case CONFIG_DATASOURCE_ANALOG:
      ret.source = Input_PtrAnalog(index);
      ret.max    = 255;
      break;
    case CONFIG_DATASOURCE_ANALOG_DIGI:
      ret.source = Input_PtrAnalogDigital();
      break;
    case CONFIG_DATASOURCE_OUTPUT:
      ret.source = Output_Ptr();
      break;
  }
  return ret;
}

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
inline void _user_usb(void) {
  ConfigUser_USBMap_t usbmap;
  eeprom_read_block((void *)&usbmap, (void *)&(eeprom.User.USBMap), sizeof(usbmap));

  // Get pointer
  USBemani_Input_t *Report = InputList_BuildReport();

  // Axis
  for (uint8_t i = 0; i < 8; i++) {
    if (usbmap.map[i] == CONFIG_DATASOURCE_NC) continue;

    int8_t *ptr  = NULL;
    int8_t value = -100;
    if (i & 0x04) value = 100;

    switch(i & 0x03) {
      case 0x00: // LX
        ptr = &(Report->LX);
        break;
      case 0x01: // RX
        ptr = &(Report->RX);
        break;
      case 0x02: // LY
        ptr = &(Report->LY);
        break;
      case 0x03: // RY
        ptr = &(Report->RY);
        break;
    }

    _datasource ds = _datasource_parse(usbmap.map[i]);
    InputList_RegisterAxis(ds.source, ds.mask, ptr, value);
  }

  // Buttons
  uint16_t *ptr = &(Report->Button);
  for (uint8_t i = 0; i < 16; i++) {
    if (usbmap.map[8+i] == CONFIG_DATASOURCE_NC) return;
    uint16_t value  = (1 << i);

    _datasource ds = _datasource_parse(usbmap.map[8+i]);
    InputList_RegisterButton(ds.source, ds.mask, ptr, value);
  }
}

inline void _user_ps2map(void) {
  ConfigUser_PS2Map_t ps2map;

  for (uint8_t i = 0; i < 16; i++) {
    eeprom_read_block((void *)&ps2map, (void *)&(eeprom.User.PS2Map[i]), sizeof(ps2map));
    if (ps2map.source == CONFIG_DATASOURCE_NC)
      return;

    _datasource ds = _datasource_parse(ps2map.source);
    PS2_MapInput(ds.source, ds.mask, ps2map.output);
  }
}

inline void _user_out(void) {
  ConfigUser_Out_t out;
  eeprom_read_block((void *)&out, (void *)&(eeprom.User.Out), sizeof(out));

  OutputList_TimeoutAt(out.hid_timeout);
  for (uint8_t i = 0; i < 16; i++) {
    _datasource ds = _datasource_parse(out.channels[i]);
    OutputList_Register(ds.source, ds.mask, 1 << i);
  }
}

inline void _user_encoder(void) {
  ConfigUser_Encoder_t encoder;

  for (uint8_t i = 0; i < Input_CountRotary(); i++) {
    eeprom_read_block((void *)&encoder, (void *)&(eeprom.User.Encoder[i]), sizeof(encoder));
    Input_RotaryHold(i, encoder.hold_time / Input_CountRotary());
    Input_RotaryLogicalTarget(i, encoder.target_max, encoder.target_rot);
  }
}

inline void _user_analog(void) {
  ConfigUser_Analog_t analog;

  for (uint8_t i = 0; i < Input_CountAnalog(); i++) {
    eeprom_read_block((void *)&analog, (void *)&(eeprom.User.Analog[i]), sizeof(analog));
    Input_AnalogDigitalThresholds(i, analog.trigger, analog.release);
  }
}

inline void _user_rgb(void) {
  ConfigUser_RGB_t rgb;
  eeprom_read_block((void *)&rgb, (void *)&(eeprom.User.RGB), sizeof(rgb));

  switch (rgb.fade_process) {
    case CONFIG_RGB_FRAMEPROCESS_FADE:
      RGB_SetProcessFrame(RGB_FadeRange);
      break;
    case CONFIG_RGB_FRAMEPROCESS_FADERANDOM:
      RGB_SetProcessFrame(RGB_FadeRangeRandom);
      break;
    default:
      RGB_SetProcessFrame(RGB_ClearRange);
  }
  RGB_SetFadeRate(rgb.fade_rate);
  Effect_SetSplashFadeRate(rgb.splash_fade_rate);
  Effect_SetSplashBounds(rgb.splash_bounds_start, rgb.splash_bounds_end);
}

inline void _user_effects(void) {
  ConfigUser_Effect_t effect;
  ColorProvider_t *pout;
  Effect_t        *eout;

  for (int i = 0; i < 32; i++) {
    eeprom_read_block((void *)&effect, (void *)&(eeprom.User.Effect[i]), sizeof(effect));
    if (effect.trigger == CONFIG_DATASOURCE_NC) return;

    _datasource source  = _datasource_parse(effect.source);

    // Color Provider
    switch (effect.effect & CONFIG_COLOR_PROVIDER) {
      case CONFIG_COLOR_PROVIDER_HUE:
        pout = ColorProvider_Hue(effect.hue);
        break;
      case CONFIG_COLOR_PROVIDER_HUECYCLE:
        pout = ColorProvider_HueCycle(effect.hue, effect.hue_delta);
        break;
      case CONFIG_COLOR_PROVIDER_HUERANDOM:
        pout = ColorProvider_HueRandom();
        break;
      case CONFIG_COLOR_PROVIDER_RAINBOW:
        pout = ColorProvider_RainbowVariable(effect.quantity, source.source, source.max);
        break;
      default: // Invalid color provider, stop
        return;
    }

    // Effect
    switch (effect.effect & CONFIG_EFFECT) {
      case CONFIG_EFFECT_SINGLECOLOR:
        eout = Effect_Single(pout, effect.start, effect.size);
        break;
      case CONFIG_EFFECT_MULTICOLOR:
        eout = Effect_Multi(pout, effect.start, effect.size);
        break;
      case CONFIG_EFFECT_CYCLEONPRESS:
        eout = Effect_Press(pout, effect.start, effect.size);
        break;
      case CONFIG_EFFECT_ONLYONPRESS:
        eout = Effect_SingleShot(pout, effect.start, effect.size);
        break;
      case CONFIG_EFFECT_SPLASH:
        eout = Effect_Splash(pout, effect.start, effect.size);
        break;
      default: // Invalid color provider, stop
        return;
    }

    // Trigger
    if (effect.trigger & CONFIG_DATASOURCE_ALWAYS)
      Effect_AutoQueue(eout);
    else {
      _datasource trigger = _datasource_parse(effect.trigger);
      Effect_Defer(eout, trigger.source, trigger.mask);
    }
  }
}

// Check if EEPROM is valid
bool _eeprom_valid(void) {
  bool status = true;
  uint16_t crc16;
  uint8_t *bytes;

  // Device Config
  crc16 = 0xffff;
  bytes = (uint8_t *)&(eeprom.Device);
  for (uint16_t i = 0; i < sizeof(ConfigDevice_t); i++) {
    crc16 = _crc16_update(crc16, eeprom_read_byte(EEPROM_BYTE(bytes)));
    bytes++;
  }

  if ( crc16 != eeprom_read_word( EEPROM_WORD(&(eeprom.Header.crc16_device)) ) ) {
    status = false;
  }

  // User Config
  crc16 = 0xffff;
  bytes = (uint8_t *)&(eeprom.User);
  for (uint16_t i = 0; i < sizeof(ConfigUser_t); i++) {
    crc16 = _crc16_update(crc16, eeprom_read_byte(EEPROM_BYTE(bytes)));
    bytes++;
  }

  if ( crc16 != eeprom_read_word( EEPROM_WORD(&(eeprom.Header.crc16_user)) ) ) {
    status = false;
  }

  return status;
}

// Reset the EEPROM to a safe state (effectively wiping the configuration clean)
void _eeprom_reset(void) {
}

void Config_LoadFromEEPROM(void) {
  // If EEPROM is considered dirty, don't use it.
  if (!_eeprom_valid()) {
    PORTE |= 0x40;
    return;
  }

  //// Device
  // IO
  _device_input();
  _device_output();
  _device_encoder();
  _device_analog();
  InputOutput_Begin(INPUT_FREQ_8KHZ);
  // PS2
  _device_ps2();
  // RGB
  _device_rgb();

  //// User
  // USB Mapping
  _user_usb();
  // PS2 Mapping
  _user_ps2map();
  // Output
  _user_out();
  // Encoders
  _user_encoder();
  // Analog
  _user_analog();
  // RGB Effects
  _user_rgb();
  _user_effects();
}
