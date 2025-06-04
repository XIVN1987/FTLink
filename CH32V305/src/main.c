#include "ch32v30x.h"
#include "vcom_serial.h"
#include "usb_transfer.h"


void JTAG_handle(void);
void SysTick_Config(uint32_t ticks);

int main(void)
{
	VCOM_Init();

	USBD_Config();

	SysTick_Config(SystemCoreClock / 1000);

	while(1)
	{
		JTAG_handle();

		VCOM_TransferData();
	}

	return 0;
}


void SysTick_Config(uint32_t ticks)
{
	SysTick->CTLR= 0;
	SysTick->SR  = 0;
	SysTick->CNT = 0;
	SysTick->CMP = ticks;
	SysTick->CTLR= 0x0F;

	NVIC_EnableIRQ(SysTicK_IRQn);
}

volatile uint32_t SysTick_ms = 0;

void SysTick_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void SysTick_Handler(void)
{
	SysTick->SR = 0;

	SysTick_ms++;
}
