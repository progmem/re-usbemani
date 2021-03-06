#include "usbemani.h"

// Keep track of whether HID lighting is active or not
int main(void) {
  SetupHardware();

  GlobalInterruptEnable();
  USB_Init();

  for (;;) {
    // USB-related tasks
    HID_Task();
    USB_USBTask();
    // Process incoming input
    Input_Task();
    Analog_Task();
    Rotary_Task();
    Output_Task();
    // Handle the preparation of new data for the PS2
    PS2_Task();

    // Update traditional lighting
    if (!OutputList_Timer())
      Output_Set(OutputList_Build());

    // Update RGB lighting
    if (RGB_Ready()) {
      RGB_ProcessFrame();
      Effect_QueueDeferred();
      Effect_Run();
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
        // USBemani_Input_t InputReport;
        // CreateInputReport(&InputReport);
        // Endpoint_ClearSETUP();
        // Endpoint_Write_Control_Stream_LE(&InputReport, sizeof(USBemani_Input_t));
        // Endpoint_ClearOUT();
        USBemani_Input_t *InputReport = InputList_BuildReport();
        Endpoint_ClearSETUP();
        Endpoint_Write_Control_Stream_LE(InputReport, sizeof(USBemani_Input_t));
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
  Output_Set(Report->Lights);
  OutputList_ResetTimer();
}

/*
void CreateInputReport(USBemani_Input_t *Report) {
  memset(Report, 0, sizeof(USBemani_Input_t));

  Report->Slider  = Input_GetRotaryLogicalPosition(0);
  Report->Dial    = Input_GetRotaryLogicalPosition(1);
  Report->Wheel   = Input_GetRotaryLogicalPosition(2);
  Report->Z       = Input_GetRotaryLogicalPosition(3);
  Report->RZ      = Input_GetRotaryLogicalPosition(4);

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

  Report->Button  = Input_GetButtons();

}
*/

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
    // USBemani_Input_t InputReport;
    // CreateInputReport(&InputReport);
    // Endpoint_Write_Stream_LE(&InputReport, sizeof(USBemani_Input_t), NULL);
    USBemani_Input_t *InputReport = InputList_BuildReport();
    Endpoint_Write_Stream_LE(InputReport, sizeof(USBemani_Input_t), NULL);
    Endpoint_ClearIN();
  }
}

void Input_ExecuteOnInterrupt() {
  OutputList_TimerTick();
  RGB_Transmit();
}