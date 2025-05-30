#include <stdint.h>
#include "mpsse_jtag_port.h"


#define MPSSE_IDLE			0
#define MPSSE_RCV_LENGTH_L	1
#define MPSSE_RCV_LENGTH_H	2
#define MPSSE_TRANSMIT		3
#define MPSSE_TMS_OUT		4
#define MPSSE_TCK_OUT		5
#define MPSSE_RCV0			6
#define MPSSE_RCV1			7
#define MPSSE_ERROR			8


#define CF_WCLK_F		0x01
#define CF_BIT			0x02
#define CF_RCLK_F		0x04
#define CF_LSB			0x08
#define CF_WTDI			0x10
#define CF_RTDO			0x20
#define CF_WTMS			0x40

#define CTRL_LOOPBACK	0x01
#define CTRL_DIV5		0x02
#define CTRL_PHASE3		0x04
#define CTRL_RTCK		0x08


static int m_state;
static int m_cmd;
static int m_len;
static int m_div;
static int m_ctrl = CTRL_DIV5;
static uint8_t m_buf[4];


static int jtag_delay_value = 1;

static void jtag_set_delay(int div)
{
	static uint8_t delay_6M[6] = {1, 5, 8, 12, 16, 20};		// freq = 6M/(1+div);
	static uint8_t delay_30M[16] = {1, 1, 1, 1, 1, 2, 3, 4, 4, 5, 6, 7, 8, 8, 8, 9};	// freq = 30M/(1+div);
	
	if(m_ctrl & CTRL_DIV5)
	{
		if(div > 5)
			div = 5;
		
		jtag_delay_value = delay_6M[div];
	}
	else
	{
		if(div > 15)
			div = 15;
		
		jtag_delay_value = delay_30M[div];
	}
}


// +ve: 在clk上升沿将数据发出。对方将在下降沿看到数据。clk空闲时为高。---- --|__|-
// -ve: 在clk下降沿将数据发出。对方将在上升沿看到数据。clk空闲时为低。____ __|--|_
extern void JTAG_write(int byte);
extern void JTAG_flush_resp(void);
int JTAG_execute(uint8_t *req, int req_size)
{
	uint32_t (*jtag_xfer_func)(uint32_t data, uint32_t nbit, uint32_t delay) = 0;
	int rp = 0;

	while(rp<req_size){
		switch(m_state){
		case MPSSE_IDLE:
			m_cmd = req[rp++];
			if(m_cmd&0x80){
				switch(m_cmd){
				case 0x80:
				case 0x82:
					m_state = MPSSE_RCV0;
					break;
				case 0x81:
				case 0x83:
					JTAG_write(JTAG_get_pins((m_cmd>>1)&1));
					break;
				case 0x84:
					m_ctrl |=  CTRL_LOOPBACK;
					break;
				case 0x85:
					m_ctrl &= ~CTRL_LOOPBACK;
					break;
				case 0x86: // Set TCK div
					m_state = MPSSE_RCV0;
					break;
				case 0x87: // Flush buffer to pc
					JTAG_flush_resp();
					break;
				case 0x88: // Wait GPIOL1 high
					break;
				case 0x89: // Wait GPIOL1 low
					break;
				case 0x8a:
					m_ctrl &= ~CTRL_DIV5;
					break;
				case 0x8b:
					m_ctrl |=  CTRL_DIV5;
					break;
				case 0x8c:
					break;
				case 0x8d:
					break;
				case 0x8e:
					m_state = MPSSE_RCV0;
					break;
				case 0x8f:
					m_state = MPSSE_RCV0;
					break;
				case 0x96:
					break;
				case 0x97:
					break;
				default:
					JTAG_write(0xfa);
					JTAG_write(m_cmd);
					break;
				}
			}else{
				int type = m_cmd&0x09;

				jtag_xfer_func = 0;
				if(m_cmd&CF_WTMS){
					if(type==0x08){
						jtag_xfer_func = JTAG_xfer_tms_lsb_1;
					}else if(type==0x09){
						jtag_xfer_func = JTAG_xfer_tms_lsb_0;
					}
				}else if(type==0x00){
					jtag_xfer_func = JTAG_xfer_tdi_msb_1;
				}else if(type==0x01){
					jtag_xfer_func = JTAG_xfer_tdi_msb_0;
				}else if(type==0x08){
					jtag_xfer_func = JTAG_xfer_tdi_lsb_1;
				}else if(type==0x09){
					jtag_xfer_func = JTAG_xfer_tdi_lsb_0;
				}
				if(jtag_xfer_func==0){
					JTAG_write(0xfa);
					JTAG_write(m_cmd);
				}else{
					m_state = MPSSE_RCV_LENGTH_L;
				}
			}
			break;
		case MPSSE_RCV0:
			m_buf[0] = req[rp++];
			m_buf[1] = 0;
			if(m_cmd==0x8e){
				m_state = MPSSE_TCK_OUT;
			}else{
				m_state = MPSSE_RCV1;
			}
			break;
		case MPSSE_RCV1:
			m_buf[1] = req[rp++];
			if(m_cmd==0x86){
				m_div = (m_buf[1]<<8) | m_buf[0];
				jtag_set_delay(m_div);
				m_state = MPSSE_IDLE;
			}else if(m_cmd==0x8f){
				m_state = MPSSE_TCK_OUT;
			}else{
				// 0x80 or 0x82
				JTAG_set_pins((m_cmd>>1)&1, m_buf[0], m_buf[1]);
				m_state = MPSSE_IDLE;
			}
			break;
		case MPSSE_RCV_LENGTH_L:
			m_len = req[rp++];
			if(m_cmd&CF_BIT){
				if(m_cmd&CF_WTMS){
					m_state = MPSSE_TMS_OUT;
				}else{
					m_state = MPSSE_TRANSMIT;
				}
			}else{
				m_state = MPSSE_RCV_LENGTH_H;
			}
			break;
		case MPSSE_RCV_LENGTH_H:
			m_len |= (req[rp++]<<8);
			if(m_cmd&CF_WTMS){
				m_state = MPSSE_TMS_OUT;
			}else{
				m_state = MPSSE_TRANSMIT;
			}
			break;
		case MPSSE_TRANSMIT:
		{
			int bcnt = (m_cmd&CF_BIT)? m_len+1 : 8;
			int wdata = 0xff, rdata;

			if(m_cmd&CF_WTDI){
				wdata = req[rp++];
			}

			rdata = jtag_xfer_func(wdata, bcnt, jtag_delay_value);

			if(m_cmd&CF_RTDO){
				JTAG_write(rdata);
			}
			if((m_cmd&CF_BIT) || (m_len==0)){
				m_state = MPSSE_IDLE;
			}
			m_len -= 1;
			break;
		}
		case MPSSE_TMS_OUT:
		{
			int bcnt = m_len+1;
			int wdata = req[rp++];
			int rdata;

			JTAG_set_tdi(wdata&0x80);

			rdata = jtag_xfer_func(wdata, bcnt, jtag_delay_value);

			if(m_cmd&CF_RTDO){
				JTAG_write(rdata);
			}
			m_state = MPSSE_IDLE;
			break;
		}
		case MPSSE_TCK_OUT:
		{
			m_len = (m_buf[1]<<8) | m_buf[0];
			m_len += 1;
			int bcnt = (m_cmd==0x8e)? m_len : m_len*8;

			for(int i=0; i<bcnt; i++){
				JTAG_set_tck(0);
				for(int j=0; j<jtag_delay_value; j++){
					__NOP();
					__NOP();
				}
				JTAG_set_tck(1);
				for(int j=0; j<jtag_delay_value; j++){
					__NOP();
					__NOP();
				}
			}
			JTAG_set_tck(0);
			m_state = MPSSE_IDLE;
			break;
		}
		default:
			break;
		}

		if(m_state==MPSSE_ERROR){
			m_state = MPSSE_IDLE;
			break;
		}
	}

	return rp;
}
