#ifndef __USB_DESC_H__
#define __USB_DESC_H__


#define USBD_VID	0x0403
#define USBD_PID	0x6010


/* EP number */
#define USB_BULK_IN_EP		0x81
#define USB_BULK_OUT_EP		0x02
#define VCP_BULK_IN_EP		0x83
#define VCP_BULK_OUT_EP		0x04


/* EP maximum packet size */
#define USB_MAX_EP0_SZ		64

#define USB_BULK_IN_SZ_FS	64
#define USB_BULK_OUT_SZ_FS	64
#define VCP_BULK_IN_SZ_FS	64
#define VCP_BULK_OUT_SZ_FS	64

#define USB_BULK_IN_SZ_HS	512
#define USB_BULK_OUT_SZ_HS	512
#define VCP_BULK_IN_SZ_HS	512
#define VCP_BULK_OUT_SZ_HS	512


/* Descriptor size */
#define USB_SIZE_DEVICE_DESC		18
#define USB_SIZE_CONFIG_DESC		9
#define USB_SIZE_INTERFACE_DESC		9
#define USB_SIZE_ENDPOINT_DESC		7
#define USB_SIZE_QUALIFIER_DESC		10
#define USB_SIZE_STRING_LANGID		4
#define USB_SIZE_STRING_VENDOR		10
#define USB_SIZE_STRING_PRODUCT		16
#define USB_SIZE_STRING_SERIALNBR	26
#define USB_SIZE_STRING_IFACEJTAG   26
#define USB_SIZE_STRING_IFACEUART   26


#define USB_SIZE_CONFIG_TOTAL	(USB_SIZE_CONFIG_DESC + (USB_SIZE_INTERFACE_DESC + USB_SIZE_ENDPOINT_DESC * 2) * 2)


extern const uint8_t USB_DevDesc[];
extern const uint8_t USB_CfgDesc_FS[];
extern const uint8_t USB_CfgDesc_HS[];
extern const uint8_t USB_StringLangID[];
extern const uint8_t USB_StringVendor[];
extern const uint8_t USB_StringProduct[];
extern		 uint8_t USB_StringSerialNbr[];
extern const uint8_t USB_StringIFaceJTAG[];
extern const uint8_t USB_StringIFaceUART[];
extern const uint8_t USB_QualifierDesc[];


#endif
