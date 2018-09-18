/*
********************************************************************************
* COPYRIGHT(c) ЗАО «ЧИП и ДИП», 2017
* 
* Программное обеспечение предоставляется на условиях «как есть» (as is).
* При распространении указание автора обязательно.
********************************************************************************
*/




#ifndef __RDC2_0026_BOARD_H
#define __RDC2_0026_BOARD_H

#include "stm32f042x6.h"

//светодиод
#define       LED_GPIO                     GPIOA
#define       LED_PIN                      0

//SPI
#define       SPI_GPIO                     GPIOA
#define       SPI_PORT                     SPI1
#define       SPI_SCK_PIN                  5
#define       SPI_SCK_AF                   0
#define       SPI_MISO_PIN                 6
#define       SPI_MISO_AF                  0
#define       SPI_MOSI_PIN                 7
#define       SPI_MOSI_AF                  0
#define       SPI_NSS1_PIN                 4
#define       SPI_NSS2_PIN                 3
#define       SPI_NSS3_PIN                 2
#define       SPI_NSS4_PIN                 1

//93xx_ORG
#define       MICROWIRE_ORG_GPIO           GPIOB
#define       MICROWIRE_ORG_PIN            1

//I2C
#define       I2C_SCL_PIN                  1
#define       I2C_SCL_PORT                 GPIOF
#define       I2C_SCL_AF                   1
#define       I2C_SDA_PIN                  0
#define       I2C_SDA_PORT                 GPIOF
#define       I2C_SDA_AF                   1
#define       I2C_GPIO                     GPIOF
#define       I2C_PORT                     I2C1

//типы памяти
#define       I2C_MEMORY                   1
#define       SPI_MEMORY_EEPROM            2
#define       SPI_MEMORY_FLASH             3
#define       MICROWIRE_EEPROM_ORG_8       4
#define       MICROWIRE_EEPROM_ORG_16      5


#define       DATA_BUFFER_SIZE             4096
#define       MEMORY_DETECTED              0xAA
#define       I2C_READ_MAX_BYTES_NUM       128



extern const uint32_t I2C_TIMINGR[];
extern const uint16_t I2C_TIMER_PCS[];
extern const uint8_t SPI_CHIP_SEL_OUT[];


typedef struct
{
  uint8_t Type;
  uint32_t Volume; //объем памяти в байтах //- 1, т.е. адрес последнего байта
                   //для I2C: от 1 кбит до 2 Мбит
                   //для SPI EEPROM: от 1 кбит до 2 Мбит
                   //для SPI flash: от 256 кбит до 128 Мбит
                   //для MICROWIRE: от 1 кбит до 16 кбит
  uint16_t PageSize; //от 8 до 256 байт
  uint8_t AdrBytes; //количество байт для адресации памяти;
                    //для SPI flash всегда 3 байта до 128 Мбит
                    //для SPI EEPROM 25хх, 95хх и FRAM зависит от объема: до 4 кбит включительно - 1 байт;
                                                                           //более 4 кбит - 2 байта
                                                                           //от 1 Мбит и выше - 3 байта
                    //для I2C зависит от объема: до 16 кбит включительно - 1 байт
                                                         //более 16 кбит - 2 байта
                    //для MICROWIRE всегда равно нулю
  
  uint8_t AdrBitsInCmd; //количество бит в команде для адресации памяти
                        //для I2C - в адресе устройства А0-А2 используются не как адрес устройства, а как адрес памяти пропорционально объему
                        //для I2C микросхемы 24AA515 (Microchip) А2 - старший бит адреса
                        //для SPI EEPROM 25хх, 95хх и FRAM - 1 бит в байте команды для объема 4 кбит, в остальных случаях ноль.
                        //для SPI flash - всегда равно нулю
                        //для MICROWIRE - количество бит для адресации памяти
//адресация памяти микросхем 93хх MICROWIRE
//  объем | 1 кбит (93хх46) | 2 кбит (93хх57) | 2 кбит (93хх56) / 4 кбит (93хх66) | 8 кбит (93хх76) / 16 кбит (93хх86) | 
//  8 бит |     А6 - А0     |     А7 - А0     |             А8 - А0               |             А10 - А0               |
// 16 бит |     А5 - А0     |     А6 - А0     |             А7 - А0               |              А9 - А0               |
  
  uint8_t Frequency; //индекс настроек частоты
  uint16_t SPI_CS;
  uint16_t TransBytes;
  uint32_t CurAdr;
  
} MemoryIC_Type;


typedef struct
{
  void (*Bus_Init)(uint8_t FreqSet);
  void (*Write)(MemoryIC_Type* Memory, uint8_t* DataArray);
  void (*Read)(MemoryIC_Type* Memory, uint8_t* DataArray);
  void (*IsReady)(uint16_t CS_PIN, uint8_t* Status);
  void (*IsPresent)(uint16_t CS_PIN, uint8_t* Status);
  void (*Bus_DeInit)(void);
  
} MemoryFunc_Type;


void RDC2_0026_Init();

uint16_t* RDC2_0026_GetDataPos();


#endif //__RDC2_0026_BOARD_H

