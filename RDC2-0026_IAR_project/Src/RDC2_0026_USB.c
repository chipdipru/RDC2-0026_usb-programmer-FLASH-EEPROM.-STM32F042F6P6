/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/


#include "RDC2_0026_USB.h"
#include "usbd_custom_hid_core.h"
#include "usbd_usr.h"


USB_CORE_HANDLE  USB_Device_dev;
uint8_t PrevXferDone = 1;
static uint16_t USBStatus = RDC2_0026_USB_IDLE;
static uint8_t USBDataBuf[USB_MESSAGE_LENGTH];
static uint8_t *DataBufferPtr;
static uint16_t DataIndex = 0;


void RDC2_0026_USB_Init(uint8_t *MemoryDataBuffer)
{
  DataBufferPtr = MemoryDataBuffer;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGCOMPEN;
  SYSCFG->CFGR1 |= 1 << 4;
  USBD_Init(&USB_Device_dev, &USR_desc, &USBD_HID_cb, &USR_cb);  
}
//------------------------------------------------------------------------------
void RDC2_0026_USB_RecPacket(uint8_t *Packet)
{
  if ((*(Packet + USB_REPORT_ID_POS)) == USB_DATA_ID)
  {
    for (uint8_t i = 0; i < (*(Packet + USB_DATA_SIZE_POS)); i++)
      *(DataBufferPtr + DataIndex + i) = *(Packet + USB_START_DATA_POS + i);
    
    DataIndex += *(Packet + USB_DATA_SIZE_POS);
    
    if (DataIndex >= DATA_BUFFER_SIZE)
    {
      DataIndex = 0;
      USBStatus |= RDC2_0026_USB_BUF_OVERFLOW;
    }
  }
  
  else
  {
    for (uint8_t i = 0; i < USB_MESSAGE_LENGTH; i++)
      USBDataBuf[i] = *(Packet + i);
  
    switch(*(USBDataBuf + USB_CMD_POS))
    {
      case USB_CMD_MEMORY_INIT:
        USBStatus |= RDC2_0026_USB_MEMORY_INIT;
      break;
    
      case USB_CMD_MEMORY_WRITE:
        USBStatus |= RDC2_0026_USB_MEMORY_WRITE;
        DataIndex = 0;
      break;
      
      case USB_CMD_MEMORY_READ:
        USBStatus |= RDC2_0026_USB_MEMORY_READ;
      break;

      case USB_CMD_MEMORY_ERASE:
        USBStatus |= RDC2_0026_USB_MEMORY_ERASE;
      break;
  
      case USB_CMD_MEMORY_BLANK_CHECK:
        USBStatus |= RDC2_0026_USB_MEMORY_BLANK_CHECK;
      break;
      
      case USB_CMD_GET_STATE:
        USBDataBuf[USB_REPORT_ID_POS] = USB_CMD_ID;
        USBDataBuf[USB_CMD_POS] = USB_CMD_GET_STATE;
        USBDataBuf[USB_START_DATA_POS] = USBStatus;
        USBDataBuf[USB_START_DATA_POS + 1] = (uint8_t)(*RDC2_0026_GetDataPos());
        USBDataBuf[USB_START_DATA_POS + 2] = (uint8_t)((*RDC2_0026_GetDataPos()) >> 8);
        USBDataBuf[USB_START_DATA_POS + 3] = (uint8_t)((*RDC2_0026_GetDataPos()) >> 16);
        USBDataBuf[USB_START_DATA_POS + 4] = (uint8_t)((*RDC2_0026_GetDataPos()) >> 24);
        RDC2_0026_USB_SendData((uint8_t *)USBDataBuf);
        if ((USBStatus & RDC2_0026_USB_OPER_COMPLETE) == RDC2_0026_USB_OPER_COMPLETE)
          USBStatus &= ~(RDC2_0026_USB_OPER_COMPLETE | RDC2_0026_USB_MEMORY_BLANK);
          
      break;
      
      case USB_CMD_SPI_READ_STATUS:
        USBStatus |= RDC2_0026_USB_SPI_READ_STATUS;
      break;
      
      case USB_CMD_SPI_WRITE_STATUS:
        USBStatus |= RDC2_0026_USB_SPI_WRITE_STATUS;
      break;

      default:
      break;
    }
  }
}
//------------------------------------------------------------------------------
void RDC2_0026_USB_SendData(uint8_t *Data)
{
  if ((PrevXferDone) && (USB_Device_dev.dev.device_status == USB_CONFIGURED))
  {    
    USBD_HID_SendReport(&USB_Device_dev, Data, USB_MESSAGE_LENGTH);
    PrevXferDone = 0;
  } 
}
//------------------------------------------------------------------------------
uint16_t* RDC2_0026_USB_GetStatus()
{
  return &USBStatus;
}
//------------------------------------------------------------------------------
void RDC2_0026_USB_ClearStatus(uint16_t StatusFlag)
{
  USBStatus &= ~StatusFlag;
}
//------------------------------------------------------------------------------
void RDC2_0026_USB_SetStatus(uint16_t StatusFlag)
{
  USBStatus |= StatusFlag;
}
//------------------------------------------------------------------------------
uint8_t* RDC2_0026_USB_GetPacket()
{
  return USBDataBuf;
}
//------------------------------------------------------------------------------
uint8_t* RDC2_0026_USB_IsReadyToSend()
{
  return &PrevXferDone;
}
//------------------------------------------------------------------------------
uint16_t* RDC2_0026_USB_GetDataIndex()
{
  return &DataIndex;
}



