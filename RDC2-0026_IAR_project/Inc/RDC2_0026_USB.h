/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/




#ifndef __RDC2_0026_USB_H
#define __RDC2_0026_USB_H

#include "RDC2_0026_board.h"


#define       USB_MESSAGE_LENGTH           0x40 //длина пакета USB, максимум 64 байта


#define       USB_REPORT_ID_POS            0
#define       USB_CMD_POS                  0x01 //индекс команды в посылке
#define       USB_DATA_SIZE_POS            1
#define       USB_START_DATA_POS           2

#define       USB_CMD_ID                   1
#define       USB_DATA_ID                  2

//команды
#define       USB_CMD_MEMORY_INIT          0
#define       USB_CMD_MEMORY_READ          1
#define       USB_CMD_MEMORY_WRITE         2
#define       USB_CMD_MEMORY_ERASE         3
#define       USB_CMD_GET_STATE            4
#define       USB_CMD_MEMORY_BLANK_CHECK   5
#define       USB_CMD_SPI_READ_STATUS      6
#define       USB_CMD_SPI_WRITE_STATUS     7


#define       MEMORY_TYPE_OFFSET           (USB_CMD_POS + 1)
#define       MEMORY_VOLUME_OFFSET         (MEMORY_TYPE_OFFSET + 1)
#define       MEMORY_PAGE_SIZE_OFFSET      (MEMORY_VOLUME_OFFSET + 4)
#define       MEMORY_ADDRESS_BYTES_OFFSET  (MEMORY_PAGE_SIZE_OFFSET + 2)
#define       MEMORY_ADDRESS_BITS_OFFSET   (MEMORY_ADDRESS_BYTES_OFFSET + 1)
#define       MEMORY_FREQUENCY_OFFSET      (MEMORY_ADDRESS_BITS_OFFSET + 1)
#define       MEMORY_SPI_CS_OFFSET         (MEMORY_FREQUENCY_OFFSET + 1)
#define       MEMORY_START_ADDRESS_OFFSET  (MEMORY_SPI_CS_OFFSET + 1)
#define       MEMORY_DEINIT_FLAG_OFFSET    (MEMORY_START_ADDRESS_OFFSET + 4)

#define       NOT_DEINIT_AFTER_CONNECTION  0
#define       DEINIT_AFTER_CONNECTION      1


//Status
#define       RDC2_0026_USB_IDLE           0
#define       RDC2_0026_USB_MEMORY_BLANK   (1 << 0)
#define       RDC2_0026_USB_MEMORY_INIT    (1 << 1)
#define       RDC2_0026_USB_MEMORY_WRITE   (1 << 2)
#define       RDC2_0026_USB_MEMORY_READ    (1 << 3)
#define       RDC2_0026_USB_MEMORY_ERASE   (1 << 4)
#define       RDC2_0026_USB_BUF_OVERFLOW   (1 << 5)
#define       RDC2_0026_USB_OPER_COMPLETE  (1 << 6)
#define       RDC2_0026_USB_MEMORY_BLANK_CHECK (1 << 7)
#define       RDC2_0026_USB_SPI_READ_STATUS    (1 << 8)
#define       RDC2_0026_USB_SPI_WRITE_STATUS   (1 << 9)



void RDC2_0026_USB_Init(uint8_t *MemoryDataBuffer);

void RDC2_0026_USB_RecPacket(uint8_t *Packet);

void RDC2_0026_USB_SendData(uint8_t *Data);

uint16_t* RDC2_0026_USB_GetStatus();

void RDC2_0026_USB_ClearStatus(uint16_t StatusFlag);

void RDC2_0026_USB_SetStatus(uint16_t StatusFlag);

uint8_t* RDC2_0026_USB_GetPacket();

uint8_t* RDC2_0026_USB_IsReadyToSend();

uint16_t* RDC2_0026_USB_GetDataIndex();


#endif //__RDC2_0026_USB_H


