// Define your device configuration here
Config_t EEMEM eeprom = {
  .Header = {
    .string = "USBMNIv3"
  },
  .Device = {
    .PS2 = {
      .pin    = PS2_C6,
      .invert = PS2_TRANSISTOR,
    },
    .RGB = {
      .pin      = RGB_C7,
      .quantity = 60,
    },
    .Input = {
      .pin = {PIN_D0, PIN_D1, PIN_D2, PIN_D3,
              PIN_D4, PIN_D5, PIN_D6, PIN_D7,
              PIN_B4, PIN_B5, PIN_B6, PIN_B7,
              PIN_NC}
    },
    .Output = {
      .pin = {(PIN_F7 | OUT_LATCH), PIN_NC},
    },
    .Encoder = {
      {.pin_a = PIN_F0, .pin_b = PIN_F1, .ppr = 50},
      {.pin_a = PIN_NC},
    },
    .Analog = {
      .pin = {ANALOG_F6, ANALOG_NC},
    }
  },
  .User = {
    .USBMap = {
      .map_button = {
        CONFIG_DATASOURCE_DIGITAL |  0,
        CONFIG_DATASOURCE_DIGITAL |  1,
        CONFIG_DATASOURCE_DIGITAL |  2,
        CONFIG_DATASOURCE_DIGITAL |  3,
        CONFIG_DATASOURCE_DIGITAL |  4,
        CONFIG_DATASOURCE_DIGITAL |  5,
        CONFIG_DATASOURCE_DIGITAL |  6,
        CONFIG_DATASOURCE_DIGITAL |  7,
        CONFIG_DATASOURCE_DIGITAL |  8,
        CONFIG_DATASOURCE_DIGITAL |  9,
        CONFIG_DATASOURCE_DIGITAL |  10,
        CONFIG_DATASOURCE_DIGITAL |  11,
        CONFIG_DATASOURCE_NC,
      },
      .map_ly_neg = CONFIG_DATASOURCE_ENCODER_CCW | 0,
      .map_ly_pos = CONFIG_DATASOURCE_ENCODER_CW  | 0,
      .map_lx_neg = CONFIG_DATASOURCE_ENCODER_CCW | 1,
      .map_lx_pos = CONFIG_DATASOURCE_ENCODER_CW  | 1,
    },
    .PS2Map = {
      {.source = CONFIG_DATASOURCE_DIGITAL      | 0, .output = PS2_SQUARE},
      {.source = CONFIG_DATASOURCE_DIGITAL      | 1, .output = PS2_L1},
      {.source = CONFIG_DATASOURCE_DIGITAL      | 2, .output = PS2_CROSS},
      {.source = CONFIG_DATASOURCE_DIGITAL      | 3, .output = PS2_R1},
      {.source = CONFIG_DATASOURCE_DIGITAL      | 4, .output = PS2_CIRCLE},
      {.source = CONFIG_DATASOURCE_DIGITAL      | 5, .output = PS2_L2},
      {.source = CONFIG_DATASOURCE_DIGITAL      | 6, .output = PS2_LEFT},
      {.source = CONFIG_DATASOURCE_DIGITAL      | 7, .output = PS2_SELECT},
      {.source = CONFIG_DATASOURCE_DIGITAL      | 8, .output = PS2_START},
      {.source = CONFIG_DATASOURCE_ENCODER_CCW  | 0, .output = PS2_UP},
      {.source = CONFIG_DATASOURCE_ENCODER_CW   | 0, .output = PS2_DOWN},
      {.source = CONFIG_DATASOURCE_NC}
    },
    .HIDOut = {
      .timeout = 60,
    },
    .Encoder = {
      {.hold_time = 1000, .target_max = 256, .target_rot = 144},
    },
    .Analog = {
      {.trigger = 128, .release = 64},
    },
    .Effect = {
      // Turntable
      {
        .trigger  = CONFIG_DATASOURCE_ALWAYS,
        .effect   = CONFIG_EFFECT_MULTICOLOR | CONFIG_COLOR_PROVIDER_RAINBOW,
        .start    = 0,
        .size     = 24,
        .quantity = 24,
        .source   = CONFIG_DATASOURCE_ENCODER | 0
      },
      // Keys 1 - 9
      {
        .trigger  = CONFIG_DATASOURCE_DIGITAL | 7,
        .effect   = CONFIG_EFFECT_SPLASH | CONFIG_COLOR_PROVIDER_HUE,
        .start    = 24,
        .size     = 4,
        .hue      = (((65535 / 9) * 0) >> 8)
      },
      {
        .trigger  = CONFIG_DATASOURCE_DIGITAL | 8,
        .effect   = CONFIG_EFFECT_SPLASH | CONFIG_COLOR_PROVIDER_HUE,
        .start    = 28,
        .size     = 4,
        .hue      = (((65535 / 9) * 1) >> 8)
      },
      {
        .trigger  = CONFIG_DATASOURCE_DIGITAL | 0,
        .effect   = CONFIG_EFFECT_SPLASH | CONFIG_COLOR_PROVIDER_HUE,
        .start    = 32,
        .size     = 4,
        .hue      = (((65535 / 9) * 2) >> 8)
      },
      {
        .trigger  = CONFIG_DATASOURCE_DIGITAL | 1,
        .effect   = CONFIG_EFFECT_SPLASH | CONFIG_COLOR_PROVIDER_HUE,
        .start    = 36,
        .size     = 4,
        .hue      = (((65535 / 9) * 3) >> 8)
      },
      {
        .trigger  = CONFIG_DATASOURCE_DIGITAL | 2,
        .effect   = CONFIG_EFFECT_SPLASH | CONFIG_COLOR_PROVIDER_HUE,
        .start    = 40,
        .size     = 4,
        .hue      = (((65535 / 9) * 4) >> 8)
      },
      {
        .trigger  = CONFIG_DATASOURCE_DIGITAL | 3,
        .effect   = CONFIG_EFFECT_SPLASH | CONFIG_COLOR_PROVIDER_HUE,
        .start    = 44,
        .size     = 4,
        .hue      = (((65535 / 9) * 5) >> 8)
      },
      {
        .trigger  = CONFIG_DATASOURCE_DIGITAL | 4,
        .effect   = CONFIG_EFFECT_SPLASH | CONFIG_COLOR_PROVIDER_HUE,
        .start    = 48,
        .size     = 4,
        .hue      = (((65535 / 9) * 6) >> 8)
      },
      {
        .trigger  = CONFIG_DATASOURCE_DIGITAL | 5,
        .effect   = CONFIG_EFFECT_SPLASH | CONFIG_COLOR_PROVIDER_HUE,
        .start    = 52,
        .size     = 4,
        .hue      = (((65535 / 9) * 7) >> 8)
      },
      {
        .trigger  = CONFIG_DATASOURCE_DIGITAL | 6,
        .effect   = CONFIG_EFFECT_SPLASH | CONFIG_COLOR_PROVIDER_HUE,
        .start    = 56,
        .size     = 4,
        .hue      = (((65535 / 9) * 8) >> 8)
      },
      {.trigger = CONFIG_DATASOURCE_NC},
    }
  },
};