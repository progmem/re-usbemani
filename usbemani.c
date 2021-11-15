#include "usbemani.h"
uint16_t count = 0;
uint8_t  hid_lights = 0;

void SetupEffects(void) {
  // Turntable Ring, rainbow
  ColorProvider_t *rainbow_cp = ColorProvider_RainbowVariable(
    24, Input_PtrRotaryPhysicalPosition(0), Input_GetRotaryMaximum(0)
  );
  Effect_t        *rainbow    = Effect_Multi(rainbow_cp, 0, 24);
  Effect_AutoQueue(rainbow);

  /* Effect examples
  // Key 1, basic hue
  ColorProvider_t *key1_cp = ColorProvider_Hue(224);
  Effect_t        *key1    = Effect_Single(key1_cp, 28, 4);
  Effect_Defer(key1, Input_PtrButtons(), (1<<0));

  // Key 2, light only on initial press
  ColorProvider_t *key2_cp = ColorProvider_Hue(160);
  Effect_t        *key2    = Effect_SingleShot(key2_cp, 32, 4);
  Effect_Defer(key2, Input_PtrButtons(), (1<<1));

  // Key 3, lights when held, spawns an effect on press
  ColorProvider_t *key3_cp = ColorProvider_HueCycle(0, 49);
  Effect_t        *key3    = Effect_Splash(key3_cp, 36, 4);
  Effect_Defer(key3, Input_PtrButtons(), (1<<2));

  // Key 4, cycles hue on hold
  ColorProvider_t *key4_cp = ColorProvider_HueCycle(0, 7);
  Effect_t        *key4    = Effect_Single(key4_cp, 40, 4);
  Effect_Defer(key4, Input_PtrButtons(), (1<<3));

  // Key 5, cycles hue on press
  ColorProvider_t *key5_cp = ColorProvider_HueCycle(0, 16);
  Effect_t        *key5    = Effect_Press(key5_cp, 44, 4);
  Effect_Defer(key5, Input_PtrButtons(), (1<<4));

  // Key 6, random hue on press
  ColorProvider_t *key6_cp = ColorProvider_HueRandom();
  Effect_t        *key6    = Effect_Press(key6_cp, 48, 4);
  Effect_Defer(key6, Input_PtrButtons(), (1<<5));
  */

  /* Splash on all 9 buttons */
  for (int i = 0; i < 9; i++) {
    ColorProvider_t *color = ColorProvider_Hue(
      ((65535 / 9) * i) >> 8
    );
    Effect_t    *key = Effect_Splash(color, 28 + (i << 2), 4);
    Effect_Defer(key, Output_Ptr(), 1<<i);
  }
}

int main(void) {
  SetupHardware();
  SetupEffects();

  GlobalInterruptEnable();
  USB_Init();

  ColorProvider_t *rainbow_cp = ColorProvider_RainbowVariable(1, RGB_PtrTicks(), 256);
  Effect_t        *rainbow    = Effect_Multi(rainbow_cp, 0, 1);
  Effect_AutoQueue(rainbow);

  for (;;) {
    HID_Task();
    USB_USBTask();
    Input_Task();
    PS2_Task();

    // Update output
    if (!hid_lights)
      Output_Set(Input_GetButtons());

    if (RGB_Ready()) {
      uint8_t ticks = RGB_GetTicks();

      if (ticks & 0x40)
        rainbow->index = 143 - (ticks & 0x3f);
      else
        rainbow->index = 80 + (ticks & 0x3f);

      RGB_FadeRangeRandom(0, 144);
      Effect_QueueDeferred();
      Effect_Run();
      RGB_Sync();
    }
  }
}

// Setup the reference hardware
void SetupReferenceHardware(void) {
  // Rotary Encoders
  Input_RegisterRotary(PIN_F0, PIN_F1, 50, 2000);
  Input_RegisterRotary(PIN_F4, PIN_F5, 50, 2000);
  Input_SetRotaryLogicalTarget(0, 256, 144);
  Input_SetRotaryLogicalTarget(0, 256, 72);

  // Buttons
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
  // Latched Output
  Output_RegisterLatch(PIN_F7);
  // Input Init
  InputOutput_Begin(INPUT_FREQ_8KHZ);

  // RGB Init
  RGB_Init(144);

  // PS2 Inputs
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
  // PS2 Init (with transistor)
  PS2_Init(0xFF);
}

void SetupHardware(void) {
  DDRE |= 0x40;

  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  clock_prescale_set(clock_div_1);

  SetupReferenceHardware();
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

        /* Write the report data to the control endpoint */
        Endpoint_Write_Control_Stream_LE(&InputReport, sizeof(USBemani_Input_t));
        Endpoint_ClearOUT();
      }

      break;
    case HID_REQ_SetReport:
      if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
        USBemani_Output_t OutputReport;

        Endpoint_ClearSETUP();

        /* Read the report data from the control endpoint */
        Endpoint_Read_Control_Stream_LE(&OutputReport, sizeof(USBemani_Output_t));
        Endpoint_ClearIN();

        ProcessOutputReport(&OutputReport);
      }

      break;
  }
}

void ProcessOutputReport(USBemani_Output_t *Report) {
  hid_lights = 30;
  Output_Set(Report->Lights);
}

void CreateInputReport(USBemani_Input_t *Report) {
  memset(Report, 0, sizeof(USBemani_Input_t));

  /*
  Report->LX = ((Input_GetButtons() & 0x01) ? -100 : 0); // Part of wheel
  Report->LY = ((Input_GetButtons() & 0x02) ? -100 : 0); // Part of wheel
  Report->RX = ((Input_GetButtons() & 0x04) ? -100 : 0); // Part of Z Axis
  Report->RY = ((Input_GetButtons() & 0x08) ? -100 : 0); // Part of Z Axis

  Report->Rotary[0] = ((Input_GetButtons() & 0x10) ? 32767 : 0); // Rz
  Report->Rotary[1] = ((Input_GetButtons() & 0x20) ? 32767 : 0); // Buttons
  Report->Rotary[2] = ((Input_GetButtons() & 0x40) ? 32767 : 0); // X/Y
  Report->Rotary[3] = ((Input_GetButtons() & 0x80) ? 32767 : 0);  // X/Y Rotation
  Report->Rotary[4] = ((Input_GetButtons() & 0x100) ? 32767 : 0); // Slider

  Report->Button    = (Input_GetButtons() & 0xFFE0);
  */

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
  if (hid_lights) --hid_lights;
  RGB_Transmit();
}