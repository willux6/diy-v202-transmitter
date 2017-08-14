#include "spi.h"
#include "stcdiy.h"
#include "nRF24L01.h"
#include <stdio.h>

// Hardware SPI

uint8_t SPI_transfer(uint8_t dat);

#define PAYLOAD_SIZE 16
uint8_t payload_size = PAYLOAD_SIZE;
uint8_t dynamic_payloads_enabled  = 0;
SPI_InitTypeDef SPI;

/****************************************************************************/

void nRF_csn(int mode) {
  CSN_PIN = mode;
}

/****************************************************************************/

void nRF_ce(int level)
{
  CE_PIN = level;
}

/****************************************************************************/

void nRF_begin() {
	SPI.SPI_Module = ENABLE;			//ENABLE,DISABLE
	SPI.SPI_SSIG = DISABLE;				//ENABLE, DISABLE // 1.2 is used as ADC2
	SPI.SPI_FirstBit = SPI_MSB;			//SPI_MSB, SPI_LSB
	SPI.SPI_Mode = SPI_Mode_Master;		//SPI_Mode_Master, SPI_Mode_Slave
	SPI.SPI_CPOL = SPI_CPOL_Low;		//SPI_CPOL_High,   SPI_CPOL_Low
	SPI.SPI_CPHA = SPI_CPHA_1Edge;		//SPI_CPHA_1Edge,  SPI_CPHA_2Edge
	SPI.SPI_Interrupt = ENABLE;			//ENABLE,DISABLE
	SPI.SPI_Speed = SPI_Speed_128;			//SPI_Speed_4,      SPI_Speed_16,SPI_Speed_64,SPI_Speed_128
	SPI.SPI_IoUse = SPI_P12_P13_P14_P15;//SPI_P12_P13_P14_P15, SPI_P24_P23_P22_P21, SPI_P54_P40_P41_P43
	SPI_Init(&SPI);
	payload_size = PAYLOAD_SIZE;
	dynamic_payloads_enabled = 0;
 }

uint8_t nRF_read_register_buf(uint8_t reg, uint8_t* buf, uint8_t len)
{
  uint8_t status;
  //nRF_ce(1);
  nRF_csn(0);
  status = SPI_transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- )
    *buf++ = SPI_transfer(0xff);

  nRF_csn(1);
  //nRF_ce(0);

  return status;
}

/****************************************************************************/

uint8_t nRF_read_register(uint8_t reg)
{
  uint8_t result;
  nRF_csn(0);
  SPI_transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
  result = SPI_transfer(0xff);

  nRF_csn(1);
  return result;
}

/****************************************************************************/

uint8_t nRF_write_register_buf(uint8_t reg, const uint8_t* buf, uint8_t len)
{
  uint8_t status;

  nRF_csn(0);
  status = SPI_transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  while ( len-- )
    SPI_transfer(*buf++);

  nRF_csn(1);

  return status;
}

/****************************************************************************/

uint8_t nRF_write_register(uint8_t reg, uint8_t value)
{
  uint8_t status;

  nRF_csn(0);
  //printf("nRF_write_register:%d:%x\r\n", reg, value);
  status = SPI_transfer( W_REGISTER | ( REGISTER_MASK & reg ) );
  SPI_transfer(value);
  nRF_csn(1);

  return status;
}

/****************************************************************************/

uint8_t nRF_write_payload(const void* buf, uint8_t len)
{
  uint8_t status, data_len, blank_len;

  uint8_t* current;
  current = (uint8_t*)(buf);

  data_len = (len < payload_size) ? len : PAYLOAD_SIZE;
  blank_len = (dynamic_payloads_enabled != 0) ? 0 : PAYLOAD_SIZE - data_len;
  
  //printf("[Writing %d bytes %d blanks] l:%u  ps:%u\r\n",data_len,blank_len,len,payload_size);
  
  nRF_csn(0);
  status = SPI_transfer( W_TX_PAYLOAD );
  while ( data_len-- )
    SPI_transfer(*current++);
  while ( blank_len-- )
    SPI_transfer(0);
  nRF_csn(1);

  return status;
}

/****************************************************************************/

uint8_t nRF_read_payload(void* buf, uint8_t len)
{
  uint8_t status;
  uint8_t* current = (uint8_t*)(buf);

  uint8_t data_len = (len < payload_size)?len:PAYLOAD_SIZE;
  uint8_t blank_len = dynamic_payloads_enabled ? 0 : PAYLOAD_SIZE - data_len;
  
  //printf("[Reading %u bytes %u blanks]\r\n",data_len,blank_len);
  
  nRF_csn(0);
  status = SPI_transfer( R_RX_PAYLOAD );
  while ( data_len-- )
    *current++ = SPI_transfer(0xff);
  while ( blank_len-- )
    SPI_transfer(0xff);
  nRF_csn(1);

  return status;
}

/****************************************************************************/

uint8_t nRF_flush_rx(void)
{
  uint8_t status;

  nRF_csn(0);
  status = SPI_transfer( FLUSH_RX );
  nRF_csn(1);

  return status;
}

/****************************************************************************/

uint8_t nRF_flush_tx(void)
{
  uint8_t status;

  nRF_csn(0);
  status = SPI_transfer( FLUSH_TX );
  nRF_csn(1);

  return status;
}

/****************************************************************************/

void nRF_activate(uint8_t code)
{
  nRF_csn(0);
  SPI_transfer(ACTIVATE);
  SPI_transfer(code);
  nRF_csn(1);
}
