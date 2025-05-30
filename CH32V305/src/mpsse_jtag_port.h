#ifndef __MPSSE_JTAG_PORT_H__
#define __MPSSE_JTAG_PORT_H__


#define __NOP()	asm("nop")


void JTAG_setup(void);
void JTAG_close(void);
void JTAG_set_tck(uint32_t val);
void JTAG_set_tdi(uint32_t val);
void JTAG_set_pins(uint32_t id, uint32_t val, uint32_t dir);
uint32_t JTAG_get_pins(uint32_t id);
uint32_t JTAG_xfer_tms_lsb_0(uint32_t data, uint32_t nbit, uint32_t delay);
uint32_t JTAG_xfer_tms_lsb_1(uint32_t data, uint32_t nbit, uint32_t delay);
uint32_t JTAG_xfer_tdi_lsb_0(uint32_t data, uint32_t nbit, uint32_t delay);
uint32_t JTAG_xfer_tdi_lsb_1(uint32_t data, uint32_t nbit, uint32_t delay);
uint32_t JTAG_xfer_tdi_msb_0(uint32_t data, uint32_t nbit, uint32_t delay);
uint32_t JTAG_xfer_tdi_msb_1(uint32_t data, uint32_t nbit, uint32_t delay);


#endif
