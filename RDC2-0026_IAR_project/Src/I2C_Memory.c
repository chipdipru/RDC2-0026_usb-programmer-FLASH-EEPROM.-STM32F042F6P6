/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/


#include "I2C_Memory.h"


void I2C_Memory_Init(uint8_t TimingSet)
{
  I2C_GPIO->MODER |= (2 << (2 * I2C_SCL_PIN)) | (2 << (2 * I2C_SDA_PIN));
  
  RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
  
  I2C_PORT->TIMINGR = I2C_TIMINGR[TimingSet];
  I2C_PORT->CR1 = I2C_CR1_PE;
  RCC->I2C_TIMER_ENR |= I2C_TIMER_CLK_EN;
  I2C_TIMER->PSC = I2C_TIMER_PCS[TimingSet];
}
//------------------------------------------------------------------------------
void I2C_Memory_Write(MemoryIC_Type* Memory, uint8_t* DataArray)
{
  if (Memory->AdrBitsInCmd != 0)
  {
    I2C_PORT->CR2 &= ~I2C_CR2_SADD;
    I2C_PORT->CR2 |= (I2C_MEMORY_ADDRESS | ((uint8_t)((Memory->CurAdr) >> (8 * (Memory->AdrBytes))))) << 1;
  }
  
  I2C_PORT->CR2 &= ~(I2C_CR2_NBYTES | I2C_CR2_RD_WRN);
  I2C_PORT->CR2 |= ((Memory->TransBytes + Memory->AdrBytes) << 16);
  
  I2C_PORT->TXDR = (uint8_t)((Memory->CurAdr) >> (8 * (Memory->AdrBytes - 1)));
  I2C_PORT->CR2 |= I2C_CR2_START;
  while(!(I2C_PORT->ISR & I2C_ISR_TXE)); //для надежности добавить счетчик
                                         //и выход с ошибкой
  for (uint8_t i = 1; i < (Memory->AdrBytes); i++)
  {
    I2C_PORT->TXDR = (uint8_t)((Memory->CurAdr) >> (8 * (Memory->AdrBytes - 1 - i)));
    while(!(I2C_PORT->ISR & I2C_ISR_TXE));
  }
  
  while(Memory->TransBytes)
  {
    I2C_PORT->TXDR = *DataArray;
    while(!(I2C_PORT->ISR & I2C_ISR_TXE)); //для надежности добавить счетчик
                                           //и выход с ошибкой
    *DataArray++;
    (Memory->TransBytes)--;
  }
  
  I2C_PORT->CR2 |= I2C_CR2_STOP;
}
//------------------------------------------------------------------------------
void I2C_Memory_Read(MemoryIC_Type* Memory, uint8_t* DataArray)
{
  if (Memory->AdrBitsInCmd != 0)
  {
    I2C_PORT->CR2 &= ~I2C_CR2_SADD;
    I2C_PORT->CR2 |= (I2C_MEMORY_ADDRESS | ((uint8_t)((Memory->CurAdr) >> (8 * (Memory->AdrBytes))))) << 1;
  }
  
  I2C_PORT->CR2 &= ~(I2C_CR2_NBYTES | I2C_CR2_RD_WRN);
  I2C_PORT->CR2 |= ((Memory->AdrBytes) << 16);
  
  I2C_PORT->TXDR = (uint8_t)((Memory->CurAdr) >> (8 * (Memory->AdrBytes - 1)));
  I2C_PORT->CR2 |= I2C_CR2_START;
  while(!(I2C_PORT->ISR & I2C_ISR_TXE)); //для надежности добавить счетчик
                                         //и выход с ошибкой
  for (uint8_t i = 1; i < (Memory->AdrBytes); i++)
  {
    I2C_PORT->TXDR = (uint8_t)((Memory->CurAdr) >> (8 * (Memory->AdrBytes - 1 - i)));
    while(!(I2C_PORT->ISR & I2C_ISR_TXE));
  }
  
  I2C_PORT->CR2 |= I2C_CR2_RD_WRN;
  I2C_PORT->CR2 &= ~I2C_CR2_NBYTES;
  I2C_PORT->CR2 |= ((Memory->TransBytes) << 16);
  I2C_PORT->CR2 |= I2C_CR2_START;
  
  do
  {
    while(!(I2C_PORT->ISR & I2C_ISR_RXNE)); //для надежности добавить счетчик
                                            //и выход с ошибкой
    *DataArray = I2C_PORT->RXDR;
    *DataArray++;
    (Memory->TransBytes)--;
  }
  while(Memory->TransBytes);
  
  I2C_PORT->CR2 |= I2C_CR2_STOP;
  I2C_PORT->ICR |= I2C_ICR_STOPCF;
}
//----------------------------------------------------------------------------
void I2C_Memory_DeInit()
{
  I2C_PORT->CR1 &= ~I2C_CR1_PE;
  I2C_GPIO->MODER &= ~((3 << (2 * I2C_SCL_PIN)) | (3 << (2 * I2C_SDA_PIN)));
  I2C_PORT->TIMINGR = 0;
  RCC->APB1ENR &= ~RCC_APB1ENR_I2C1EN;
  RCC->I2C_TIMER_ENR &= ~I2C_TIMER_CLK_EN;
}
//------------------------------------------------------------------------------
void I2C_Memory_IsReady(uint16_t CS_PIN, uint8_t *Status)
{
  *Status = MEMORY_DETECTED;
  
  I2C_PORT->CR2 &= ~I2C_CR2_SADD;
  I2C_PORT->CR2 |= I2C_MEMORY_ADDRESS << 1;
  I2C_PORT->CR2 &= ~(I2C_CR2_NBYTES | I2C_CR2_RD_WRN);
    
  I2C_PORT->CR2 |= I2C_CR2_START;

  I2C_TIMER->CR1 |= TIM_CR1_CEN;
  while((I2C_TIMER->SR & TIM_SR_UIF) != TIM_SR_UIF);
  I2C_TIMER->SR &= ~TIM_SR_UIF;

  I2C_PORT->CR2 |= I2C_CR2_START;
  while((I2C_TIMER->SR & TIM_SR_UIF) != TIM_SR_UIF);
  I2C_TIMER->SR &= ~TIM_SR_UIF;

  I2C_PORT->CR2 |= I2C_CR2_STOP;
  
  while((I2C_TIMER->SR & TIM_SR_UIF) != TIM_SR_UIF);
  I2C_TIMER->SR &= ~TIM_SR_UIF;
  I2C_TIMER->CR1 &= ~TIM_CR1_CEN;
  
  if ((I2C_PORT->ISR & I2C_ISR_NACKF) == I2C_ISR_NACKF)
  {
    *Status = 0;
    I2C_PORT->ICR |= I2C_ICR_NACKCF;
    I2C_PORT->ISR |= I2C_ISR_TXE;
  }
}



