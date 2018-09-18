/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/


#include "MICROWIRE_Memory.h"


static uint8_t MicroStatus;
static uint8_t BytesNum;
static uint8_t AddressBits;
static uint32_t Address;
static uint8_t BitCnt;
static uint8_t *DataPtr;
static uint8_t DataSize;
static uint8_t MicroCmd;
static uint16_t ChipSel;


void MICROWIRE_Memory_Init(uint8_t TimingSet)
{
  SPI_GPIO->MODER |= (1 << (2 * SPI_NSS2_PIN)) | (1 << (2 * SPI_NSS4_PIN)) | (1 << (2 * SPI_SCK_PIN)) | (1 << (2 * SPI_MOSI_PIN));
    
  RCC->MICROWIRE_TIMER_ENR |= MICROWIRE_TIMER_CLK_EN;
  MICROWIRE_TIMER->PSC = TimingSet;
}
//------------------------------------------------------------------------------
void MICROWIRE_Memory_Write(MemoryIC_Type* Memory, uint8_t* DataArray)
{
  MicroStatus = MICROWIRE_WRITE | MICROWIRE_EMPTY_CLOCK;
  MicroCmd = MICROWIRE_CMD_WRITE;
  MICROWIRE_Start_Transfer(Memory, DataArray);
}
//------------------------------------------------------------------------------
void MICROWIRE_Memory_Read(MemoryIC_Type* Memory, uint8_t* DataArray)
{
  MicroStatus = MICROWIRE_READ | MICROWIRE_EMPTY_CLOCK;
  MicroCmd = MICROWIRE_CMD_READ;
  MICROWIRE_Start_Transfer(Memory, DataArray);
}
//----------------------------------------------------------------------------
void MICROWIRE_Memory_DeInit()
{
  RCC->MICROWIRE_TIMER_ENR &= ~MICROWIRE_TIMER_CLK_EN;
  SPI_GPIO->MODER &= ~((3 << (2 * SPI_NSS2_PIN)) | (3 << (2 * SPI_NSS4_PIN)) | (3 << (2 * SPI_SCK_PIN)) | (3 << (2 * SPI_MOSI_PIN)));
  MICROWIRE_ORG_GPIO->MODER &= ~(3 << (2 * MICROWIRE_ORG_PIN));
}
//------------------------------------------------------------------------------
void MICROWIRE_Memory_IsReady(uint16_t CS_PIN, uint8_t *Status)
{
  MICROWIRE_TIMER->DIER &= ~TIM_DIER_UIE;
  MICROWIRE_TIMER->CR1 |= TIM_CR1_CEN;
  
  while((MICROWIRE_TIMER->SR & TIM_SR_UIF) != TIM_SR_UIF);
  MICROWIRE_TIMER->SR &= ~TIM_SR_UIF;
  
  SPI_GPIO->ODR |= CS_PIN;
  while((SPI_GPIO->IDR & (1 << SPI_MISO_PIN)) != (1 << SPI_MISO_PIN));
  *Status = MEMORY_DETECTED;
  
  SPI_GPIO->ODR &= ~CS_PIN;
  while((MICROWIRE_TIMER->SR & TIM_SR_UIF) != TIM_SR_UIF);
  MICROWIRE_TIMER->SR &= ~TIM_SR_UIF;
  
  SPI_GPIO->ODR |= CS_PIN;
  while((MICROWIRE_TIMER->SR & TIM_SR_UIF) != TIM_SR_UIF);
  MICROWIRE_TIMER->SR &= ~TIM_SR_UIF;
  
  SPI_GPIO->ODR &= ~CS_PIN;
  MICROWIRE_TIMER->CR1 &= ~TIM_CR1_CEN;
}
//------------------------------------------------------------------------------
void MICROWIRE_TIMER_ISR(void)
{
  MICROWIRE_TIMER->SR &= ~TIM_SR_UIF;
  
  uint8_t TransferStatus = MicroStatus & MICROWIRE_TRANSFER_MASK;
  
  if ((MicroStatus & MICROWIRE_CLOCK_PHASE) != MICROWIRE_CLOCK_PHASE)
  {
    switch(TransferStatus)
    {
      case MICROWIRE_START:
        SPI_GPIO->ODR |= ChipSel | (1 << SPI_MOSI_PIN);
      break;
      
      case MICROWIRE_CMD:
        if (MicroCmd & (1 << (BitCnt - 1)))
          SPI_GPIO->ODR |= (1 << SPI_MOSI_PIN);
        else
          SPI_GPIO->ODR &= ~(1 << SPI_MOSI_PIN);
      break;
      
      case MICROWIRE_ADDRESS:
        if (Address & (1 << (BitCnt - 1)))
          SPI_GPIO->ODR |= (1 << SPI_MOSI_PIN);
        else
          SPI_GPIO->ODR &= ~(1 << SPI_MOSI_PIN);
      break;
      
      case MICROWIRE_DATA:
        if (MicroStatus & MICROWIRE_WRITE)
        {
          uint8_t BitOffset = BitCnt;
          if (BitCnt > 8)
            BitOffset -= 8;
          
          if ((*DataPtr) & (1 << (BitOffset - 1)))
            SPI_GPIO->ODR |= (1 << SPI_MOSI_PIN);
          else
            SPI_GPIO->ODR &= ~(1 << SPI_MOSI_PIN);
        }
      break;
      
      default:
      break;
    }
    
    MicroStatus |= MICROWIRE_CLOCK_PHASE;
    SPI_GPIO->ODR |= (1 << SPI_SCK_PIN);
  }
  
  else
  {
    BitCnt--;
    
    switch(TransferStatus)
    {
      case MICROWIRE_EMPTY_CLOCK:
        if (BitCnt == 0)
        {
          BitCnt = MICROWIRE_START_BITS;
          MicroStatus &= ~MICROWIRE_EMPTY_CLOCK;
          MicroStatus |= MICROWIRE_START;
        }
      break;
      
      case MICROWIRE_START:
        if (BitCnt == 0)
        {
          BitCnt = MICROWIRE_CMD_BITS;
          MicroStatus &= ~MICROWIRE_START;
          MicroStatus |= MICROWIRE_CMD;
        }
      break;
      
      case MICROWIRE_CMD:
        if (BitCnt == 0)
        {
          BitCnt = AddressBits;
          MicroStatus &= ~MICROWIRE_CMD;
          MicroStatus |= MICROWIRE_ADDRESS;
        }
      break;
      
      case MICROWIRE_ADDRESS:
        if (BitCnt == 0)
        {
          BitCnt = DataSize;
          MicroStatus &= ~MICROWIRE_ADDRESS;
          MicroStatus |= MICROWIRE_DATA;
        }
      break;
      
      case MICROWIRE_DATA:
        if (MicroStatus & MICROWIRE_READ)
        {
          uint8_t BitOffset = BitCnt;
          if (BitCnt >= 8)
            BitOffset -= 8;
          
          if (SPI_GPIO->IDR & (1 << SPI_MISO_PIN))
            (*DataPtr) |= 1 << BitOffset;
        }
        
        if (BitCnt == 0)
        {
          BytesNum--;
          
          if (BytesNum)
          {
            BitCnt = DataSize;
            *DataPtr++;
            if (MicroStatus & MICROWIRE_READ)
              *DataPtr = 0;
          }

          else
          {
            SPI_GPIO->ODR &= ~ChipSel;
            MICROWIRE_TIMER->CR1 &= ~TIM_CR1_CEN;
          }
        }
        
        else if ((BitCnt == 8) && (DataSize == 16))
        {
          *DataPtr++;
          if (MicroStatus & MICROWIRE_READ)
            *DataPtr = 0;
        }        
      break;
      
      default:
      break;
    }
    
    MicroStatus &= ~MICROWIRE_CLOCK_PHASE;
    SPI_GPIO->ODR &= ~(1 << SPI_SCK_PIN);
  }
}
//------------------------------------------------------------------------------
void MICROWIRE_Start_Transfer(MemoryIC_Type* Memory, uint8_t* DataArray)
{
  BytesNum = Memory->TransBytes;
  AddressBits = Memory->AdrBitsInCmd;
  Address = Memory->CurAdr;
  BitCnt = MICROWIRE_EMPTY_CLOCK_BITS;
  DataPtr = DataArray;
  if (MicroStatus & MICROWIRE_READ)
    *DataPtr = 0;
  ChipSel = Memory->SPI_CS;
  
  if (Memory->Type == MICROWIRE_EEPROM_ORG_8)
    DataSize = 8;
  else
    DataSize = 16;
  
  MICROWIRE_TIMER->DIER = TIM_DIER_UIE;
  MICROWIRE_TIMER->CR1 |= TIM_CR1_CEN;
  
  while((MICROWIRE_TIMER->CR1 & TIM_CR1_CEN) == TIM_CR1_CEN);//пока не закончилась передача
}
//------------------------------------------------------------------------------
void MICROWIRE_Memory_Clock_Pulse()
{
  SPI_GPIO->ODR |= (1 << SPI_SCK_PIN);
  while((MICROWIRE_TIMER->SR & TIM_SR_UIF) != TIM_SR_UIF);
  MICROWIRE_TIMER->SR &= ~TIM_SR_UIF;

  SPI_GPIO->ODR &= ~(1 << SPI_SCK_PIN);
  while((MICROWIRE_TIMER->SR & TIM_SR_UIF) != TIM_SR_UIF);
  MICROWIRE_TIMER->SR &= ~TIM_SR_UIF;

}
//------------------------------------------------------------------------------
void MICROWIRE_Memory_IsPresent(uint16_t CS_PIN, uint8_t* Status)
{
  *Status = MEMORY_DETECTED;
}
//------------------------------------------------------------------------------
void MICROWIRE_Memory_EWEN_EWDS(uint16_t CS_PIN, uint8_t AdrBits, uint8_t Cmd)
{
  MICROWIRE_TIMER->DIER &= ~TIM_DIER_UIE;
  MICROWIRE_TIMER->CR1 |= TIM_CR1_CEN;
  
  MICROWIRE_Memory_Clock_Pulse();
  //старт бит 1
  SPI_GPIO->ODR |= CS_PIN | (1 << SPI_MOSI_PIN);
  MICROWIRE_Memory_Clock_Pulse();
  //код 00
  SPI_GPIO->ODR &= ~(1 << SPI_MOSI_PIN);
  for(uint8_t i = 0; i < 2; i++)
    MICROWIRE_Memory_Clock_Pulse();
  //адресные биты
  //если разрешение записи, адресные биты = 1;
  //если запрет записи, адресные биты = 0
  if (Cmd == MICROWIRE_EWEN)
    SPI_GPIO->ODR |= (1 << SPI_MOSI_PIN);
  for(uint8_t i = 0; i < AdrBits; i++)
    MICROWIRE_Memory_Clock_Pulse();
  
  GPIOA->ODR &= ~CS_PIN;
  MICROWIRE_Memory_Clock_Pulse();
  
  MICROWIRE_TIMER->CR1 &= ~TIM_CR1_CEN;
  MICROWIRE_TIMER->SR &= ~TIM_SR_UIF;
  MICROWIRE_TIMER->DIER = TIM_DIER_UIE;
}


