/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/

#ifndef __MICROWIRE_MEMORY_H
#define __MICROWIRE_MEMORY_H


#include "RDC2_0026_board.h"

//таймер
#define       MICROWIRE_TIMER              TIM17
#define       MICROWIRE_TIMER_ENR          APB2ENR
#define       MICROWIRE_TIMER_CLK_EN       RCC_APB2ENR_TIM17EN
#define       MICROWIRE_TIMER_IRQ          TIM17_IRQn

#define       MICROWIRE_PRIORITY           1


//состояния
#define       MICROWIRE_READ               (1 << 0)
#define       MICROWIRE_WRITE              (1 << 1)
#define       MICROWIRE_EMPTY_CLOCK        (1 << 2)
#define       MICROWIRE_START              (1 << 3)
#define       MICROWIRE_CMD                (1 << 4)
#define       MICROWIRE_ADDRESS            (1 << 5)
#define       MICROWIRE_DATA               (1 << 6)
#define       MICROWIRE_CLOCK_PHASE        (1 << 7)

#define       MICROWIRE_TRANSFER_MASK      (MICROWIRE_EMPTY_CLOCK | MICROWIRE_START | \
                                            MICROWIRE_CMD | MICROWIRE_ADDRESS | MICROWIRE_DATA)

#define       MICROWIRE_EMPTY_CLOCK_BITS   1
#define       MICROWIRE_START_BITS         1
#define       MICROWIRE_CMD_BITS           2

//команды
#define       MICROWIRE_CMD_READ           0x02
#define       MICROWIRE_CMD_WRITE          0x01


#define       MICROWIRE_EWDS               0
#define       MICROWIRE_EWEN               1




void MICROWIRE_Memory_Init(uint8_t TimingSet);

void MICROWIRE_Memory_Write(MemoryIC_Type* Memory, uint8_t* DataArray);

void MICROWIRE_Memory_Read(MemoryIC_Type* Memory, uint8_t* DataArray);

void MICROWIRE_Memory_DeInit();

void MICROWIRE_Memory_IsReady(uint16_t CS_PIN, uint8_t *Status);

void MICROWIRE_Memory_IsPresent(uint16_t CS_PIN, uint8_t* Status);

void MICROWIRE_TIMER_ISR(void);

void MICROWIRE_Start_Transfer(MemoryIC_Type* Memory, uint8_t* DataArray);

void MICROWIRE_Memory_Clock_Pulse();

void MICROWIRE_Memory_EWEN_EWDS(uint16_t CS_PIN, uint8_t AdrBits, uint8_t Cmd);


#endif //__MICROWIRE_MEMORY_H


