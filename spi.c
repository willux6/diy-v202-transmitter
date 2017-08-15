#define SPI_C
#include "spi.h"
#include <stdio.h>
#define SPI_INTER	// process spi interrupt

//========================================================================
// function: void	SPI_Init(SPI_InitTypeDef *SPIx)
// description: SPI initialize the program.
// parameter: SPIx: Structural parameters, please refer to spi.h in the definition.
// return: none.
//========================================================================
// Both methods with interrupt or without interrupt works
#ifdef SPI_INTER
volatile __data uint8_t retval = 0;
volatile __data _Bool spi_int_done = 0;
#endif

void	SPI_Init(SPI_InitTypeDef *SPIx)
{
	if(SPIx->SPI_SSIG == ENABLE)			SPCTL &= ~(1<<7);	//enable SS, conform Master or Slave by SS pin.
	else									SPCTL |=  (1<<7);	//disable SS, conform Master or Slave by SPI_Mode
	if(SPIx->SPI_Module == ENABLE)			SPCTL |=  (1<<6);	//SPI enable
	else									SPCTL &= ~(1<<6);	//SPI disable
	if(SPIx->SPI_FirstBit == SPI_LSB)		SPCTL |= ~(1<<5);	//LSB first
	else									SPCTL &= ~(1<<5);	//MSB first
	if(SPIx->SPI_Mode == SPI_Mode_Slave)	SPCTL &= ~(1<<4);	//slave
	else									SPCTL |=  (1<<4);	//master
	if(SPIx->SPI_CPOL == SPI_CPOL_High)		SPCTL |=  (1<<3);	//SCLK Idle High, Low Active.
	else									SPCTL &= ~(1<<3);	//SCLK Idle Low, High Active.
	if(SPIx->SPI_CPHA == SPI_CPHA_2Edge)	SPCTL |=  (1<<2);	//sample at the second edge
	else									SPCTL &= ~(1<<2);	//sample at the first  edge
	if(SPIx->SPI_Interrupt == ENABLE)		IE2 |=  (1<<1);		// ESPI = 1 in IE2
	else									IE2 &= ~(1<<1);
	SPCTL = (SPCTL & ~3) | (SPIx->SPI_Speed & 3);					//set speed
	AUXR1 = (AUXR1 & ~(3<<2)) | SPIx->SPI_IoUse;
	SPDAT = 0;
	#ifdef SPI_INTER
	IP2 |= 0x02; // PSPI SPI interrupt is assigned highest priority
	IE |= 0x80; 	// EA = 1, each interrupt source is individually enabled or disabled
	EA = 1;
	#endif
}

uint8_t SPI_transfer(uint8_t dat)
{
	#ifndef SPI_INTER
	uint8_t ret;
	SPDAT = dat;
	while( !(SPSTAT & SPIF) );
	ret = SPDAT;
	SPSTAT = SPIF | WCOL;	//Clear 0 SPIF with WCOL logo
	return ret;
	#else
	SPDAT = dat;
	while ( !spi_int_done) ;
	spi_int_done = 0;
	return retval;
	#endif
}

void  isr_spi(void) __interrupt (SPI_VECTOR)
{
	#ifdef SPI_INTER
	retval = SPDAT;
	spi_int_done = 1;
	SPSTAT = SPIF | WCOL;
	#endif
}
