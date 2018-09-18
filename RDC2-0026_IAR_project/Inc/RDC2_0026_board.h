/*
********************************************************************************
* COPYRIGHT(c) ��� ���� � ��ϻ, 2017
* 
* ����������� ����������� ��������������� �� �������� ���� ����� (as is).
* ��� ��������������� �������� ������ �����������.
********************************************************************************
*/




#ifndef __RDC2_0026_BOARD_H
#define __RDC2_0026_BOARD_H

#include "stm32f042x6.h"

//���������
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

//���� ������
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
  uint32_t Volume; //����� ������ � ������ //- 1, �.�. ����� ���������� �����
                   //��� I2C: �� 1 ���� �� 2 ����
                   //��� SPI EEPROM: �� 1 ���� �� 2 ����
                   //��� SPI flash: �� 256 ���� �� 128 ����
                   //��� MICROWIRE: �� 1 ���� �� 16 ����
  uint16_t PageSize; //�� 8 �� 256 ����
  uint8_t AdrBytes; //���������� ���� ��� ��������� ������;
                    //��� SPI flash ������ 3 ����� �� 128 ����
                    //��� SPI EEPROM 25��, 95�� � FRAM ������� �� ������: �� 4 ���� ������������ - 1 ����;
                                                                           //����� 4 ���� - 2 �����
                                                                           //�� 1 ���� � ���� - 3 �����
                    //��� I2C ������� �� ������: �� 16 ���� ������������ - 1 ����
                                                         //����� 16 ���� - 2 �����
                    //��� MICROWIRE ������ ����� ����
  
  uint8_t AdrBitsInCmd; //���������� ��� � ������� ��� ��������� ������
                        //��� I2C - � ������ ���������� �0-�2 ������������ �� ��� ����� ����������, � ��� ����� ������ ��������������� ������
                        //��� I2C ���������� 24AA515 (Microchip) �2 - ������� ��� ������
                        //��� SPI EEPROM 25��, 95�� � FRAM - 1 ��� � ����� ������� ��� ������ 4 ����, � ��������� ������� ����.
                        //��� SPI flash - ������ ����� ����
                        //��� MICROWIRE - ���������� ��� ��� ��������� ������
//��������� ������ ��������� 93�� MICROWIRE
//  ����� | 1 ���� (93��46) | 2 ���� (93��57) | 2 ���� (93��56) / 4 ���� (93��66) | 8 ���� (93��76) / 16 ���� (93��86) | 
//  8 ��� |     �6 - �0     |     �7 - �0     |             �8 - �0               |             �10 - �0               |
// 16 ��� |     �5 - �0     |     �6 - �0     |             �7 - �0               |              �9 - �0               |
  
  uint8_t Frequency; //������ �������� �������
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

