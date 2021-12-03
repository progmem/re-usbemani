#pragma once
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <avr/eeprom.h>
#include <util/crc16.h>

// Include everything for use in configuration
#include "color.h"
#include "color_provider.h"
#include "effect.h"
#include "effect_deferer.h"
#include "input.h"
#include "ps2.h"
#include "rgb.h"

#define EEPROM_BYTE(x) (const uint8_t  *)(x)
#define EEPROM_WORD(x) (const uint16_t *)(x)
#define EEPROM_ADDR(x) (const void     *)(x)

typedef enum {
  CONFIG_DATASOURCE_ALWAYS      = 0x00, // Index doesn't matter
  CONFIG_DATASOURCE_DIGITAL     = 0x10, // 0-15
  CONFIG_DATASOURCE_ENCODER     = 0x20, // 0-4
  CONFIG_DATASOURCE_ENCODER_CW  = 0x28, // 0x08 << 1 = 0x10
  CONFIG_DATASOURCE_ENCODER_CCW = 0x30, // 0x10 << 1 = 0x20
  CONFIG_DATASOURCE_ANALOG      = 0x40, // 0-11
  CONFIG_DATASOURCE_INDEX       = 0x0F,
  CONFIG_DATASOURCE_SOURCE      = 0xF0,
  CONFIG_DATASOURCE_NC          = 0xFF,
} CONFIG_DATASOURCE;

typedef enum {
  // Color Providers
  CONFIG_COLOR_PROVIDER_HUE = 0,
  CONFIG_COLOR_PROVIDER_HUECYCLE,
  CONFIG_COLOR_PROVIDER_HUERANDOM,
  CONFIG_COLOR_PROVIDER_RAINBOW,
  CONFIG_COLOR_PROVIDER = 0x0F,
  // Effects
  CONFIG_EFFECT_SINGLECOLOR   = 0x00,
  CONFIG_EFFECT_MULTICOLOR    = 0x10,
  CONFIG_EFFECT_CYCLEONPRESS  = 0x20,
  CONFIG_EFFECT_ONLYONPRESS   = 0x30,
  CONFIG_EFFECT_SPLASH        = 0x40,
  CONFIG_EFFECT    = 0xF0,
} CONFIG_RGB_EFFECT;

typedef enum {
  CONFIG_RGB_FRAMEPROCESS_CLEAR,
  CONFIG_RGB_FRAMEPROCESS_FADE,
  CONFIG_RGB_FRAMEPROCESS_FADERANDOM
} CONFIG_RGB_FRAMEPROCESS;

//// Structs
// Header
typedef struct {
  char      string[8];
  uint16_t  crc16_device;
  uint16_t  crc16_user;
} ConfigHeader_t;
// Device Config
typedef struct {
  PS2_PIN     pin;
  PS2_INVERT  invert;
} ConfigDevice_PS2_t;
typedef struct {
  RGB_PIN pin;
  uint8_t quantity;
} ConfigDevice_RGB_t;
typedef struct {
  INPUT_PIN_INDEX pin[16];
} ConfigDevice_Input_t;
typedef struct {
  INPUT_PIN_INDEX pin[16];
} ConfigDevice_Output_t;
typedef struct {
  INPUT_PIN_INDEX pin_a;
  INPUT_PIN_INDEX pin_b;
  uint16_t        ppr;
} ConfigDevice_Encoder_t;
typedef struct {
  ANALOG_PIN_INDEX pin[12];
} ConfigDevice_Analog_t;
typedef struct {
  ConfigDevice_PS2_t      PS2;
  ConfigDevice_RGB_t      RGB;
  ConfigDevice_Input_t    Input;
  ConfigDevice_Output_t   Output;
  ConfigDevice_Encoder_t  Encoder[5];
  ConfigDevice_Analog_t   Analog;
} ConfigDevice_t;
// User Config
typedef struct {
  union {
    struct { CONFIG_DATASOURCE map[24]; };
    struct {
      CONFIG_DATASOURCE map_button[16];
      CONFIG_DATASOURCE map_lx_neg;
      CONFIG_DATASOURCE map_rx_neg;
      CONFIG_DATASOURCE map_ly_neg;
      CONFIG_DATASOURCE map_ry_neg;
      CONFIG_DATASOURCE map_lx_pos;
      CONFIG_DATASOURCE map_rx_pos;
      CONFIG_DATASOURCE map_ly_pos;
      CONFIG_DATASOURCE map_ry_pos;
    };
  };
} ConfigUser_USBMap_t;
typedef struct {
  CONFIG_DATASOURCE source;
  PS2_INPUT         output;
} ConfigUser_PS2Map_t;
typedef struct {
  uint8_t timeout;
} ConfigUser_HIDOut_t;
typedef struct {
  uint16_t hold_time;
  uint16_t target_max;
  uint16_t target_rot;
} ConfigUser_Encoder_t;
typedef struct {
  uint8_t trigger;
  uint8_t release;
} ConfigUser_Analog_t;
typedef struct {
  CONFIG_RGB_FRAMEPROCESS fade_process;
  uint8_t fade_rate;
  uint8_t splash_fade_rate;
  uint8_t splash_bounds_start;
  uint8_t splash_bounds_end;
} ConfigUser_RGB_t;
typedef struct {
  CONFIG_DATASOURCE trigger; // Trigger for effect
  CONFIG_RGB_EFFECT effect;
  uint8_t           start;
  uint8_t           size;
  union {
    struct { uint8_t hue; uint8_t hue_delta; }; // Hue, HueCycle
    struct { uint8_t quantity; CONFIG_DATASOURCE source; }; // RainbowVariable
  };
} ConfigUser_Effect_t;
typedef struct {
  ConfigUser_USBMap_t   USBMap;
  ConfigUser_PS2Map_t   PS2Map[16];
  ConfigUser_HIDOut_t   HIDOut;
  ConfigUser_Encoder_t  Encoder[5];
  ConfigUser_Analog_t   Analog[12];
  ConfigUser_RGB_t      RGB;
  ConfigUser_Effect_t   Effect[32];
} ConfigUser_t;

// Encapsulating Struct
typedef struct {
  ConfigHeader_t Header;
  ConfigDevice_t Device;
  ConfigUser_t   User;
} Config_t;

#define CONFIG_HEADER "USBEMANI"

void Config_LoadFromEEPROM(void);