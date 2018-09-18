/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/


#ifndef __I2C_MEMORY_H
#define __I2C_MEMORY_H


#include "RDC2_0026_board.h"



#define       I2C_MEMORY_ADDRESS           0x50


//таймер
#define       I2C_TIMER                    TIM16
#define       I2C_TIMER_ENR                APB2ENR
#define       I2C_TIMER_CLK_EN             RCC_APB2ENR_TIM16EN


void I2C_Memory_Init(uint8_t TimingSet);

void I2C_Memory_Write(MemoryIC_Type* Memory, uint8_t* DataArray);

void I2C_Memory_Read(MemoryIC_Type* Memory, uint8_t* DataArray);

void I2C_Memory_DeInit();

void I2C_Memory_IsReady(uint16_t CS_PIN, uint8_t *Status);


#endif //__I2C_MEMORY_H


