/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/


#include "SPI_Memory.h"



void SPI_Memory_Init(uint8_t FreqSet)
{
  SPI_GPIO->MODER |= (2 << (2 * SPI_SCK_PIN)) | \
                     (2 << (2 * SPI_MISO_PIN)) | (2 << (2 * SPI_MOSI_PIN));
  
  SPI_GPIO->MODER |= (1 << (2 * SPI_NSS1_PIN)) | (1 << (2 * SPI_NSS3_PIN)) | \
                     (1 << (2 * SPI_NSS4_PIN));
  
  SPI_GPIO->ODR |= (1 << (2 * SPI_NSS1_PIN)) | (1 << (2 * SPI_NSS3_PIN)) | \
                   (1 << (2 * SPI_NSS4_PIN));
  
  SPI_PORT->CR1 |= FreqSet << 3;
}
//------------------------------------------------------------------------------
void SPI_Memory_Write(MemoryIC_Type* Memory, uint8_t* DataArray)
{
  SPI_GPIO->ODR &= ~(Memory->SPI_CS);
  SPI_Send(MEMORY_WRITE_ENABLE);
  SPI_GPIO->ODR |= (Memory->SPI_CS);
  
  uint8_t SPI_Cmd = MEMORY_WRITE_TO_ARRAY;
  SPI_GPIO->ODR &= ~(Memory->SPI_CS);

  if ((Memory->AdrBitsInCmd != 0) && ((Memory->CurAdr) > 0xFF))
    SPI_Cmd = MEMORY_WRITE_TO_ARRAY_A8;
  SPI_Send(SPI_Cmd);
  for (uint8_t i = 0; i < (Memory->AdrBytes); i++)
    SPI_Send((uint8_t)((Memory->CurAdr) >> (8 * i)));
  
  while(Memory->TransBytes)
  {
    SPI_Send(*DataArray);
    *DataArray++;
    (Memory->TransBytes)--;
  }
  
  SPI_GPIO->ODR |= (Memory->SPI_CS);
}
//------------------------------------------------------------------------------
void SPI_Memory_Read(MemoryIC_Type* Memory, uint8_t* DataArray)
{
  uint8_t SPI_Cmd = MEMORY_READ_FROM_ARRAY;
  SPI_GPIO->ODR &= ~(Memory->SPI_CS);

  if ((Memory->AdrBitsInCmd != 0) && ((Memory->CurAdr) > 0xFF))
    SPI_Cmd = MEMORY_READ_FROM_ARRAY_A8;
  SPI_Send(SPI_Cmd);
  for (uint8_t i = 0; i < (Memory->AdrBytes); i++)
    SPI_Send((uint8_t)((Memory->CurAdr) >> (8 * i)));

  do
  {
    *DataArray = SPI_Send(0x00);
    *DataArray++;
    (Memory->TransBytes)--;
  }
  while(Memory->TransBytes);
  
 SPI_GPIO->ODR |= (Memory->SPI_CS);
}
//------------------------------------------------------------------------------
void SPI_Memory_ReadStatus(uint16_t CS_PIN, uint8_t *Status)
{
  SPI_GPIO->ODR &= ~CS_PIN;
  
  SPI_Send(MEMORY_READ_STATUS);
  *Status = SPI_Send(0x00);
  
  SPI_GPIO->ODR |= CS_PIN;
}
//------------------------------------------------------------------------------
void SPI_Memory_WriteStatus(uint16_t CS_PIN, uint8_t *Status)
{
  SPI_GPIO->ODR &= ~CS_PIN;
  SPI_Send(MEMORY_WRITE_ENABLE);
  SPI_GPIO->ODR |= CS_PIN;
  
  SPI_GPIO->ODR &= ~CS_PIN;
  SPI_Send(MEMORY_WRITE_STATUS);
  SPI_Send(*Status);
  SPI_GPIO->ODR |= CS_PIN;
}
//------------------------------------------------------------------------------
void SPI_Memory_IsReady(uint16_t CS_PIN, uint8_t *Status)
{
  SPI_Memory_ReadStatus(CS_PIN, Status);
  if(((*Status) & MEMORY_STATUS_READY) == MEMORY_STATUS_READY)
    *Status = 0;
  else
    *Status = MEMORY_DETECTED;
}
//----------------------------------------------------------------------------
void SPI_Memory_DeInit()
{
  SPI_GPIO->MODER &= ~((3 << (2 * SPI_SCK_PIN)) | (3 << (2 * SPI_MISO_PIN)) | (3 << (2 * SPI_MOSI_PIN)) | \
                       (3 << (2 * SPI_NSS1_PIN)) | (3 << (2 * SPI_NSS2_PIN)) | \
                       (3 << (2 * SPI_NSS3_PIN)) | (3 << (2 * SPI_NSS4_PIN)));
}
//------------------------------------------------------------------------------
uint8_t SPI_Send(uint8_t DataToSend)
{
  while(!(SPI_PORT->SR & SPI_SR_TXE));
  SPI1_DR_8BIT = (uint8_t)DataToSend;
  while(!(SPI_PORT->SR & SPI_SR_RXNE));
  return SPI1_DR_8BIT;
}
//------------------------------------------------------------------------------
void SPI_Memory_IsPresent(uint16_t CS_PIN, uint8_t* Status)
{
  uint8_t InitStatus;
  
  SPI_Memory_ReadStatus(CS_PIN, &InitStatus);
  
  SPI_GPIO->ODR &= ~CS_PIN;
  SPI_Send(MEMORY_WRITE_ENABLE);
  SPI_GPIO->ODR |= CS_PIN;
  
  SPI_Memory_ReadStatus(CS_PIN, Status);
  
  if ((*Status) != InitStatus)
  {
    *Status = MEMORY_DETECTED;
    (*(Status + 1)) = JEDEC_ID_RESPONSE_BYTES;
    
    SPI_GPIO->ODR &= ~CS_PIN;
    SPI_Send(MEMORY_WRITE_DISABLE);
    SPI_GPIO->ODR |= CS_PIN;
    
    SPI_GPIO->ODR &= ~CS_PIN;
    SPI_Send(MEMORY_READ_JEDEC_ID);
    for (uint8_t i = 0; i < 3; i++)
      (*(Status + 2 + i)) = SPI_Send(0x00);

    SPI_GPIO->ODR |= CS_PIN;
  }
  
  else
    *Status = 0;
}
//------------------------------------------------------------------------------
void SPI_Memory_Erase(uint16_t CS_PIN)
{
  SPI_GPIO->ODR &= ~CS_PIN;
  SPI_Send(MEMORY_WRITE_ENABLE);
  SPI_GPIO->ODR |= CS_PIN;
  
  SPI_GPIO->ODR &= ~CS_PIN;
  SPI_Send(MEMORY_CHIP_ERASE);
  SPI_GPIO->ODR |= CS_PIN;
}



