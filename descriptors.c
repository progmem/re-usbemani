#include "descriptors.h"
#define HID_LED_BLOCK(led_num)  \
  HID_RI_USAGE(8, led_num),     \
  HID_RI_COLLECTION(8, 0x02),   \
    HID_RI_USAGE_PAGE(8, 0x08), \
    HID_RI_USAGE(8, 0x4B),      \
    HID_RI_REPORT_SIZE(8, 1),   \
    HID_RI_REPORT_COUNT(8, 1),  \
    HID_RI_OUTPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_NON_VOLATILE), \
  HID_RI_END_COLLECTION(0)

const USB_Descriptor_HIDReport_Datatype_t PROGMEM USBemaniDeviceReport[] = {
  // Total 334 bytes
  // Start (10 bytes)
	HID_RI_USAGE_PAGE(8, 0x01),           // Generic Desktop
	HID_RI_USAGE(8, 0x04),                // Joystic
	HID_RI_COLLECTION(8, 0x01),           // Application
    HID_RI_USAGE(8, 0x01),              // Pointer
    HID_RI_COLLECTION(8, 0x00),         // Physical
      // Relative inputs (18 bytes)
      HID_RI_USAGE(8, 0x30),            // X
      HID_RI_USAGE(8, 0x31),            // Y
      HID_RI_USAGE(8, 0x33),            // Rx
      HID_RI_USAGE(8, 0x34),            // Ry
      HID_RI_LOGICAL_MINIMUM(8, -100),
      HID_RI_LOGICAL_MAXIMUM(8, 100),
      HID_RI_REPORT_COUNT(8, 4),
      HID_RI_REPORT_SIZE(8, 8),
      HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
	    // Absolute inputs (24 bytes)
      HID_RI_USAGE(8, 0x36),            // Slider
      HID_RI_USAGE(8, 0x37),            // Dial
      HID_RI_USAGE(8, 0x38),            // Wheel
      HID_RI_USAGE(8, 0x32),            // Z
      HID_RI_USAGE(8, 0x35),            // Rz
      HID_RI_LOGICAL_MINIMUM(16, -32768),
      HID_RI_LOGICAL_MAXIMUM(16,  32767),  // Encoder Resolution [bytes 44, 45]
      HID_RI_REPORT_COUNT(8, 5),
      HID_RI_REPORT_SIZE(8, 16),
      HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE | HID_IOF_WRAP | HID_IOF_NO_PREFERRED_STATE),
    HID_RI_END_COLLECTION(0),
    // Buttons (16 bytes)
    HID_RI_USAGE_PAGE(8, 0x09),
    HID_RI_USAGE_MINIMUM(8, 1),
    HID_RI_USAGE_MAXIMUM(8, 16),
    HID_RI_LOGICAL_MINIMUM(8, 0x00),
    HID_RI_LOGICAL_MAXIMUM(8, 0x01),
    HID_RI_REPORT_SIZE(8, 1),
    HID_RI_REPORT_COUNT(8, 16),
    HID_RI_INPUT(8, HID_IOF_DATA | HID_IOF_VARIABLE | HID_IOF_ABSOLUTE),
    // LEDs (256+2 bytes)
    HID_RI_USAGE_PAGE(8, 0x0A),
    HID_LED_BLOCK(1),
    HID_LED_BLOCK(2),
    HID_LED_BLOCK(3),
    HID_LED_BLOCK(4),
    HID_LED_BLOCK(5),
    HID_LED_BLOCK(6),
    HID_LED_BLOCK(7),
    HID_LED_BLOCK(8),
    HID_LED_BLOCK(9),
    HID_LED_BLOCK(10),
    HID_LED_BLOCK(11),
    HID_LED_BLOCK(12),
    HID_LED_BLOCK(13),
    HID_LED_BLOCK(14),
    HID_LED_BLOCK(15),
    HID_LED_BLOCK(16),
    // Command Area (8 bytes)
    HID_RI_REPORT_SIZE(8, 8),
    HID_RI_REPORT_COUNT(8, 2),
    HID_RI_OUTPUT(8, HID_IOF_CONSTANT),
  HID_RI_END_COLLECTION(0),
};

const USB_Descriptor_Device_t PROGMEM DeviceDescriptor = {
	.Header                 = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

	.USBSpecification       = VERSION_BCD(1,1,0),
	.Class                  = USB_CSCP_NoDeviceClass,
	.SubClass               = USB_CSCP_NoDeviceSubclass,
	.Protocol               = USB_CSCP_NoDeviceProtocol,

	.Endpoint0Size          = FIXED_CONTROL_ENDPOINT_SIZE,

	.VendorID               = 0x0573,
	.ProductID              = 0xFFFE,
	.ReleaseNumber          = VERSION_BCD(0,0,1),

	.ManufacturerStrIndex   = STRING_ID_Manufacturer,
	.ProductStrIndex        = STRING_ID_Product,
	.SerialNumStrIndex      = NO_DESCRIPTOR,

	.NumberOfConfigurations = FIXED_NUM_CONFIGURATIONS
};

const USB_Descriptor_Configuration_t PROGMEM ConfigurationDescriptor = {
	.Config = {
    .Header                 = {.Size = sizeof(USB_Descriptor_Configuration_Header_t), .Type = DTYPE_Configuration},

    .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
    .TotalInterfaces        = 1,

    .ConfigurationNumber    = 1,
    .ConfigurationStrIndex  = NO_DESCRIPTOR,

    .ConfigAttributes       = (USB_CONFIG_ATTR_RESERVED | USB_CONFIG_ATTR_SELFPOWERED),

    .MaxPowerConsumption    = USB_CONFIG_POWER_MA(500)
  },

	.HID_Interface = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Interface_t), .Type = DTYPE_Interface},

		.InterfaceNumber        = INTERFACE_ID_USBemani,
		.AlternateSetting       = 0x00,

		.TotalEndpoints         = 2,

		.Class                  = HID_CSCP_HIDClass,
		.SubClass               = HID_CSCP_NonBootSubclass,
		.Protocol               = HID_CSCP_NonBootProtocol,

		.InterfaceStrIndex      = NO_DESCRIPTOR
	},

	.HID_USBemani = {
		.Header                 = {.Size = sizeof(USB_HID_Descriptor_HID_t), .Type = HID_DTYPE_HID},

		.HIDSpec                = VERSION_BCD(1,1,1),
		.CountryCode            = 0x00,
		.TotalReportDescriptors = 1,
		.HIDReportType          = HID_DTYPE_Report,
		.HIDReportLength        = sizeof(USBemaniDeviceReport)
	},

	.HID_INEndpoint = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

		.EndpointAddress        = EPADDR_IN,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = ENDPOINT_SIZE,
		.PollingIntervalMS      = 0x01
	},

	.HID_OUTEndpoint = {
		.Header                 = {.Size = sizeof(USB_Descriptor_Endpoint_t), .Type = DTYPE_Endpoint},

		.EndpointAddress        = EPADDR_OUT,
		.Attributes             = (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
		.EndpointSize           = ENDPOINT_SIZE,
		.PollingIntervalMS      = 0x01
	}
};

const USB_Descriptor_String_t PROGMEM LanguageString = USB_STRING_DESCRIPTOR_ARRAY(LANGUAGE_ID_ENG);
const USB_Descriptor_String_t PROGMEM ManufacturerString = USB_STRING_DESCRIPTOR(L"@progmem");

const USB_Descriptor_String_t PROGMEM ProductString       = USB_STRING_DESCRIPTOR(L"USBemani v3");
      USB_Descriptor_String_t         ProductStringCustom = USB_STRING_DESCRIPTOR(L"Custom String Goes Here!");

uint16_t CALLBACK_USB_GetDescriptor(
  const uint16_t wValue,
  const uint16_t wIndex,
  const void** const DescriptorAddress,
  uint8_t *const 	DescriptorMemorySpace
) {
  *DescriptorMemorySpace = MEMSPACE_FLASH;
	const uint8_t  DescriptorType   = (wValue >> 8);
	const uint8_t  DescriptorNumber = (wValue & 0xFF);

	const void* Address = NULL;
	uint16_t    Size    = NO_DESCRIPTOR;

	switch (DescriptorType)
	{
		case DTYPE_Device:
			Address = &DeviceDescriptor;
			Size    = sizeof(USB_Descriptor_Device_t);
			break;
		case DTYPE_Configuration:
			Address = &ConfigurationDescriptor;
			Size    = sizeof(USB_Descriptor_Configuration_t);
			break;
		case DTYPE_String:
			switch (DescriptorNumber)
			{
				case STRING_ID_Language:
					Address = &LanguageString;
					Size    = pgm_read_byte(&LanguageString.Header.Size);
					break;
				case STRING_ID_Manufacturer:
					Address = &ManufacturerString;
					Size    = pgm_read_byte(&ManufacturerString.Header.Size);
					break;
				case STRING_ID_Product:
					Address = &ProductString;
					Size    = pgm_read_byte(&ProductString.Header.Size);
					break;
			}

			break;
		case HID_DTYPE_HID:
			Address = &ConfigurationDescriptor.HID_USBemani;
			Size    = sizeof(USB_HID_Descriptor_HID_t);
			break;
		case HID_DTYPE_Report:
		  // *DescriptorMemorySpace = MEMSPACE_RAM;
			Address = &USBemaniDeviceReport;
			Size    = sizeof(USBemaniDeviceReport);
			break;
	}

	*DescriptorAddress = Address;
	return Size;
}

