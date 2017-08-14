#ifndef	__SPI_H
#define	__SPI_H
#include "stc15.h"
#include <stdint.h>

#define	SPI_Mode_Master		1
#define	SPI_Mode_Slave		0
#define	SPI_CPOL_High		1
#define	SPI_CPOL_Low		0
#define	SPI_CPHA_1Edge		1
#define	SPI_CPHA_2Edge		0
#define	SPI_Speed_4			0
#define	SPI_Speed_16		1
#define	SPI_Speed_64		2
#define	SPI_Speed_128		3
#define	SPI_MSB				0
#define	SPI_LSB				1
#define	SPI_P12_P13_P14_P15	(0<<2)
#define	SPI_P24_P23_P22_P21	(1<<2)
#define	SPI_P54_P40_P41_P43	(2<<2)
#define SPIF 0x80	// SPIF - SPI transfer completion flag bit 7
#define WCOL 0x40	// WCOL - SPI write collision flag bit 6
#define ENABLE 1		// Testing
#define DISABLE 0
#define SPI_SS P3_4		// CE_PIN in schema, not SS_PIN which is used as ADC2
#define SPI_VECTOR 9	// SPI Interrupt vector

typedef struct
{
	uint8_t	SPI_Module;		//ENABLE,DISABLE
	uint8_t	SPI_SSIG;		//ENABLE, DISABLE
	uint8_t	SPI_FirstBit;	//SPI_MSB, SPI_LSB
	uint8_t	SPI_Mode;		//SPI_Mode_Master, SPI_Mode_Slave
	uint8_t	SPI_CPOL;		//SPI_CPOL_High,   SPI_CPOL_Low
	uint8_t	SPI_CPHA;		//SPI_CPHA_1Edge,  SPI_CPHA_2Edge
	uint8_t	SPI_Interrupt;		//ENABLE,DISABLE
	uint8_t	SPI_Speed;		//SPI_Speed_4,      SPI_Speed_16,SPI_Speed_64,SPI_Speed_128
	uint8_t	SPI_IoUse;		//SPI_P12_P13_P14_P15, SPI_P24_P23_P22_P21, SPI_P54_P40_P41_P43
} SPI_InitTypeDef;

void	SPI_Init(SPI_InitTypeDef *SPIx);
uint8_t SPI_transfer(uint8_t dat);

#endif
