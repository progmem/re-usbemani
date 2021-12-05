#pragma once
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#define EEPROM_BYTE(x) (const uint8_t  *)(x)
#define EEPROM_WORD(x) (const uint16_t *)(x)
#define EEPROM_ADDR(x) (const void     *)(x)

#pragma pack(push,1)
typedef uint8_t CONFIG_DATASOURCE;
typedef uint8_t CONFIG_RGB_EFFECT;
typedef uint8_t CONFIG_RGB_FRAMEPROCESS;

typedef uint8_t  PS2_PIN;
typedef uint8_t  PS2_INVERT;
typedef uint8_t  RGB_PIN;
typedef uint16_t PS2_INPUT;
typedef uint8_t  INPUT_PIN_INDEX;
typedef uint8_t  ANALOG_PIN_INDEX;
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
      CONFIG_DATASOURCE map_lx_neg;
      CONFIG_DATASOURCE map_rx_neg;
      CONFIG_DATASOURCE map_ly_neg;
      CONFIG_DATASOURCE map_ry_neg;
      CONFIG_DATASOURCE map_lx_pos;
      CONFIG_DATASOURCE map_rx_pos;
      CONFIG_DATASOURCE map_ly_pos;
      CONFIG_DATASOURCE map_ry_pos;
      CONFIG_DATASOURCE map_button[16];
    };
  };
} ConfigUser_USBMap_t;
typedef struct {
  CONFIG_DATASOURCE source;
  PS2_INPUT         output;
} ConfigUser_PS2Map_t;
typedef struct {
  uint8_t hid_timeout;
  CONFIG_DATASOURCE channels[16];
} ConfigUser_Out_t;
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
  ConfigUser_Out_t      Out;
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

#define CONFIG_HEADER "USBMNIv3"
#pragma pack(pop)