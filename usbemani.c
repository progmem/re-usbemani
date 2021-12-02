#include "usbemani.h"

// Keep track of whether HID lighting is active or not
uint16_t HIDLightingActive = 0;

int main(void) {
  SetupHardware();
  SetupEffects();

  GlobalInterruptEnable();
  USB_Init();

  // Test color for analog
  ColorProvider_t *rainbow_cp = ColorProvider_RainbowVariable(1, Input_PtrAnalog(0), 255);
  Effect_t        *rainbow    = Effect_Single(rainbow_cp, 120, 24);
  Effect_AutoQueue(rainbow);

  ColorProvider_t *rainbow_cp2 = ColorProvider_RainbowVariable(1, Input_PtrAnalog(1), 255);
  Effect_t        *rainbow2    = Effect_Single(rainbow_cp2, 96, 24);
  Effect_Defer(rainbow2, Input_PtrAnalogDigital(), (1<<1));

  // Effect_AutoQueue(rainbow2);

  for (;;) {
    // USB-related tasks
    HID_Task();
    USB_USBTask();
    // Process incoming input
    Input_Task();
    Analog_Task();
    Output_Task();
    // Handle the preparation of new data for the PS2
    PS2_Task();

    // Update traditional lighting
    if (HIDLightingActive == 0) {
      Output_Set(Input_GetButtons());
      PORTE &= ~0x40;
    }

    // Update RGB lighting
    if (RGB_Ready()) {
      // Fade every LED in the framebuffer by a random amount
      RGB_FadeRangeRandom(0, 144);
      // Queue any deferred effects based on available input
      Effect_QueueDeferred();
      // Process the current effect queue, drawing to the framebuffer
      Effect_Run();
      // Sync the framebuffer; no more updates can be made until the strip has been rendered out
      RGB_Sync();
    }
  }
}

void SetupHardware(void) {
  //// Internals
  // Diagnostic LED should be set to an output
  DDRE |= 0x40;
  // Watchdog should be disabled
  MCUSR &= ~(1 << WDRF);
  wdt_disable();
  // Clock should not be prescaled
  clock_prescale_set(clock_div_1);

  Config_LoadFromEEPROM();
  return;

  /*
  //// Rotary Encoders
  // Up to 5 rotary encoder slots are available.
  // The first encoder registers to index 0, etc.
  // Pins on port B, D, and F are available for this usage.
  Input_RegisterRotary(PIN_F0, PIN_F1, 50, 2000);
  // Input_RegisterRotary(PIN_F4, PIN_F5, 50, 2000);
  // If desired, enter the following:
  // * The target encoder
  // * The max value in a particular game (for IIDX, 256; for SDVX, 1024)
  // * The value you would like to see after a full rotation (72 for 60Hz IIDX, 144 for 120Hz IIDX)
  // USBemani will scale the encoder output appropriately to meet the target as close as possible.
  Input_RotaryLogicalTarget(0, 256, 144);
  Input_RotaryLogicalTarget(1, 256, 72);

  //// Buttons
  // Each button is registered in the order you wish to use it.
  // Pins on port B, D, and F are available for this usage.
  Input_RegisterButton(PIN_D0);
  Input_RegisterButton(PIN_D1);
  Input_RegisterButton(PIN_D2);
  Input_RegisterButton(PIN_D3);
  Input_RegisterButton(PIN_D4);
  Input_RegisterButton(PIN_D5);
  Input_RegisterButton(PIN_D6);
  Input_RegisterButton(PIN_D7);
  Input_RegisterButton(PIN_B4);
  Input_RegisterButton(PIN_B5);
  Input_RegisterButton(PIN_B6);
  Input_RegisterButton(PIN_B7);

  Input_RegisterAnalog(PIN_F5, ANALOG_NORMAL);
  Input_RegisterAnalog(PIN_F4, ANALOG_NORMAL);
  Input_AnalogDigitalThresholds(0, 128, 64);
  Input_AnalogDigitalThresholds(1, 128, 64);
  // USBemani features two output methods for traditional LED lighting.
  // Pins on port B, D, and F are available for these usages:
  // * Direct, requiring one pin per light (12 buttons + 12 lights = 24 pins)
  // Output_RegisterPin(...);
  // Output_RegisterPin(...);
  // Output_RegisterPin(...);
  // * Latched, requiring the following:
  //   * A set of latch chips and resistors (to be detailed in README)
  //   * One additional pin for the latch itself
  Output_RegisterLatch(PIN_F7);

  //// PS2 Output
  // PS2 output _requires_ the SPI pins (B0-B3), in addition to pin C6 or C7.
  // Do _not_ use the SPI pins for inputs as well!
  // To map a given input to a given button on the PS2, pass the following:
  // * A pointer to an input to monitor.
  // * A mask to one or more bits to check.
  // * One or more buttons to be pressed on the PS2.
  // For multiple buttons, logical OR them together, e.g. PS2_L1 | PS2_L2
  // The following is a sample mapping for a IIDX KOC.
  PS2_MapInput(Input_PtrButtons(), (1 << 0), PS2_SQUARE);
  PS2_MapInput(Input_PtrButtons(), (1 << 1), PS2_L1);
  PS2_MapInput(Input_PtrButtons(), (1 << 2), PS2_CROSS);
  PS2_MapInput(Input_PtrButtons(), (1 << 3), PS2_R1);
  PS2_MapInput(Input_PtrButtons(), (1 << 4), PS2_CIRCLE);
  PS2_MapInput(Input_PtrButtons(), (1 << 5), PS2_L2);
  PS2_MapInput(Input_PtrButtons(), (1 << 6), PS2_LEFT);
  PS2_MapInput(Input_PtrButtons(), (1 << 7), PS2_SELECT);
  PS2_MapInput(Input_PtrButtons(), (1 << 8), PS2_START);
  PS2_MapInput(Input_PtrRotaryDirection(0), INPUT_ROTARY_CCW, PS2_UP);
  PS2_MapInput(Input_PtrRotaryDirection(0), INPUT_ROTARY_CW,  PS2_DOWN);

  //// Initializations
  // USBemani features 4 polling frequencies for input: 1, 2, 4, and 8 kHz.
  // It is advised to leave this at the default 8kHz.
  InputOutput_Begin(INPUT_FREQ_8KHZ);
  // The PS2 "Acknowledge" line is available on either C6 or C7.
  // Indicate which pin you want to use here.
  // Additionally, indicate how MISO will be used for the PS2.
  // * PS2_TRANSISTOR is the most reliable method, requiring an N-Channel MOSFET or transistor.
  //    * Connect the AVR MISO pin to the base/gate.
  //    * Connect the PS2 MISO pin to the collector/drain.
  //    * Connect the emitter/source to ground.
  //    * The BS170 N-Channel MOSFET works well with no gate resistor.
  // * PS2_DIRECT can be used if running at 3.3V.
  //    * This will provide varying levels of success based on a number of factors.
  //    * Cable quality and the presence of a ferrite core are factors that come to play.
  //    * A generic PS1 extension with no ferrites can work better than Konami's cable!
  //    * If PS2_DIRECT doesn't work for you, you must use PS2_TRANSISTOR.
  PS2_Init(PS2_C6, PS2_TRANSISTOR);
  // RGB is available on C6 or C7.
  // Indicate which pin and the number of LEDs to be used.
  RGB_Init(RGB_C7, 144);
  */
}

void SetupEffects(void) {
  /*** Other effect examples
  // Basic hue. Lights up as long as the button is held.
  ColorProvider_t *key1_cp = ColorProvider_Hue(224);
  Effect_t        *key1    = Effect_Single(key1_cp, 28, 4);
  Effect_Defer(key1, Input_PtrButtons(), (1<<0));

  // Light up -only- on press, not on hold.
  ColorProvider_t *key2_cp = ColorProvider_Hue(160);
  Effect_t        *key2    = Effect_SingleShot(key2_cp, 32, 4);
  Effect_Defer(key2, Input_PtrButtons(), (1<<1));

  // Cycle colors. On press, spawn a splash effect with the same color.
  ColorProvider_t *key3_cp = ColorProvider_HueCycle(0, 49);
  Effect_t        *key3    = Effect_Splash(key3_cp, 36, 4);
  Effect_Defer(key3, Input_PtrButtons(), (1<<2));

  // Cycle hue as long as the button is held.
  ColorProvider_t *key4_cp = ColorProvider_HueCycle(0, 7);
  Effect_t        *key4    = Effect_Single(key4_cp, 40, 4);
  Effect_Defer(key4, Input_PtrButtons(), (1<<3));

  // Cycle hue only when the button is pressed. Hold the hue as long as the button is held.
  ColorProvider_t *key5_cp = ColorProvider_HueCycle(0, 16);
  Effect_t        *key5    = Effect_Press(key5_cp, 44, 4);
  Effect_Defer(key5, Input_PtrButtons(), (1<<4));

  // Display a random hue.
  ColorProvider_t *key6_cp = ColorProvider_HueRandom();
  Effect_t        *key6    = Effect_Press(key6_cp, 48, 4);
  Effect_Defer(key6, Input_PtrButtons(), (1<<5));
  ****/

  // Turntable Ring, rainbow
  ColorProvider_t *rainbow_cp = ColorProvider_RainbowVariable(
    24, Input_PtrRotaryPhysicalPosition(0), Input_GetRotaryMaximum(0)
  );
  Effect_t        *rainbow    = Effect_Multi(rainbow_cp, 0, 24);
  Effect_AutoQueue(rainbow);

  // Splash for all buttons
  for (int i = 0; i < 12; i++) {
    ColorProvider_t *color = ColorProvider_Hue(
      ((65535 / 12) * i) >> 8
    );
    Effect_t    *key = Effect_Splash(color, 28 + (i << 2), 4);
    Effect_Defer(key, Output_Ptr(), 1<<i);
  }
}

void EVENT_USB_Device_Connect(void) {
}

void EVENT_USB_Device_Disconnect(void) {
}

void EVENT_USB_Device_ConfigurationChanged(void) {
  Endpoint_ConfigureEndpoint(EPADDR_IN,  EP_TYPE_INTERRUPT, ENDPOINT_SIZE, 1);
  Endpoint_ConfigureEndpoint(EPADDR_OUT, EP_TYPE_INTERRUPT, ENDPOINT_SIZE, 1);
}

void EVENT_USB_Device_ControlRequest(void) {
  switch (USB_ControlRequest.bRequest)
  {
    case HID_REQ_GetReport:
      if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) {
        USBemani_Input_t InputReport;

        CreateInputReport(&InputReport);

        Endpoint_ClearSETUP();
        Endpoint_Write_Control_Stream_LE(&InputReport, sizeof(USBemani_Input_t));
        Endpoint_ClearOUT();
      }

      break;
    case HID_REQ_SetReport:
      if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
        USBemani_Output_t OutputReport;

        Endpoint_ClearSETUP();
        Endpoint_Read_Control_Stream_LE(&OutputReport, sizeof(USBemani_Output_t));
        Endpoint_ClearIN();

        ProcessOutputReport(&OutputReport);
      }

      break;
  }
}

void ProcessOutputReport(USBemani_Output_t *Report) {
  HIDLightingActive = 60;
  Output_Set(Report->Lights);

  if (Report->Lights & 1)
    PORTE |=  0x40;
  else
    PORTE &= ~0x40;
}

void CreateInputReport(USBemani_Input_t *Report) {
  memset(Report, 0, sizeof(USBemani_Input_t));

  if (Input_GetRotaryDirection(0)) {
    Report->LX = -100;
    if (Input_GetRotaryDirection(0) == INPUT_ROTARY_CW)
      Report->LX = 100;
  }

  if (Input_GetRotaryDirection(1)) {
    Report->LY = -100;
    if (Input_GetRotaryDirection(1) == INPUT_ROTARY_CW)
      Report->LY = 100;
  }

  Report->Slider  = Input_GetRotaryLogicalPosition(0);
  Report->Dial    = Input_GetRotaryLogicalPosition(1);
  Report->Wheel   = Input_GetRotaryLogicalPosition(2);
  Report->Z       = Input_GetRotaryLogicalPosition(3);
  Report->RZ      = Input_GetRotaryLogicalPosition(4);
  Report->Button  = Input_GetButtons();

}

void HID_Task(void) {
  if (USB_DeviceState != DEVICE_STATE_Configured)
    return;

  Endpoint_SelectEndpoint(EPADDR_OUT);
  if (Endpoint_IsOUTReceived()) {
    if (Endpoint_IsReadWriteAllowed()) {
      USBemani_Output_t OutputReport;
      Endpoint_Read_Stream_LE(&OutputReport, sizeof(USBemani_Output_t), NULL);
      ProcessOutputReport(&OutputReport);
    }
    Endpoint_ClearOUT();
  }

  Endpoint_SelectEndpoint(EPADDR_IN);
  if (Endpoint_IsINReady()) {
    USBemani_Input_t InputReport;
    CreateInputReport(&InputReport);
    Endpoint_Write_Stream_LE(&InputReport, sizeof(USBemani_Input_t), NULL);
    Endpoint_ClearIN();
  }
}

void Input_ExecuteOnInterrupt() {
  if (HIDLightingActive) --HIDLightingActive;
  RGB_Transmit();
}