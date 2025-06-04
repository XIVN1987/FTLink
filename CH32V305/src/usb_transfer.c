#include <string.h>
#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "ch32v30x_usbhs_device.h"

#include "usb_desc.h"
#include "vcom_serial.h"
#include "usb_transfer.h"


uint8_t jtag_req_buf[USB_BULK_OUT_SZ_HS];
uint8_t jtag_resp_buf[USB_BULK_IN_SZ_HS];
volatile int jtag_req_size;
volatile int jtag_resp_size;

void USBD_Config(void)
{
	jtag_req_size = 0;
	
	jtag_resp_buf[0] = 0x02;
	jtag_resp_buf[1] = 0x60;
	jtag_resp_size = 2;
	
	USBHS_RCC_Init();
    USBHS_Device_Init(ENABLE);
}


void USB_Out_callback(uint8_t data[], uint32_t nbytes)
{
	memcpy(jtag_req_buf, data, nbytes);
	jtag_req_size = nbytes;
}


void USB_In_callback(void)
{
	jtag_resp_size = 2;
}


int JTAG_execute(uint8_t *req, int req_size);
void JTAG_handle(void)
{
	if(jtag_req_size == 0)
		return;
	
	JTAG_execute(jtag_req_buf, jtag_req_size);
	jtag_req_size = 0;

	USBD_RxReady(USB_BULK_OUT_EP);
}


void JTAG_write(int byte)
{
	jtag_resp_buf[jtag_resp_size] = byte;
	jtag_resp_size += 1;
	
	if(jtag_resp_size == USB_BULK_IN_SZ_HS)
	{
		USBD_TxWrite(USB_BULK_IN_EP, jtag_resp_buf, jtag_resp_size);
		while(jtag_resp_size > 2) __NOP();
	}
}

void JTAG_flush_resp(void)
{
	USBD_TxWrite(USB_BULK_IN_EP, jtag_resp_buf, jtag_resp_size);
	while(jtag_resp_size > 2) __NOP();
}


/* Vendor Requests */
#define VREQ_RESET				0x00	// Reset the port
#define VREQ_SET_MODEM_CTRL		0x01	// Set modem control register
#define VREQ_SET_FLOW_CTRL		0x02	// Set flow control register
#define VREQ_SET_BAUDRATE		0x03	// Set baud rate
#define VREQ_SET_DATA			0x04	// Set data characteristics
#define VREQ_POLL_MODEM_STATUS	0x05
#define VREQ_SET_EVENT_CHAR		0x06
#define VREQ_SET_ERROR_CHAR		0x07
#define VREQ_SET_LATENCY_TIMER	0x09
#define VREQ_GET_LATENCY_TIMER	0x0A
#define VREQ_SET_BITMODE		0x0B
#define VREQ_READ_PINS			0x0C
#define VREQ_READ_EEPROM		0x90
#define VREQ_WRITE_EEPROM		0x91
#define VREQ_ERASE_EEPROM		0x92

static const uint16_t ftdi_eeprom_info[] =
{
	0x0800, 0x0403, 0x6010, 0x0500, 0x3280, 0x0000, 0x0200, 0x1096,
	0x1aa6, 0x0000, 0x0046, 0x0310, 0x004f, 0x0070, 0x0065, 0x006e,
	0x002d, 0x0045, 0x0043, 0x031a, 0x0055, 0x0053, 0x0042, 0x0020,
	0x0044, 0x0065, 0x0062, 0x0075, 0x0067, 0x0067, 0x0065, 0x0072,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x1027 
};


extern void JTAG_setup(void);
extern void JTAG_exit(void);

void USB_VendorRequest(USB_SETUP_REQ * pSetup)
{
	uint16_t eeprom_buf;
	
	int port = pSetup->wIndex & 0xFF;
	if(port==0)
		port = 1;
	
	switch(pSetup->bRequest)
	{
	case VREQ_RESET:
		break;
	
	case VREQ_READ_EEPROM:
		eeprom_buf = pSetup->wIndex < 0x40 ? ftdi_eeprom_info[pSetup->wIndex] : 0x0000;
		memcpy(USBHS_EP0_Buf, &eeprom_buf, 2);
		break;
	
	case VREQ_SET_BITMODE:
		if(port==1)
		{
			if((pSetup->wValue >> 8) == 0x02)
				JTAG_setup();
			else
				JTAG_exit();
		}
		break;
	
	case VREQ_SET_DATA:
		if(port==2)
		{
			/* D0-D7   databits
			 * D8-D10  parity, 0 None, 1 Odd, 2 Even, 3 Mark, 4 Space
			 * D11-D12 stopbits, 0 STOP_BIT_1, 1 STOP_BIT_15, 2 STOP_BIT_2
			 * D14     break, 0 OFF, 1 ON
			 */
			LineCfg.u8DataBits   = (pSetup->wValue >> 0) & 0xFF;
			LineCfg.u8ParityType = (pSetup->wValue >> 8) & 0x07;
			LineCfg.u8CharFormat = (pSetup->wValue >>11) & 0x03;
			VCOM_LineCoding(&LineCfg);
		}
		break;
	
	case VREQ_SET_BAUDRATE:
		if(port==2)
		{
			LineCfg.u32DTERate = 3000000 / (pSetup->wValue & 0x3FFF);
			VCOM_LineCoding(&LineCfg);
		}
		break;
	
	case VREQ_SET_FLOW_CTRL:
		break;
	
	case VREQ_SET_MODEM_CTRL:
		break;
	
	case VREQ_POLL_MODEM_STATUS:
		eeprom_buf = 0x6002;
		memcpy(USBHS_EP0_Buf, &eeprom_buf, 2);
		break;
	
	case VREQ_SET_LATENCY_TIMER:
		break;
	
	case VREQ_GET_LATENCY_TIMER:
		USBHS_EP0_Buf[0] = 16;
		break;
	
	case VREQ_SET_EVENT_CHAR:	
	case VREQ_SET_ERROR_CHAR:
	default:
		break;
	}
}
