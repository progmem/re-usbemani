#pragma once
#include <LUFA/Drivers/USB/USB.h>
#include <avr/pgmspace.h>

typedef struct {
  USB_Descriptor_Configuration_Header_t Config;

  USB_Descriptor_Interface_t            HID_Interface;
  USB_HID_Descriptor_HID_t              HID_USBemani;
  USB_Descriptor_Endpoint_t             HID_INEndpoint;
  USB_Descriptor_Endpoint_t             HID_OUTEndpoint;
} USB_Descriptor_Configuration_t;

enum InterfaceDescriptors_t {
  INTERFACE_ID_USBemani = 0,
};
enum StringDescriptors_t {
  STRING_ID_Language     = 0,
  STRING_ID_Manufacturer = 1,
  STRING_ID_Product      = 2,
};

#define EPADDR_IN         (ENDPOINT_DIR_IN  | 1)
#define EPADDR_OUT        (ENDPOINT_DIR_OUT | 2)
#define ENDPOINT_SIZE     8

uint16_t CALLBACK_USB_GetDescriptor(
  const uint16_t wValue,
  const uint16_t wIndex,
  const void** const DescriptorAddress,
  uint8_t* const DescriptorMemorySpace
) ATTR_WARN_UNUSED_RESULT ATTR_NON_NULL_PTR_ARG(3);
