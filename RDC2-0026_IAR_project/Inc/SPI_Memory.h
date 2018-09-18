/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/


#ifndef __SPI_MEMORY_H
#define __SPI_MEMORY_H


#include "RDC2_0026_board.h"


#define       SPI1_DR_8BIT                 (*(__IO uint8_t *)((uint32_t)&(SPI1->DR)))

//команды
#define       MEMORY_WRITE_ENABLE          0x06
#define       MEMORY_WRITE_DISABLE         0x04
#define       MEMORY_WRITE_TO_ARRAY        0x02
#define       MEMORY_WRITE_TO_ARRAY_A8     0x0A
#define       MEMORY_READ_FROM_ARRAY       0x03
#define       MEMORY_READ_FROM_ARRAY_A8    0x0B
#define       MEMORY_READ_STATUS           0x05
#define       MEMORY_WRITE_STATUS          0x01
#define       MEMORY_READ_JEDEC_ID         0x9F
#define       MEMORY_CHIP_ERASE            0xC7 //0x60

#define       MEMORY_STATUS_READY          (1 << 0)
#define       MEMORY_STATUS_WEL            (1 << 1)

#define       JEDEC_ID_RESPONSE_BYTES      3



void SPI_Memory_Init(uint8_t FreqSet);

void SPI_Memory_Write(MemoryIC_Type* Memory, uint8_t* DataArray);

void SPI_Memory_Read(MemoryIC_Type* Memory, uint8_t* DataArray);

void SPI_Memory_ReadStatus(uint16_t CS_PIN, uint8_t *Status);

void SPI_Memory_WriteStatus(uint16_t CS_PIN, uint8_t *Status);

void SPI_Memory_IsReady(uint16_t CS_PIN, uint8_t *Status);

void SPI_Memory_IsPresent(uint16_t CS_PIN, uint8_t* Status);

void SPI_Memory_DeInit();

void SPI_Memory_Erase(uint16_t CS_PIN);

uint8_t SPI_Send(uint8_t DataToSend);


#endif //__SPI_MEMORY_H


