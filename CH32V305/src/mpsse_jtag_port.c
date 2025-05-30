#include "ch32v30x.h"


#define PIN_TCK		GPIO_Pin_13
#define PIN_TMS		GPIO_Pin_12
#define PIN_TDI		GPIO_Pin_15
#define PIN_TDO		GPIO_Pin_14

#define DIR_OUT 	1  // push-poll output
#define DIR_IN  	4  // float input


void JTAG_setup(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_SetBits(GPIOB, PIN_TCK | PIN_TMS | PIN_TDI);

	GPIO_InitStruct.GPIO_Pin = PIN_TCK | PIN_TMS | PIN_TDI;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = PIN_TDO;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
}

void JTAG_exit(void)
{
}


void JTAG_set_tck(uint32_t val)
{
	if(val) GPIOB->BSHR = PIN_TCK;
	else	GPIOB->BCR  = PIN_TCK;
}

void JTAG_set_tdi(uint32_t val)
{
	if(val) GPIOB->BSHR = PIN_TDI;
	else	GPIOB->BCR  = PIN_TDI;
}


void JTAG_set_pins(int id, int val, int dir)
{
	int pd, pv;

	if(id==1)
		return;

	pd = (dir & 1) ? DIR_OUT : DIR_IN;
	pv = (val & 1) ? 1: 0;
	GPIO_WriteBit(GPIOB, PIN_TCK, pv);

	pd = (dir & 2) ? DIR_OUT : DIR_IN;
	pv = (val & 2) ? 1: 0;
	GPIO_WriteBit(GPIOB, PIN_TDI, pv);

	pd = (dir & 4) ? DIR_OUT : DIR_IN;
	pv = (val & 4) ? 1: 0;
	GPIO_WriteBit(GPIOB, PIN_TDO, pv);

	pd = (dir & 8) ? DIR_OUT : DIR_IN;
	pv = (val & 8) ? 1: 0;
	GPIO_WriteBit(GPIOB, PIN_TMS, pv);
}

int JTAG_get_pins(int id)
{
	int val = 0;

	if(id==1)
		return 0xff;

	if(GPIOB->INDR & PIN_TCK) val |= 0x01;
	if(GPIOB->INDR & PIN_TDI) val |= 0x02;
	if(GPIOB->INDR & PIN_TDO) val |= 0x04;
	if(GPIOB->INDR & PIN_TMS) val |= 0x08;

	return val;
}


uint32_t JTAG_xfer_tms_lsb_0(uint32_t data, uint32_t nbit, uint32_t delay)
{
	uint32_t val = 0;
	
	while(nbit--)
	{
		GPIOB->BCR  = PIN_TCK;
		
		if(data & 0x01)
			GPIOB->BSHR = PIN_TMS;
		else
			GPIOB->BCR  = PIN_TMS;
		
		for(int i = 0; i < delay; i++) __NOP();
		
		val = (val >> 1) | ((GPIOB->INDR & PIN_TDO ? 1 : 0) << 7);
		
		GPIOB->BSHR = PIN_TCK;
		
		data >>= 1;
		
		for(int i = 0; i < delay; i++) __NOP();
	}
	
	GPIOB->BCR  = PIN_TCK;
	
	return val;
}


uint32_t JTAG_xfer_tms_lsb_1(uint32_t data, uint32_t nbit, uint32_t delay)
{
	uint32_t val = 0;
	
	while(nbit--)
	{
		GPIOB->BSHR = PIN_TCK;
		
		if(data & 0x01)
			GPIOB->BSHR = PIN_TMS;
		else
			GPIOB->BCR  = PIN_TMS;
		
		for(int i = 0; i < delay; i++) __NOP();
		
		val = (val >> 1) | ((GPIOB->INDR & PIN_TDO ? 1 : 0) << 7);
		
		GPIOB->BCR  = PIN_TCK;
		
		data >>= 1;
		
		for(int i = 0; i < delay; i++) __NOP();
	}
	
	GPIOB->BSHR = PIN_TCK;
	
	return val;
}


uint32_t JTAG_xfer_tdi_lsb_0(uint32_t data, uint32_t nbit, uint32_t delay)
{
	uint32_t val = 0;
	
	while(nbit--)
	{
		GPIOB->BCR  = PIN_TCK;
		
		if(data & 0x01)
			GPIOB->BSHR = PIN_TDI;
		else
			GPIOB->BCR  = PIN_TDI;
		
		for(int i = 0; i < delay; i++) __NOP();
		
		val = (val >> 1) | ((GPIOB->INDR & PIN_TDO ? 1 : 0) << 7);
		
		GPIOB->BSHR = PIN_TCK;
		
		data >>= 1;
		
		for(int i = 0; i < delay; i++) __NOP();
	}
	
	GPIOB->BCR  = PIN_TCK;
	
	return val;
}


uint32_t JTAG_xfer_tdi_lsb_1(uint32_t data, uint32_t nbit, uint32_t delay)
{
	uint32_t val = 0;
	
	while(nbit--)
	{
		GPIOB->BSHR = PIN_TCK;
		
		if(data & 0x01)
			GPIOB->BSHR = PIN_TDI;
		else
			GPIOB->BCR  = PIN_TDI;
		
		for(int i = 0; i < delay; i++) __NOP();
		
		val = (val >> 1) | ((GPIOB->INDR & PIN_TDO ? 1 : 0) << 7);
		
		GPIOB->BCR  = PIN_TCK;
		
		data >>= 1;
		
		for(int i = 0; i < delay; i++) __NOP();
	}
	
	GPIOB->BSHR = PIN_TCK;
	
	return val;
}


uint32_t JTAG_xfer_tdi_msb_0(uint32_t data, uint32_t nbit, uint32_t delay)
{
	uint32_t val = 0;
	
	while(nbit--)
	{
		GPIOB->BCR  = PIN_TCK;
		
		if(data & 0x80)
			GPIOB->BSHR = PIN_TDI;
		else
			GPIOB->BCR  = PIN_TDI;
		
		for(int i = 0; i < delay; i++) __NOP();
		
		val = (val << 1) | (GPIOB->INDR & PIN_TDO ? 1 : 0);
		
		GPIOB->BSHR = PIN_TCK;
		
		data <<= 1;
		
		for(int i = 0; i < delay; i++) __NOP();
	}
	
	GPIOB->BCR  = PIN_TCK;
	
	return val;
}


uint32_t JTAG_xfer_tdi_msb_1(uint32_t data, uint32_t nbit, uint32_t delay)
{
	uint32_t val = 0;
	
	while(nbit--)
	{
		GPIOB->BSHR = PIN_TCK;
		
		if(data & 0x80)
			GPIOB->BSHR = PIN_TDI;
		else
			GPIOB->BCR  = PIN_TDI;
		
		for(int i = 0; i < delay; i++) __NOP();
		
		val = (val << 1) | (GPIOB->INDR & PIN_TDO ? 1 : 0);
		
		GPIOB->BCR  = PIN_TCK;
		
		data <<= 1;
		
		for(int i = 0; i < delay; i++) __NOP();
	}
	
	GPIOB->BSHR = PIN_TCK;
	
	return val;
}
