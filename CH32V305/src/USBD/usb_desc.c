#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "ch32v30x_usbhs_device.h"

#include "usb_desc.h"


/* Device Descriptor */
const uint8_t USB_DevDesc[] =
{
	USB_SIZE_DEVICE_DESC,				// bLength
	USB_DESCR_TYP_DEVICE,				// bDescriptorType
	0x00, 0x02,							// bcdUSB
	0x00,								// bDeviceClass
	0x00,								// bDeviceSubClass
	0x00,								// bDeviceProtocol
	USB_MAX_EP0_SZ,						// bMaxPacketSize0
	USBD_VID & 0xFF, USBD_VID >> 8,		// idVendor
	USBD_PID & 0xFF, USBD_PID >> 8,		// idProduct
	0x00, 0x05,							// bcdDevice
	0x01,								// iManufacture
	0x02,								// iProduct
	0x03,								// iSerialNumber
	0x01								// bNumConfigurations
};


/* Configuration Descriptor (FS) */
const uint8_t USB_CfgDesc_FS[] =
{
	USB_SIZE_CONFIG_DESC,				// bLength
	USB_DESCR_TYP_CONFIG,				// bDescriptorType
	USB_SIZE_CONFIG_TOTAL & 0xFF,
	USB_SIZE_CONFIG_TOTAL >> 8,			// wTotalLength
	2,									// bNumInterfaces
	0x01,								// bConfigurationValue
	0x00,								// iConfiguration
	0x80,								// bmAttributes, D6: self power  D5: remote wake-up
	0xFA,								// MaxPower, 250 * 2mA = 500mA

	// I/F descriptor: JTAG
	USB_SIZE_INTERFACE_DESC,			// bLength
	USB_DESCR_TYP_INTERF,				// bDescriptorType
	0x00,								// bInterfaceNumber
	0x00,								// bAlternateSetting
	0x02,								// bNumEndpoints
	0xFF,								// bInterfaceClass
	0xFF,								// bInterfaceSubClass
	0xFF,								// bInterfaceProtocol
	0x04,								// iInterface

	// EP Descriptor
	USB_SIZE_ENDPOINT_DESC,				// bLength
	USB_DESCR_TYP_ENDP,					// bDescriptorType
	USB_BULK_IN_EP,						// bEndpointAddress
	USB_EPT_BULK,						// bmAttributes
	USB_BULK_IN_SZ_FS, 0x00,			// wMaxPacketSize
	0x00,								// bInterval

	// EP Descriptor
	USB_SIZE_ENDPOINT_DESC,				// bLength
	USB_DESCR_TYP_ENDP,					// bDescriptorType
	USB_BULK_OUT_EP,					// bEndpointAddress
	USB_EPT_BULK,						// bmAttributes
	USB_BULK_OUT_SZ_FS, 0x00,			// wMaxPacketSize
	0x00,								// bInterval

	// I/F descriptor: UART
	USB_SIZE_INTERFACE_DESC,			// bLength
	USB_DESCR_TYP_INTERF,				// bDescriptorType
	0x01,								// bInterfaceNumber
	0x00,								// bAlternateSetting
	0x02,								// bNumEndpoints
	0xFF,								// bInterfaceClass
	0xFF,								// bInterfaceSubClass
	0xFF,								// bInterfaceProtocol
	0x05,								// iInterface

	// ENDPOINT descriptor
	USB_SIZE_ENDPOINT_DESC,				// bLength
	USB_DESCR_TYP_ENDP,					// bDescriptorType
	VCP_BULK_IN_EP,						// bEndpointAddress
	USB_EPT_BULK,						// bmAttributes
	VCP_BULK_IN_SZ_FS, 0x00,			// wMaxPacketSize
	0x00,								// bInterval

	// ENDPOINT descriptor
	USB_SIZE_ENDPOINT_DESC,				// bLength
	USB_DESCR_TYP_ENDP,					// bDescriptorType
	VCP_BULK_OUT_EP,					// bEndpointAddress
	USB_EPT_BULK,						// bmAttributes
	VCP_BULK_OUT_SZ_FS, 0x00,			// wMaxPacketSize
	0x00,								// bInterval
};


/* Configuration Descriptor (HS) */
const uint8_t USB_CfgDesc_HS[] =
{
	USB_SIZE_CONFIG_DESC,				// bLength
	USB_DESCR_TYP_CONFIG,				// bDescriptorType
	USB_SIZE_CONFIG_TOTAL & 0xFF,
	USB_SIZE_CONFIG_TOTAL >> 8,			// wTotalLength
	2,									// bNumInterfaces
	0x01,								// bConfigurationValue
	0x00,								// iConfiguration
	0x80,								// bmAttributes, D6: self power  D5: remote wake-up
	0xFA,								// MaxPower, 250 * 2mA = 500mA

	// I/F descriptor: JTAG
	USB_SIZE_INTERFACE_DESC,			// bLength
	USB_DESCR_TYP_INTERF,				// bDescriptorType
	0x00,								// bInterfaceNumber
	0x00,								// bAlternateSetting
	0x02,								// bNumEndpoint
	0xFF,								// bInterfaceClass
	0xFF,								// bInterfaceSubClass
	0xFF,								// bInterfaceProtocol
	0x04,								// iInterface

	// EP Descriptor
	USB_SIZE_ENDPOINT_DESC,				// bLength
	USB_DESCR_TYP_ENDP,					// bDescriptorType
	USB_BULK_IN_EP,						// bEndpointAddress
	USB_EPT_BULK,						// bmAttributes
	USB_BULK_IN_SZ_HS & 0xFF,
	USB_BULK_IN_SZ_HS >> 8,				// wMaxPacketSize
	0x00,								// bInterval

	// EP Descriptor
	USB_SIZE_ENDPOINT_DESC,				// bLength
	USB_DESCR_TYP_ENDP,					// bDescriptorType
	USB_BULK_OUT_EP,					// bEndpointAddress
	USB_EPT_BULK,						// bmAttributes
	USB_BULK_OUT_SZ_HS & 0xFF,
	USB_BULK_OUT_SZ_HS >> 8,			// wMaxPacketSize
	0x00,								// bInterval

	// I/F descriptor: UART
	USB_SIZE_INTERFACE_DESC,			// bLength
	USB_DESCR_TYP_INTERF,				// bDescriptorType
	0x01,								// bInterfaceNumber
	0x00,								// bAlternateSetting
	0x02,								// bNumEndpoints
	0xFF,								// bInterfaceClass
	0xFF,								// bInterfaceSubClass
	0xFF,								// bInterfaceProtocol
	0x05,								// iInterface

	// ENDPOINT descriptor
	USB_SIZE_ENDPOINT_DESC,				// bLength
	USB_DESCR_TYP_ENDP,					// bDescriptorType
	VCP_BULK_IN_EP,						// bEndpointAddress
	USB_EPT_BULK,						// bmAttributes
	VCP_BULK_IN_SZ_HS & 0xFF,
	VCP_BULK_IN_SZ_HS >> 8,				// wMaxPacketSize
	0x00,								// bInterval

	// ENDPOINT descriptor
	USB_SIZE_ENDPOINT_DESC,				// bLength
	USB_DESCR_TYP_ENDP,					// bDescriptorType
	VCP_BULK_OUT_EP,					// bEndpointAddress
	USB_EPT_BULK,						// bmAttributes
	VCP_BULK_OUT_SZ_HS & 0xFF,
	VCP_BULK_OUT_SZ_HS >> 8,			// wMaxPacketSize
	0x00,								// bInterval
};


/* Language Descriptor */
const uint8_t USB_StringLangID[] =
{
	USB_SIZE_STRING_LANGID,
	USB_DESCR_TYP_STRING,
	0x09, 0x04
};


/* Manufacturer Descriptor */
const uint8_t USB_StringVendor[] =
{
	USB_SIZE_STRING_VENDOR,
	USB_DESCR_TYP_STRING,
	'F', 0, 'T', 0, 'D', 0, 'I', 0
};


/* Product Descriptor */
const uint8_t USB_StringProduct[] =
{
	USB_SIZE_STRING_PRODUCT,
	USB_DESCR_TYP_STRING,
	'F', 0, 'T', 0, '2', 0, '2', 0, '3', 0, '2', 0, 'H', 0
};


/* Serial Number Descriptor */
uint8_t USB_StringSerialNbr[] =
{
	USB_SIZE_STRING_SERIALNBR,
	USB_DESCR_TYP_STRING,
	'0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, '8', 0, '9', 0, 'A', 0, 'B', 0
};


/* Interface Descriptor: JTAG */
const uint8_t USB_StringIFaceJTAG[] =
{
	USB_SIZE_STRING_IFACEJTAG,
	USB_DESCR_TYP_STRING,
	'F', 0, 'T', 0, '2', 0, '2', 0, '3', 0, '2', 0, 'H', 0, '-', 0, 'J', 0, 'T', 0, 'A', 0, 'G', 0
};


/* Interface Descriptor: UART */
const uint8_t USB_StringIFaceUART[] =
{
	USB_SIZE_STRING_IFACEUART,
	USB_DESCR_TYP_STRING,
	'F', 0, 'T', 0, '2', 0, '2', 0, '3', 0, '2', 0, 'H', 0, '-', 0, 'U', 0, 'A', 0, 'R', 0, 'T', 0
};


/* Device Qualified Descriptor */
const uint8_t USB_QualifierDesc[] =
{
	USB_SIZE_QUALIFIER_DESC,	// bLength
	USB_DESCR_TYP_QUALIF,		// bDescriptorType
	0x00, 0x02,					// bcdUSB
	0x00,						// bDeviceClass
	0x00,						// bDeviceSubClass
	0x00,						// bDeviceProtocol
	0x40,						// bMaxPacketSize0 for other speed
	0x01,						// bNumConfigurations
	0x00,						// bReserved
};
