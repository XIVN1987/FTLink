/********************************** (C) COPYRIGHT *******************************
* File Name          : ch32v30x_usbhs_device.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2022/08/20
* Description        : This file provides all the USBHS firmware functions.
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* SPDX-License-Identifier: Apache-2.0
*******************************************************************************/
#include <string.h>
#include "ch32v30x.h"
#include "ch32v30x_usb.h"
#include "ch32v30x_usbhs_device.h"

#include "usb_desc.h"
#include "vcom_serial.h"
#include "usb_transfer.h"


const uint8_t *pUSBHS_Descr;

static USB_SETUP_REQ USBHS_Setup;

/* USB Device Status */
volatile uint8_t USBHS_DevAddr;
volatile uint8_t USBHS_DevSpeed;
volatile uint8_t USBHS_DevConfig;
volatile uint8_t USBHS_DevEnumStatus;


/* Endpoint Buffer */
uint8_t USBHS_EP0_Buf[USB_MAX_EP0_SZ]        __attribute__((aligned(4)));
uint8_t USBHS_EP1_Tx_Buf[USB_BULK_IN_SZ_HS]  __attribute__((aligned(4)));
uint8_t USBHS_EP2_Rx_Buf[USB_BULK_OUT_SZ_HS] __attribute__((aligned(4)));
uint8_t USBHS_EP3_Tx_Buf[VCP_BULK_IN_SZ_HS]  __attribute__((aligned(4)));
uint8_t USBHS_EP4_Rx_Buf[VCP_BULK_OUT_SZ_HS] __attribute__((aligned(4)));


/*********************************************************************
 * @fn      USBHS_RCC_Init
 *
 * @brief   Initializes the clock for USB2.0 High speed device.
 *
 * @return  none
 */
void USBHS_RCC_Init(void)
{
    RCC_USBCLK48MConfig(RCC_USBCLK48MCLKSource_USBPHY);

    RCC_USBHSPLLCLKConfig(RCC_HSBHSPLLCLKSource_HSE);       // USBHS PLL 时钟源：HSE
    RCC_USBHSConfig(RCC_USBPLL_Div2);                       // USBHS PLL 时钟源进入 PLL 前 2 分频
    RCC_USBHSPLLCKREFCLKConfig(RCC_USBHSPLLCKREFCLK_4M);    // USBHS PLL 时钟源分频后频率为 4MHz（外接晶振为 8MHz）
    RCC_USBHSPHYPLLALIVEcmd(ENABLE);

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_USBHS, ENABLE);
}


/*********************************************************************
 * @fn      USBHS_Device_Endp_Init
 *
 * @brief   Initializes USB device endpoints.
 *
 * @return  none
 */
void USBHS_Device_Endp_Init(void)
{
    USBHSD->ENDP_CONFIG = USBHS_UEP1_T_EN | USBHS_UEP2_R_EN |
                          USBHS_UEP3_T_EN | USBHS_UEP4_R_EN;

    USBHSD->UEP0_MAX_LEN = USB_MAX_EP0_SZ;      // 端点 n 接收数据的最大包长度
    USBHSD->UEP2_MAX_LEN = USB_BULK_OUT_SZ_HS;
    USBHSD->UEP4_MAX_LEN = VCP_BULK_OUT_SZ_HS;

    USBHSD->UEP0_DMA     = (uint32_t)USBHS_EP0_Buf;
    USBHSD->UEP1_TX_DMA  = (uint32_t)USBHS_EP1_Tx_Buf;
    USBHSD->UEP2_RX_DMA  = (uint32_t)USBHS_EP2_Rx_Buf;
    USBHSD->UEP3_TX_DMA  = (uint32_t)USBHS_EP3_Tx_Buf;
    USBHSD->UEP4_RX_DMA  = (uint32_t)USBHS_EP4_Rx_Buf;

    USBHSD->UEP0_TX_LEN  = 0;
    USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_RES_NAK;
    USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_RES_ACK;

    USBHSD->UEP1_TX_LEN  = 0;
    USBHSD->UEP1_TX_CTRL = USBHS_UEP_T_RES_NAK;

    USBHSD->UEP2_RX_CTRL = USBHS_UEP_R_RES_ACK;

    USBHSD->UEP3_TX_LEN  = 0;
    USBHSD->UEP3_TX_CTRL = USBHS_UEP_T_RES_NAK;

    USBHSD->UEP4_RX_CTRL = USBHS_UEP_R_RES_ACK;
}


/*********************************************************************
 * @fn      USBHS_Device_Init
 *
 * @brief   Initializes USB high-speed device.
 *
 * @return  none
 */
void USBHS_Device_Init(FunctionalState sta)
{
    USBHSD->CONTROL = USBHS_UC_CLR_ALL | USBHS_UC_RESET_SIE;
    for(int i = 0; i < SystemCoreClock / 1000; i++) __NOP();

    if(sta)
    {
        USBHSD->CONTROL &= ~USBHS_UC_RESET_SIE;
        USBHSD->HOST_CTRL = USBHS_UH_PHY_SUSPENDM;
        USBHSD->CONTROL = USBHS_UC_DMA_EN | USBHS_UC_INT_BUSY | USBHS_UC_SPEED_HIGH;
        USBHSD->INT_EN = USBHS_UIE_SETUP_ACT | USBHS_UIE_TRANSFER | USBHS_UIE_DETECT | USBHS_UIE_SUSPEND;

        USBHS_Device_Endp_Init();

        USBHSD->CONTROL |= USBHS_UC_DEV_PU_EN;

        NVIC_EnableIRQ(USBHS_IRQn);
    }
    else
    {
        USBHSD->CONTROL = 0;

        NVIC_DisableIRQ(USBHS_IRQn);
    }
}


void USBD_RxReady(uint32_t ep)
{
    if(ep == USB_BULK_OUT_EP)
        USBHSD->UEP2_RX_CTRL = (USBHSD->UEP2_RX_CTRL & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
    else if(ep == VCP_BULK_OUT_EP)
        USBHSD->UEP4_RX_CTRL = (USBHSD->UEP4_RX_CTRL & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_ACK;
}


void USBD_TxWrite(uint32_t ep, uint8_t * data, uint32_t size)
{
    if(ep == USB_BULK_IN_EP)
    {
        USBHSD->UEP1_TX_DMA = (uint32_t)data;
        USBHSD->UEP1_TX_LEN = size;
		USBHSD->UEP1_TX_CTRL = (USBHSD->UEP1_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
    }
    else if(ep == VCP_BULK_IN_EP)
    {
        USBHSD->UEP3_TX_DMA = (uint32_t)data;
        USBHSD->UEP3_TX_LEN = size;
		USBHSD->UEP3_TX_CTRL = (USBHSD->UEP3_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_ACK;
    }
}


extern volatile VCOM Vcom;
/*********************************************************************
 * @fn      USBHS_IRQHandler
 *
 * @brief   This function handles USBHS exception.
 *
 * @return  none
 */
void USBHS_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void USBHS_IRQHandler( void )
{
    uint8_t intf, intst, error;
    uint16_t len;

    intf = USBHSD->INT_FG;
    intst = USBHSD->INT_ST;

    if(intf & USBHS_UIF_SETUP_ACT)
    {
        USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_NAK;
        USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_NAK;

        memcpy(&USBHS_Setup, USBHS_EP0_Buf, sizeof(USBHS_Setup));

        len = 0;
        error = 0;
        if((USBHS_Setup.bRequestType & USB_REQ_TYP_MASK) == USB_REQ_TYP_STANDARD)
        {
            /* usb standard request processing */
            switch(USBHS_Setup.bRequest)
            {
            case USB_GET_DESCRIPTOR:
                switch(USBHS_Setup.wValue >> 8)
                {
                case USB_DESCR_TYP_DEVICE:
                    pUSBHS_Descr = USB_DevDesc;
                    len          = USB_DevDesc[0];
                    break;

                case USB_DESCR_TYP_CONFIG:
                    if((USBHSD->SPEED_TYPE & USBHS_USB_SPEED_TYPE) == USBHS_USB_SPEED_HIGH )
                    {
                        USBHS_DevSpeed = USBHS_USB_SPEED_HIGH;

                        pUSBHS_Descr = USB_CfgDesc_HS;
                        len          = USB_CfgDesc_HS[2] + (USB_CfgDesc_HS[3] << 8);
                    }
                    else
                    {
                        USBHS_DevSpeed = USBHS_USB_SPEED_FULL;

                        pUSBHS_Descr = USB_CfgDesc_FS;
                        len          = USB_CfgDesc_FS[2] + (USB_CfgDesc_FS[3] << 8);
                    }
                    break;

                case USB_DESCR_TYP_SPEED:
                    if(USBHS_DevSpeed == USBHS_USB_SPEED_HIGH)
                    {
                        pUSBHS_Descr = USB_CfgDesc_FS;
                        len          = USB_CfgDesc_FS[2] + (USB_CfgDesc_FS[3] << 8);
                    }
                    else
                    {
                        pUSBHS_Descr = USB_CfgDesc_HS;
                        len          = USB_CfgDesc_HS[2] + (USB_CfgDesc_HS[3] << 8);
                    }
                    break;

                case USB_DESCR_TYP_STRING:
                    switch(USBHS_Setup.wValue & 0xFF)
                    {
                    case DEF_STRING_DESC_LANG:
                        pUSBHS_Descr = USB_StringLangID;
                        len          = USB_StringLangID[0];
                        break;

                    case DEF_STRING_DESC_MANU:
                        pUSBHS_Descr = USB_StringVendor;
                        len          = USB_StringVendor[0];
                        break;

                    case DEF_STRING_DESC_PROD:
                        pUSBHS_Descr = USB_StringProduct;
                        len          = USB_StringProduct[0];
                        break;

                    case DEF_STRING_DESC_SERN:
                        pUSBHS_Descr = USB_StringSerialNbr;
                        len          = USB_StringSerialNbr[0];
                        break;

                    case 4:
                        pUSBHS_Descr = USB_StringIFaceJTAG;
                        len          = USB_StringIFaceJTAG[0];
                        break;
                    
                    case 5:
                        pUSBHS_Descr = USB_StringIFaceUART;
                        len          = USB_StringIFaceUART[0];
                        break;

                    default:
                        error = 0xFF;
                        break;
                    }
                    break;

                case USB_DESCR_TYP_QUALIF:
                    pUSBHS_Descr = USB_QualifierDesc;
                    len          = USB_QualifierDesc[0];
                    break;

                default :
                    error = 0xFF;
                    break;
                }

                /* Copy Descriptors to Endp0 DMA buffer */
                if(USBHS_Setup.wLength > len)
                {
                    USBHS_Setup.wLength = len;
                }
                len = (USBHS_Setup.wLength >= USB_MAX_EP0_SZ) ? USB_MAX_EP0_SZ : USBHS_Setup.wLength;
                memcpy(USBHS_EP0_Buf, pUSBHS_Descr, len);
                pUSBHS_Descr += len;
                break;

            case USB_SET_ADDRESS:
                USBHS_DevAddr = USBHS_Setup.wValue & 0xFF;
                break;

            case USB_GET_CONFIGURATION:
                USBHS_EP0_Buf[0] = USBHS_DevConfig;
                USBHS_Setup.wLength = 1;
                break;

            case USB_SET_CONFIGURATION:
                USBHS_DevConfig = USBHS_Setup.wValue & 0xFF;
                USBHS_DevEnumStatus = 1;
                
                Vcom.in_ready = 1;
                break;

            case USB_GET_INTERFACE:     // AltSetting Select
                USBHS_EP0_Buf[0] = 0x00;
                USBHS_Setup.wLength = 1;
                break;

            case USB_SET_INTERFACE:
                break;

            case USB_CLEAR_FEATURE:
                if((USBHS_Setup.bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                {
                    if((USBHS_Setup.wValue & 0xFF) == USB_REQ_FEAT_ENDP_HALT)
                    {
                        switch(USBHS_Setup.wIndex & 0xFF)
                        {
                        case USB_BULK_IN_EP:
                            USBHSD->UEP1_TX_CTRL = USBHS_UEP_T_RES_NAK;
                            break;

                        case USB_BULK_OUT_EP:
                            USBHSD->UEP2_RX_CTRL = USBHS_UEP_R_RES_ACK;
                            break;

                        case VCP_BULK_IN_EP:
                            USBHSD->UEP3_TX_CTRL = USBHS_UEP_T_RES_NAK;
                            break;

                        case VCP_BULK_OUT_EP:
                            USBHSD->UEP4_RX_CTRL = USBHS_UEP_R_RES_ACK;
                            break;

                        default:
                            error = 0xFF;
                            break;
                        }
                    }
                }
                break;

            case USB_SET_FEATURE:
                if((USBHS_Setup.bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                {
                    if((USBHS_Setup.wValue & 0xFF) == USB_REQ_FEAT_ENDP_HALT)
                    {
                        switch(USBHS_Setup.wIndex & 0xFF)
                        {
                        case USB_BULK_IN_EP:
                            USBHSD->UEP1_TX_CTRL = (USBHSD->UEP1_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_STALL;
                            break;

                        case USB_BULK_OUT_EP:
                            USBHSD->UEP2_RX_CTRL = (USBHSD->UEP2_RX_CTRL & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_STALL;
                            break;

                        case VCP_BULK_IN_EP:
                            USBHSD->UEP3_TX_CTRL = (USBHSD->UEP3_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_STALL;
                            break;

                        case VCP_BULK_OUT_EP:
                            USBHSD->UEP4_RX_CTRL = (USBHSD->UEP4_RX_CTRL & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_STALL;
                            break;

                        default:
                            error = 0xFF;
                            break;
                        }
                    }
                }
                break;

            case USB_GET_STATUS:
                USBHS_EP0_Buf[0] = 0x00;

                if((USBHS_Setup.bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP)
                {
                    switch(USBHS_Setup.wIndex & 0xFF)
                    {
                    case USB_BULK_IN_EP:
                        USBHS_EP0_Buf[0] = (USBHSD->UEP1_TX_CTRL & USBHS_UEP_T_RES_MASK) == USBHS_UEP_T_RES_STALL ? 0x01 : 0x00;
                        break;

                    case USB_BULK_OUT_EP:
                        USBHS_EP0_Buf[0] = (USBHSD->UEP2_RX_CTRL & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_STALL ? 0x01 : 0x00;
                        break;

                    case VCP_BULK_IN_EP:
                        USBHS_EP0_Buf[0] = (USBHSD->UEP3_TX_CTRL & USBHS_UEP_T_RES_MASK) == USBHS_UEP_T_RES_STALL ? 0x01 : 0x00;
                        break;

                    case VCP_BULK_OUT_EP:
                        USBHS_EP0_Buf[0] = (USBHSD->UEP4_RX_CTRL & USBHS_UEP_R_RES_MASK) == USBHS_UEP_R_RES_STALL ? 0x01 : 0x00;
                        break;

                    default:
                        error = 0xFF;
                        break;
                    }
                }

                USBHS_EP0_Buf[1] = 0x00;
                USBHS_Setup.wLength = 2;
                break;

            default:
                error = 0xFF;
                break;
            }
        }
        else
        {
            /* usb non-standard request processing */
            if(USBHS_Setup.bRequestType & USB_REQ_TYP_CLASS)
            {
                error = 0xFF;
            }
            else if(USBHS_Setup.bRequestType & USB_REQ_TYP_VENDOR)
            {
                extern void USB_VendorRequest(USB_SETUP_REQ * pSetup);
                USB_VendorRequest(&USBHS_Setup);
            }
            else
            {
                error = 0xFF;
            }
        }

        if(error == 0xFF)
        {
            USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_STALL;
            USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_STALL;
        }
        else
        {
            /* end-point 0 data Tx/Rx */
            if(USBHS_Setup.bRequestType & USB_EPT_IN)
            {
                len = (USBHS_Setup.wLength > USB_MAX_EP0_SZ) ? USB_MAX_EP0_SZ : USBHS_Setup.wLength;
                USBHS_Setup.wLength -= len;

                USBHSD->UEP0_TX_LEN = len;
                USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
            }
            else
            {
                if(USBHS_Setup.wLength == 0)
                {
                    USBHSD->UEP0_TX_LEN = 0;
                    USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
                }
                else
                {
                    USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;
                }
            }
        }

        USBHSD->INT_FG = USBHS_UIF_SETUP_ACT;
    }
    else if(intf & USBHS_UIF_TRANSFER)
    {
        switch(intst & USBHS_UIS_TOKEN_MASK)
        {
        /* data-in stage processing */
        case USBHS_UIS_TOKEN_IN:
            switch(intst & USBHS_UIS_ENDP_MASK)
            {
            case 0:
                if(USBHS_Setup.wLength == 0)
                {
                    USBHSD->UEP0_RX_CTRL = USBHS_UEP_R_TOG_DATA1 | USBHS_UEP_R_RES_ACK;
                }

                if((USBHS_Setup.bRequestType & USB_REQ_TYP_MASK) == USB_REQ_TYP_STANDARD)
                {
                    /* Standard request endpoint 0 Data upload */
                    switch(USBHS_Setup.bRequest)
                    {
                    case USB_GET_DESCRIPTOR:
                        len = USBHS_Setup.wLength >= USB_MAX_EP0_SZ ? USB_MAX_EP0_SZ : USBHS_Setup.wLength;
                        memcpy(USBHS_EP0_Buf, pUSBHS_Descr, len);
                        USBHS_Setup.wLength -= len;
                        pUSBHS_Descr += len;

                        USBHSD->UEP0_TX_LEN = len;
                        USBHSD->UEP0_TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;
                        break;

                    case USB_SET_ADDRESS:
                        USBHSD->DEV_AD = USBHS_DevAddr;
                        break;

                    default:
                        USBHSD->UEP0_TX_LEN = 0;
                        break;
                    }
                }
                else if((USBHS_Setup.bRequestType & USB_REQ_TYP_MASK) == USB_REQ_TYP_VENDOR)
                {
                    len = USBHS_Setup.wLength >= USB_MAX_EP0_SZ ? USB_MAX_EP0_SZ : USBHS_Setup.wLength;
                    memcpy(USBHS_EP0_Buf, pUSBHS_Descr, len);
                    USBHS_Setup.wLength -= len;
                    pUSBHS_Descr += len;

                    USBHSD->UEP0_TX_LEN = len;
                    USBHSD->UEP0_TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;
                }
                else
                {
                    /* Non-standard request endpoint 0 Data upload */
                }
                break;

            case USB_BULK_IN_EP & 0x0F:
                USBHSD->UEP1_TX_CTRL = (USBHSD->UEP1_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_NAK;
                USBHSD->UEP1_TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;

                void USB_In_callback(void);
                USB_In_callback();
                break;

            case VCP_BULK_IN_EP & 0x0F:
                USBHSD->UEP3_TX_CTRL = (USBHSD->UEP3_TX_CTRL & ~USBHS_UEP_T_RES_MASK) | USBHS_UEP_T_RES_NAK;
                USBHSD->UEP3_TX_CTRL ^= USBHS_UEP_T_TOG_DATA1;

                Vcom.in_ready = 1;
                break;

            default:
                break;
            }
            break;

        /* data-out stage processing */
        case USBHS_UIS_TOKEN_OUT:
            len = USBHSH->RX_LEN;
            switch(intst & USBHS_UIS_ENDP_MASK)
            {
            case 0:
                 if(intst & USBHS_UIS_TOG_OK)
                 {
                     if((USBHS_Setup.bRequestType & USB_REQ_TYP_MASK) == USB_REQ_TYP_STANDARD)
                     {
                          /* Standard request end-point 0 Data download */
                     }
                     else
                     {
                         USBHS_Setup.wLength = 0;

                         /* Non-standard request end-point 0 Data download */
                         if(USBHS_Setup.bRequest == CDC_SET_LINE_CODING)
                         {
                             memcpy(&LineCfg, USBHS_EP0_Buf, sizeof(LineCfg));

                             VCOM_LineCoding(&LineCfg);
                         }
                     }

                     if(USBHS_Setup.wLength == 0)
                     {
                         USBHSD->UEP0_TX_LEN  = 0;
                         USBHSD->UEP0_TX_CTRL = USBHS_UEP_T_TOG_DATA1 | USBHS_UEP_T_RES_ACK;
                     }
                 }
                 break;

            case USB_BULK_OUT_EP:
                USBHSD->UEP2_RX_CTRL ^= USBHS_UEP_R_TOG_DATA1;
                USBHSD->UEP2_RX_CTRL = (USBHSD->UEP2_RX_CTRL & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_NAK;

                void USB_Out_callback(uint8_t data[], uint32_t nbytes);
                USB_Out_callback(USBHS_EP2_Rx_Buf, len);
                break;

            case VCP_BULK_OUT_EP:
                USBHSD->UEP4_RX_CTRL ^= USBHS_UEP_R_TOG_DATA1;
                USBHSD->UEP4_RX_CTRL = (USBHSD->UEP4_RX_CTRL & ~USBHS_UEP_R_RES_MASK) | USBHS_UEP_R_RES_NAK;

                Vcom.out_bytes = len;
                Vcom.out_ready = 1;
                break;

            default:
                error = 0xFF;
                break;
            }
            break;

        case USBHS_UIS_TOKEN_SOF:
            break;

        default:
            break;
        }

        USBHSD->INT_FG = USBHS_UIF_TRANSFER;
    }
    else if(intf & USBHS_UIF_BUS_RST)
    {
        USBHS_DevAddr = 0;
        USBHS_DevConfig = 0;
        USBHS_DevEnumStatus = 0;

        USBHSD->DEV_AD = 0;
        USBHS_Device_Endp_Init();

        USBHSD->INT_FG = USBHS_UIF_BUS_RST;
    }
    else
    {
        USBHSD->INT_FG = intf;
    }
}
