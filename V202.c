#include <stdint.h>
#include <stdio.h>
#include "V202.h"
#include "nRF24L01.h"

#define _nop_ __asm nop __endasm;

// This is frequency hopping table for V202 protocol
// The table is the first 4 rows of 32 frequency hopping
// patterns, all other rows are derived from the first 4.
// For some reason the protocol avoids channels, dividing
// by 16 and replaces them by subtracting 3 from the channel
// number in this case.
// The pattern is defined by 5 least significant bits of
// sum of 3 bytes comprising TX id
uint8_t freq_hopping[][16] = {
 { 0x27, 0x1B, 0x39, 0x28, 0x24, 0x22, 0x2E, 0x36,
   0x19, 0x21, 0x29, 0x14, 0x1E, 0x12, 0x2D, 0x18 }, //  00
 { 0x2E, 0x33, 0x25, 0x38, 0x19, 0x12, 0x18, 0x16,
   0x2A, 0x1C, 0x1F, 0x37, 0x2F, 0x23, 0x34, 0x10 }, //  01
 { 0x11, 0x1A, 0x35, 0x24, 0x28, 0x18, 0x25, 0x2A,
   0x32, 0x2C, 0x14, 0x27, 0x36, 0x34, 0x1C, 0x17 }, //  02
 { 0x22, 0x27, 0x17, 0x39, 0x34, 0x28, 0x2B, 0x1D,
   0x18, 0x2A, 0x21, 0x38, 0x10, 0x26, 0x20, 0x1F }  //  03
};
//  nRF24& radio;
  uint8_t _txid[3];
  uint8_t rf_channels[16];
  uint8_t packet_sent;
  uint8_t rf_ch_num;

void _delay_ms(uint8_t ms)
{
    // delay function, tuned for 11.092 MHz clock
    // optimized to assembler
    ms; // keep compiler from complaining?
    __asm;
        ; dpl contains ms param value
    delay$:
        mov	b, #8   ; i
    outer$:
        mov	a, #243    ; j
    inner$:
        djnz acc, inner$
        djnz b, outer$
        djnz dpl, delay$
    __endasm;
	//optimized for 16MHz
/*	unsigned char i, j;
	uint8_t k;
	for( k = 0; k < ms; k++){
		i = 16;
		j = 141;
		do
		{
			while (--j);
		} while (--i);
	}*/
}

void delay( uint8_t n)
{
	_delay_ms(n);
}

void delayMicroseconds( uint8_t us)
{
	//tuned for 16MHz
	uint8_t i, k;
	k = us;
	do {
		_nop_;
		_nop_;
		i = 2;
		while (--i);
	} while (--k);
}
  
void V202_setTXId(uint8_t txid_[3])
{
  //uint8_t fh_row[16];
  uint8_t sum, increment, val;
  int i;
  _txid[0] = txid_[0];
  _txid[1] = txid_[1];
  _txid[2] = txid_[2];
  sum = _txid[0] + _txid[1] + _txid[2];
  // Base row is defined by lowest 2 bits
  //(&fh_row)[16] = freq_hopping[sum & 0x03];
  // Higher 3 bits define increment to corresponding row
  increment = (sum & 0x1e) >> 2;
  for (i = 0; i < 16; ++i) {
    val = freq_hopping[sum & 0x03][i] + increment;
    // Strange avoidance of channels divisible by 16
    rf_channels[i] = (val & 0x0f) ? val : val - 3;
	//printf("Ch:%02d freq:%04x\r\n", i, rf_channels[i]);
  }
}

void V202_begin()
{
  uint8_t reg;
  //uint8_t* rx_tx_addr;
  //uint8_t* rx_p1_addr;
  //nRF_ce(1);
  nRF_begin();
  nRF_write_register(CONFIG, _BV(EN_CRC) | _BV(CRCO));
  nRF_write_register(EN_AA, 0x00);
  nRF_write_register(EN_RXADDR, 0x3F);
  nRF_write_register(SETUP_AW, 0x03);
  nRF_write_register(SETUP_RETR, 0xFF);
  nRF_write_register(RF_CH, 0x08);
  nRF_write_register(RF_SETUP, 0x05); // 0x05 - 1Mbps, 0dBm power, LNA high gaim
  nRF_write_register(STATUS, 0x70);
  nRF_write_register(OBSERVE_TX, 0x00);
  nRF_write_register(CD, 0x00);
  nRF_write_register(RX_ADDR_P2, 0xC3);
  nRF_write_register(RX_ADDR_P3, 0xC4);
  nRF_write_register(RX_ADDR_P4, 0xC5);
  nRF_write_register(RX_ADDR_P5, 0xC6);
  nRF_write_register(RX_PW_P0, 0x10);
  nRF_write_register(RX_PW_P1, 0x10);
  nRF_write_register(RX_PW_P2, 0x10);
  nRF_write_register(RX_PW_P3, 0x10);
  nRF_write_register(RX_PW_P4, 0x10);
  nRF_write_register(RX_PW_P5, 0x10);
  nRF_write_register(FIFO_STATUS, 0x00);
  //rx_tx_addr = (uint8_t *)("\x66\x88\x68\x68\x68");
  //rx_p1_addr = (uint8_t *)("\x88\x66\x86\x86\x86");
  //nRF_write_register_buf(RX_ADDR_P0, rx_tx_addr, 5);
  //nRF_write_register_buf(RX_ADDR_P1, rx_p1_addr, 5);
  //nRF_write_register_buf(TX_ADDR, rx_tx_addr, 5);
  nRF_write_register_buf(RX_ADDR_P0, (uint8_t *)"\x66\x88\x68\x68\x68", 5);
  nRF_write_register_buf(RX_ADDR_P1, (uint8_t *)"\x88\x66\x86\x86\x86", 5);
  nRF_write_register_buf(TX_ADDR, (uint8_t *)"\x66\x88\x68\x68\x68", 5);
  // Check for Beken BK2421/BK2423 chip
  // It is done by using Beken specific activate code, 0x53
  // and checking that status register changed appropriately
  // There is no harm to run it on nRF24L01 because following
  // closing activate command changes state back even if it
  // does something on nRF24L01
  nRF_activate(0x53); // magic for BK2421 bank switch
  //Serial.write("Try to switch banks "); Serial.print(nRF_read_register(STATUS)); Serial.write("\n");
  //while( !(nRF_read_register(STATUS) & 0x80)); // lisatud 20170806 testiks, et kas saab siit edasi? Järgmise bloki if-i kommenteerisin
  if (nRF_read_register(STATUS) & 0x80) {
    //Serial.write("BK2421!\n");
	printf("BK2421!\r\n");
     nRF_write_register_buf(0x00, (uint8_t *) "\x40\x4B\x01\xE2", 4);
    nRF_write_register_buf(0x01, (uint8_t *) "\xC0\x4B\x00\x00", 4);
    nRF_write_register_buf(0x02, (uint8_t *) "\xD0\xFC\x8C\x02", 4);
    nRF_write_register_buf(0x03, (uint8_t *) "\xF9\x00\x39\x21", 4);
    nRF_write_register_buf(0x04, (uint8_t *) "\xC1\x96\x9A\x1B", 4);
    nRF_write_register_buf(0x05, (uint8_t *) "\x24\x06\x7F\xA6", 4);
	for( reg = 0x06; reg < 0x0C; reg++ ) { // six row zeroes
		nRF_write_register_buf(reg, ( uint8_t *) "\x00\x00\x00\x00", 4);
	}
    nRF_write_register_buf(0x0C, ( uint8_t *) "\x00\x12\x73\x00", 4);
    nRF_write_register_buf(0x0D, ( uint8_t *) "\x46\xB4\x80\x00", 4);
    nRF_write_register_buf(0x0E, ( uint8_t *) "\x41\x10\x04\x82\x20\x08\x08\xF2\x7D\xEF\xFF", 11);
    nRF_write_register_buf(0x04, ( uint8_t *) "\xC7\x96\x9A\x1B", 4);
    nRF_write_register_buf(0x04, ( uint8_t *) "\xC1\x96\x9A\x1B", 4);
  }
  nRF_activate(0x53); // switch bank back
  
  delay(100); // suurendasin 100-ni, oli 50 20170806
  nRF_flush_tx();
  nRF_write_register(CONFIG, _BV(EN_CRC) | _BV(CRCO) | _BV(PWR_UP));
  delayMicroseconds(150);
//  packet_sent = true;
  rf_ch_num = 0;
  // This is a bogus packet which actual V202 TX sends, but it is
  // probably some garbage, so no need to send it.
//  uint8_t buf[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9B,
//                      0x9C, 0x88, 0x48, 0x8F, 0xD3, 0x00, 0xDA, 0x8F };
//  nRF_flush_tx();
//  nRF_write_payload(buf, 16);
  //*** nRF_ce(DISABLE);
//  delayMicroseconds(15);
  // It saves power to turn off radio after the transmission,
  // so as long as we have pins to do so, it is wise to turn
  // it back.
//  nRF_ce(ENABLE);
}

void V202_command(uint8_t throttle, int yaw, int pitch, int roll, uint8_t flags)
{
  uint8_t sum, i, report_done, rf_ch;
  uint8_t buf[16];
  if (flags == 0xc0) {
    // binding
    buf[0] = 0x00;
    buf[1] = 0x00;
    buf[2] = 0x00;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x00;
    buf[6] = 0x00;
  } else {
    // regular packet
    buf[0] = throttle;
    buf[1] = (uint8_t) yaw;
    buf[2] = (uint8_t) pitch;
    buf[3] = (uint8_t) roll;
    // Trims, middle is 0x40
    buf[4] = 0x40; // yaw
    buf[5] = 0x40; // pitch
    buf[6] = 0x40; // roll
  }
  // TX id
  buf[7] = _txid[0];
  buf[8] = _txid[1];
  buf[9] = _txid[2];
  // empty
  buf[10] = 0x00;
  buf[11] = 0x00;
  buf[12] = 0x00;
  buf[13] = 0x00;
  //
  buf[14] = flags;
  sum = 0;
  for (i = 0; i < 15;  ++i) sum += buf[i];
  buf[15] = sum;
  //*** nRF_ce(ENABLE);
  if (packet_sent) {
    report_done = 0;
    //if  (!(nRF_read_register(STATUS) & _BV(TX_DS))) { Serial.write("Waiting for radio\n"); report_done = true; }
    while (!(nRF_read_register(STATUS) & _BV(TX_DS))) ;
	//printf("Status-read:%04x\r\n",nRF_read_register(STATUS));
    nRF_write_register(STATUS, _BV(TX_DS));
//    if (report_done) Serial.write("Done\n");
  }
  packet_sent = 1;
  // Each packet is repeated twice on the same
  // channel, hence >> 1
  // We're not strictly repeating them, rather we
  // send new packet on the same frequency, so the
  // receiver gets the freshest command. As receiver
  // hops to a new frequency as soon as valid packet
  // received it does not matter that the packet is
  // not the same one repeated twice - nobody checks this
  rf_ch = rf_channels[rf_ch_num >> 1];
  rf_ch_num++; if (rf_ch_num >= 32) rf_ch_num = 0;
  //printf("Ch:%04x\r\n", rf_ch);
  nRF_write_register(RF_CH, rf_ch);
  nRF_flush_tx();
  nRF_write_payload(buf, 16);
  //printf("T:%d Y:%d P:%d R:%d F:%x tx0:%x tx1:%x tx2:%x\r\n", throttle, yaw, pitch, roll, flags, _txid[0], _txid[1], _txid[2]);
  //nRF_ce(DISABLE);
  delayMicroseconds(15);
  //nRF_ce(ENABLE);
  //*** nRF_ce(DISABLE);
}
