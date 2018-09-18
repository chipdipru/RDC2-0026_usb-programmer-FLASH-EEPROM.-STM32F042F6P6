/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/

#include "RDC2_0026_board.h"
#include "I2C_Memory.h"
#include "SPI_Memory.h"
#include "MICROWIRE_Memory.h"
#include "RDC2_0026_USB.h"



static MemoryIC_Type Memory;
static MemoryIC_Type *MemChip = &Memory;
static MemoryFunc_Type MemoryFunc;
static MemoryFunc_Type *MemFunc = &MemoryFunc;
static uint8_t DataBuffer[DATA_BUFFER_SIZE];
static uint16_t DataPos = 0;


int main()
{
  RDC2_0026_Init();

  for(;;)
  {
    if ((*RDC2_0026_USB_GetStatus()) != RDC2_0026_USB_IDLE)
    {
      LED_GPIO->ODR &= ~(1 << LED_PIN);
      
      if ((*RDC2_0026_USB_GetStatus()) == RDC2_0026_USB_MEMORY_INIT)
      {
        uint8_t *NewData = (uint8_t *)RDC2_0026_USB_GetPacket();
        
        MemChip->Type = *(NewData + MEMORY_TYPE_OFFSET);
        
        MemChip->Volume = 0;
        for (uint8_t i = 0; i < 4; i++)
          MemChip->Volume |= (*(NewData + MEMORY_VOLUME_OFFSET + i)) << (8 * i);
        
        MemChip->PageSize = (*(NewData + MEMORY_PAGE_SIZE_OFFSET)) | ((*(NewData + MEMORY_PAGE_SIZE_OFFSET + 1)) << 8);
        MemChip->AdrBytes = *(NewData + MEMORY_ADDRESS_BYTES_OFFSET);
        MemChip->AdrBitsInCmd = *(NewData + MEMORY_ADDRESS_BITS_OFFSET);
        MemChip->Frequency = *(NewData + MEMORY_FREQUENCY_OFFSET);
        MemChip->SPI_CS = SPI_CHIP_SEL_OUT[*(NewData + MEMORY_SPI_CS_OFFSET)];
        
        MemChip->CurAdr = 0;
        for (uint8_t i = 0; i < 4; i++)
          MemChip->CurAdr |= (*(NewData + MEMORY_START_ADDRESS_OFFSET + i)) << (8 * i);
          
        
        if (MemChip->Type == I2C_MEMORY)
        {
          MemFunc->Bus_Init = I2C_Memory_Init;
          MemFunc->Write = I2C_Memory_Write;
          MemFunc->Read = I2C_Memory_Read;
          MemFunc->IsReady = I2C_Memory_IsReady;
          MemFunc->IsPresent = I2C_Memory_IsReady;
          MemFunc->Bus_DeInit = I2C_Memory_DeInit;
        }
        else if ((MemChip->Type == SPI_MEMORY_EEPROM) || (MemChip->Type == SPI_MEMORY_FLASH))
        {
          MemFunc->Bus_Init = SPI_Memory_Init;
          MemFunc->Write = SPI_Memory_Write;
          MemFunc->Read = SPI_Memory_Read;
          MemFunc->IsReady = SPI_Memory_IsReady;
          MemFunc->IsPresent = SPI_Memory_IsPresent;
          MemFunc->Bus_DeInit = SPI_Memory_DeInit;
        }
        else
        {
          MemFunc->Bus_Init = MICROWIRE_Memory_Init;
          MemFunc->Write = MICROWIRE_Memory_Write;
          MemFunc->Read = MICROWIRE_Memory_Read;
          MemFunc->IsReady = MICROWIRE_Memory_IsReady;
          MemFunc->IsPresent = MICROWIRE_Memory_IsPresent;
          MemFunc->Bus_DeInit = MICROWIRE_Memory_DeInit;
          
          MICROWIRE_ORG_GPIO->MODER |= 1 << (2 * MICROWIRE_ORG_PIN);
          if (MemChip->Type == MICROWIRE_EEPROM_ORG_16)
            MICROWIRE_ORG_GPIO->ODR |= 1 << MICROWIRE_ORG_PIN;
        }
        
        uint8_t DeInit = 0;
        if ((*(NewData + MEMORY_DEINIT_FLAG_OFFSET)) == DEINIT_AFTER_CONNECTION)
          DeInit = DEINIT_AFTER_CONNECTION;
        
        I2C_Memory_DeInit();
        SPI_Memory_DeInit();
        MICROWIRE_Memory_DeInit();
        
        MemFunc->Bus_Init(MemChip->Frequency);
        if ((MemChip->Type == I2C_MEMORY) && (MemChip->AdrBitsInCmd == 0))
          I2C_PORT->CR2 |= I2C_MEMORY_ADDRESS << 1;
                        
        MemFunc->IsPresent(MemChip->SPI_CS, &NewData[USB_START_DATA_POS]);
        
        while((*RDC2_0026_USB_IsReadyToSend()) != 1);
        RDC2_0026_USB_SendData(NewData);
        
        if (DeInit == DEINIT_AFTER_CONNECTION)
          MemFunc->Bus_DeInit();
        
        RDC2_0026_USB_ClearStatus(RDC2_0026_USB_MEMORY_INIT);
      }
      
      else if ((*RDC2_0026_USB_GetStatus()) == RDC2_0026_USB_MEMORY_WRITE)
      {
        DataPos = 0;
        
        if ((MemChip->Type == MICROWIRE_EEPROM_ORG_8) || (MemChip->Type == MICROWIRE_EEPROM_ORG_16))
          MICROWIRE_Memory_EWEN_EWDS(MemChip->SPI_CS, MemChip->AdrBitsInCmd, MICROWIRE_EWEN);
        
        while((((*RDC2_0026_USB_GetStatus()) & RDC2_0026_USB_MEMORY_WRITE) == RDC2_0026_USB_MEMORY_WRITE)
           && (MemChip->CurAdr < MemChip->Volume))
        {
          uint8_t Status = 0;
          
          while(Status == 0)
          {
            if ((((*RDC2_0026_USB_GetStatus()) & RDC2_0026_USB_BUF_OVERFLOW) != RDC2_0026_USB_BUF_OVERFLOW)
               && (DataPos < (*RDC2_0026_USB_GetDataIndex()))
               && (((*RDC2_0026_USB_GetDataIndex()) - DataPos) >= (MemChip->PageSize)))
            {
              Status = 1;
            }
            
            else if (((*RDC2_0026_USB_GetStatus()) & RDC2_0026_USB_BUF_OVERFLOW) == RDC2_0026_USB_BUF_OVERFLOW)
            {
              Status = 1;
            }
          }
          
          MemChip->TransBytes = MemChip->PageSize;
          MemFunc->Write(MemChip, &DataBuffer[DataPos]);
          MemChip->CurAdr += MemChip->PageSize;
          DataPos += MemChip->PageSize;
          if (MemChip->Type == MICROWIRE_EEPROM_ORG_16)//в этом случае PageSize = 1, а за раз пишутся 2 байта
            DataPos++;
              
          if (DataPos >= DATA_BUFFER_SIZE)
          {
            DataPos = 0;
            RDC2_0026_USB_ClearStatus(RDC2_0026_USB_BUF_OVERFLOW);
          }
          
          Status = 0;
          while(Status == 0)
            MemFunc->IsReady(MemChip->SPI_CS, &Status);
        }
        
        RDC2_0026_USB_SetStatus(RDC2_0026_USB_OPER_COMPLETE);
        if ((MemChip->Type == MICROWIRE_EEPROM_ORG_8) || (MemChip->Type == MICROWIRE_EEPROM_ORG_16))
          MICROWIRE_Memory_EWEN_EWDS(MemChip->SPI_CS, MemChip->AdrBitsInCmd, MICROWIRE_EWDS);
        MemFunc->Bus_DeInit();
        
        RDC2_0026_USB_ClearStatus(RDC2_0026_USB_MEMORY_WRITE);
      }
      
      else if ((*RDC2_0026_USB_GetStatus()) == RDC2_0026_USB_MEMORY_READ)
      {
        uint16_t BytesToRead = USB_MESSAGE_LENGTH;
        if (MemChip->Type == SPI_MEMORY_FLASH)
          BytesToRead = 4 * USB_MESSAGE_LENGTH;
        else if (MemChip->Type == MICROWIRE_EEPROM_ORG_16)
          BytesToRead /= 2;
        
        while((((*RDC2_0026_USB_GetStatus()) & RDC2_0026_USB_MEMORY_READ) == RDC2_0026_USB_MEMORY_READ)
           && (MemChip->CurAdr < MemChip->Volume))
        {
          MemChip->TransBytes = BytesToRead;
          MemFunc->Read(MemChip, &DataBuffer[0]);
          MemChip->CurAdr += BytesToRead;
          
          uint8_t SendTime = 1;
          if (MemChip->Type == SPI_MEMORY_FLASH)
            SendTime = 4;
          
          for (uint8_t i = 0; i < SendTime; i++)
          {
            while((*RDC2_0026_USB_IsReadyToSend()) != 1);
            RDC2_0026_USB_SendData(&DataBuffer[USB_MESSAGE_LENGTH * i]);
          }
        }
        
        RDC2_0026_USB_SetStatus(RDC2_0026_USB_OPER_COMPLETE);
        MemFunc->Bus_DeInit();
                
        RDC2_0026_USB_ClearStatus(RDC2_0026_USB_MEMORY_READ);
      }
      
      else if ((*RDC2_0026_USB_GetStatus()) == RDC2_0026_USB_MEMORY_ERASE)
      {
        if (MemChip->Type != SPI_MEMORY_FLASH)
        {
          for (uint16_t i = 0; i < 256; i++)
            DataBuffer[i] = 0xFF;
          
          if ((MemChip->Type == MICROWIRE_EEPROM_ORG_8) || (MemChip->Type == MICROWIRE_EEPROM_ORG_16))
            MICROWIRE_Memory_EWEN_EWDS(MemChip->SPI_CS, MemChip->AdrBitsInCmd, MICROWIRE_EWEN);
          
          while (MemChip->CurAdr < MemChip->Volume)
          {
            MemChip->TransBytes = MemChip->PageSize;
            MemFunc->Write(MemChip, &DataBuffer[0]);
            MemChip->CurAdr += MemChip->PageSize;
            
            uint8_t Status = 0;
            while(Status == 0)
              MemFunc->IsReady(MemChip->SPI_CS, &Status);
          }
        }
        
        else
        {
          SPI_Memory_Erase(MemChip->SPI_CS);

          uint8_t Status = 0;
          while(Status == 0)
            MemFunc->IsReady(MemChip->SPI_CS, &Status);
        }
        
        RDC2_0026_USB_SetStatus(RDC2_0026_USB_OPER_COMPLETE);
        if ((MemChip->Type == MICROWIRE_EEPROM_ORG_8) || (MemChip->Type == MICROWIRE_EEPROM_ORG_16))
          MICROWIRE_Memory_EWEN_EWDS(MemChip->SPI_CS, MemChip->AdrBitsInCmd, MICROWIRE_EWDS);
        MemFunc->Bus_DeInit();
        RDC2_0026_USB_ClearStatus(RDC2_0026_USB_MEMORY_ERASE);
      }
      
      else if ((*RDC2_0026_USB_GetStatus()) == RDC2_0026_USB_MEMORY_BLANK_CHECK)
      {
        uint16_t BytesToRead;
        uint8_t EmptyFlag = 1;
        
        if (MemChip->Type == I2C_MEMORY)
          BytesToRead = I2C_READ_MAX_BYTES_NUM;
        else if ((MemChip->Volume) <= DATA_BUFFER_SIZE)
          BytesToRead = MemChip->Volume;
        else
          BytesToRead = DATA_BUFFER_SIZE;
                
        while (MemChip->CurAdr < MemChip->Volume)
        {
          if (MemChip->Type == MICROWIRE_EEPROM_ORG_16)
            BytesToRead /= 2;
          
          MemChip->TransBytes = BytesToRead;
          MemFunc->Read(MemChip, &DataBuffer[0]);
          
          if (MemChip->Type == MICROWIRE_EEPROM_ORG_16)
            BytesToRead *= 2;
          
          for (uint16_t i = 0; i < BytesToRead; i++)
          {
            if (DataBuffer[i] != 0xFF)
            {
              DataPos = MemChip->CurAdr + i;
              if (MemChip->Type == MICROWIRE_EEPROM_ORG_16)
                DataPos -= i / 2;
              
              EmptyFlag = 0;
              break;
            }
          }
          
          MemChip->CurAdr += BytesToRead;
          
          if (EmptyFlag == 0)
            break;
        }
        
        RDC2_0026_USB_SetStatus(RDC2_0026_USB_OPER_COMPLETE | EmptyFlag);
        MemFunc->Bus_DeInit();
        RDC2_0026_USB_ClearStatus(RDC2_0026_USB_MEMORY_BLANK_CHECK);
      }
      
      else if ((*RDC2_0026_USB_GetStatus()) == RDC2_0026_USB_SPI_READ_STATUS)
      {
        uint8_t *NewData = (uint8_t *)RDC2_0026_USB_GetPacket();
          
        SPI_Memory_ReadStatus(MemChip->SPI_CS, &NewData[USB_START_DATA_POS]);
        
        while((*RDC2_0026_USB_IsReadyToSend()) != 1);
        RDC2_0026_USB_SendData(NewData);
        
        MemFunc->Bus_DeInit();
        RDC2_0026_USB_ClearStatus(RDC2_0026_USB_SPI_READ_STATUS);
      }
      
      else if ((*RDC2_0026_USB_GetStatus()) == RDC2_0026_USB_SPI_WRITE_STATUS)
      {
        uint8_t *NewData = (uint8_t *)RDC2_0026_USB_GetPacket();
          
        SPI_Memory_WriteStatus(MemChip->SPI_CS, &NewData[USB_START_DATA_POS]);
        
        uint8_t Status = 0;
        while(Status == 0)
          MemFunc->IsReady(MemChip->SPI_CS, &Status);
        
        RDC2_0026_USB_SetStatus(RDC2_0026_USB_OPER_COMPLETE);
        
        MemFunc->Bus_DeInit();
        RDC2_0026_USB_ClearStatus(RDC2_0026_USB_SPI_WRITE_STATUS);
      }
      
      LED_GPIO->ODR |= (1 << LED_PIN);
    }
  }
}
//------------------------------------------------------------------------------
void RDC2_0026_Init()
{
  //HSI, PLL, 48 MHz
  FLASH->ACR = FLASH_ACR_PRFTBE | (uint32_t)FLASH_ACR_LATENCY;
  RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_DIV2 | RCC_CFGR_PLLMUL12);
  RCC->CR |= RCC_CR_PLLON;
  while((RCC->CR & RCC_CR_PLLRDY) == 0);
 
  RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL);

  RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN | RCC_AHBENR_GPIOFEN;
  
  LED_GPIO->MODER |= 1 << LED_PIN;
  LED_GPIO->ODR |= (1 << LED_PIN);
  
  //I2C
  I2C_GPIO->OTYPER |= (1 << I2C_SCL_PIN) | (1 << I2C_SDA_PIN);
  I2C_GPIO->AFR[0] |= (I2C_SCL_AF << (4 * I2C_SCL_PIN)) | (I2C_SDA_AF << (4 * I2C_SDA_PIN));
  RCC->CFGR3 |= RCC_CFGR3_I2C1SW_SYSCLK;
  RCC->I2C_TIMER_ENR |= I2C_TIMER_CLK_EN;
  I2C_TIMER->ARR = 1;
  RCC->I2C_TIMER_ENR &= ~I2C_TIMER_CLK_EN;
    
  //SPI
  SPI_GPIO->OSPEEDR |= (1 << (2 * SPI_SCK_PIN)) | (1 << (2 * SPI_MISO_PIN)) | (1 << (2 * SPI_MOSI_PIN));
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  SPI_PORT->CR1 = SPI_CR1_MSTR | SPI_CR1_SSI | SPI_CR1_SSM;
  SPI_PORT->CR2 = SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;
  SPI_PORT->CR1 |= SPI_CR1_SPE;
  
  //MICROWIRE
  RCC->MICROWIRE_TIMER_ENR |= MICROWIRE_TIMER_CLK_EN;
  MICROWIRE_TIMER->ARR = 1;
  MICROWIRE_TIMER->DIER = TIM_DIER_UIE;
  RCC->MICROWIRE_TIMER_ENR &= ~MICROWIRE_TIMER_CLK_EN;
  NVIC_EnableIRQ(MICROWIRE_TIMER_IRQ);
  NVIC_SetPriority(MICROWIRE_TIMER_IRQ, MICROWIRE_PRIORITY);
    
  //USB
  RDC2_0026_USB_Init(DataBuffer);
}
//------------------------------------------------------------------------------
uint16_t* RDC2_0026_GetDataPos()
{
  return &DataPos;
}


