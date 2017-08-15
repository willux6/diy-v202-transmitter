#include "spi.h"
#include "nRF24L01.h"
#include "V202.h"
#include "stc15.h"
#include "adc.h"
#include <stdio.h>

#define FOSC 16000000L
#define BAUD (65536 - FOSC / 4 / 115200) // 0xFFBB - 16MHz =xFFDE
#define LOBYTE(w)           ((BYTE)(WORD)(w))
#define HIBYTE(w)           ((BYTE)((WORD)(w) >> 8))
#define T1MS                (65536 - FOSC/1000)         //Initial master chip 1ms Timing
#define T100MS              (65536 - FOSC/100000)		//Initial master chip 1ms Timing
#define LEFT_BTN_PRESSED 0x80
#define RIGHT_BTN_PRESSED 0x40
#define KADC_BIT3 0x04;
#define KADC_BIT2 0x02;
#define KADC_BIT1 0x01;
#define KSC_BIT 0x08;
//#define CALIBRATE_INPUT

void InitTMR0(void);
void InitUart(void);
void putchar( char c);

// SPI interrupt protoype here, defined in spi.c
void  isr_spi(void) __interrupt (SPI_VECTOR);

// yaw = A0 (J1-Hor) ADC0
// throttle = A1 (J1-Ver) ADC1
// pitch = A2 (J2-Hor) ADC2
// roll = A3 (J2-Ver) ADC3

// STC
#include "stcdiy.h"
/*
#if 0
uint8_t txid[3] = { 0xcd, 0x31, 0x71 };
uint8_t rf_channels[16] = { 0x25, 0x2A, 0x1A, 0x3C, 0x37, 0x2B, 0x2E, 0x1D,
                            0x1B, 0x2D, 0x24, 0x3B, 0x13, 0x29, 0x23, 0x22 };
#endif
#if 0
uint8_t txid[3] = { 0x3e, 0x6a, 0xaa };
uint8_t rf_channels[16] = { 0x15, 0x1E, 0x39, 0x28, 0x2C, 0x1C, 0x29, 0x2E,
                            0x36, 0x2D, 0x18, 0x2B, 0x3A, 0x38, 0x1D, 0x1B };
#endif
#if 0
uint8_t txid[3] = { 0xcd, 0x31, 0x72 };
uint8_t rf_channels[16] = { 0x2b, 0x1f, 0x3d, 0x2c, 0x28, 0x26, 0x32, 0x3a,
                            0x1d, 0x25, 0x2d, 0x18, 0x22, 0x16, 0x31, 0x1c };
#endif
*/
// Unique binding values changed different to 0,0,0
uint8_t txid[3] = { 0x1a, 0x3f, 0x11 };

__data uint8_t flags;
__data int throttle, yaw, pitch, roll;

__data int a0, a1, a2, a3;
__data int a0min, a0max;
__data int a1min, a1max;
__data int a2min, a2max;
__data int a3min, a3max;
volatile __data long wdttimer = 0L;

void isr_tmr0(void) __interrupt (1)
{
  wdttimer++;
  WDT_CLEAR(); // 
}

// TMR0 used for WatchDog clear if enabled
void InitTMR0(void)
{
    AUXR |= 0x80;		// Timer0 in 1T mode
	TMOD = 0x00;		// set Timer0 in mode 0(16 bit auto-reloadable mode)
    TH0 = 0xfe;			// 100MS higher byte
    TL0 = 0xc0;			// 100MS lower byte
 	TR0 = 1;
    ET0 = 1;
    EA = 1;				// Enable all interrupts
}

int abs(int a){
	if( a < 0 ) return -a;
	else return a;
}

#if CALIBRATE_INPUT

void calibrate()
{
  a0min = 150; a0max=600;
  a1min = 150; a0max=600;
  a2min = 150; a0max=600;
  a3min = 150; a0max=600;
}

_Bool readInput()
{
  _Bool changed = 0;
  int a;
  a = analogRead(0); // ADC0_PIN, 0 channel
  if (a < a0min) a0min = a;
  if (a > a0max) a0max = a;
  a = (a-a0min)*255/(a0max-a0min);
  //if ( abs(a-a0)  > 4 ) changed = 1;
  if (a != a0) { changed = 1; a0 = a; }

  a = analogRead(1); // ADC1_PIN, 1 channel
  if (a < a1min) a1min = a;
  if (a > a1max) a1max = a;
  a = (a-a1min)*255/(a1max-a1min);
  //if ( abs(a-a1)  > 4 ) changed = 1;
  if (a != a1) { changed = 1; a1 = a; }

  a = analogRead(2); // ADC2_PIN, 2 channel
  if (a < a2min) a2min = a;
  if (a > a2max) a2max = a;
  a = (a2max-a)*255/(a2max-a2min);
  //if ( abs(a-a2)  > 4 ) changed = 1;
  if (a != a2) { changed = 1; a2 = a; }

  a = analogRead(6); // ADC3_PIN, 6 channel
  if (a < a3min) a3min = a;
  if (a > a3max) a3max = a;
  a = (a3max-a)*255/(a3max-a3min);
  //if ( abs(a-a3)  > 4 ) changed = 1;
  if (a != a3) { changed = 1; a3 = a; }
  return changed;
}
#else
_Bool readInput()
{
  _Bool changed = 0;
  int a;
  a = analogRead(0) / 4; // ADC0_PIN, 0 channel
  if ( a != a0) { 
   if( abs(a-a0) > 2 ) {
    changed=1; a0 = a;
   }
  }
  a = analogRead(1) / 4; // ADC1_PIN, 1 channel
  if ( a != a1) {
   if( abs(a-a1) > 2 ) {
	   changed=1; a1 = a;
   }
  }
  a = analogRead(2) / 4; // ADC2_PIN, 2 channel
  if ( a != a2) {
   if( abs(a-a2) > 2 ) {
	   changed=1; a2 = a;
   }
  }
  a = analogRead(6) / 4; // ADC3_PIN, 6 channel
  if ( a != a3) {
   if( abs(a-a3) > 2 ) {
	   changed=1; a3 = a;
   }
  }
  return changed;
}
#endif

void blinkLED(uint8_t n)
{
 uint8_t i;
 for( i = 0; i < n ; i++) {
	LED_PIN = 0;
	delay(200);
	LED_PIN = 1;
	delay(200);
 } 
}

void _tone( int msec) {
	FMQ_PIN = 0;
	delay(msec);
	FMQ_PIN = 1;
}

void main() {
	int counter = 0;
	int direction = 1;
	//int bind = 1;
	_Bool bind = 1;
	_Bool calibrated = 0;
	_Bool changed = 0;
	uint8_t res;
	long loop = 0L;
	int ksadc = 0;

// SETUP
  InitTMR0();
  InitUart();
  if (WDT_CONTR & 0x80) { // WDT_FLAG
	  printf("WDT OVR\r\n");
  }
// Set Port 3 bits mode
  P3M1 = 0x0D;
  P3M0 = 0xF2;
  WDT_CONTR |= 7; // Prescale = 6S when 16MHz clock
// Disable WatchDog
  WDT_CONTR &= ~(0x20); // EN_WDT = 0x20
//
  CE_PIN = 1;
  #ifdef CALIBRATE_INPUT
  calibrate();
  #endif
  readInput();
  V202_setTXId(txid);
  V202_begin();
  throttle = 0; yaw = 0; pitch = 0; roll = 0; flags = 0;

  res = nRF_read_register(STATUS);
  //printf("nRF status:%x\r\n", res);  
// SETUP Finished

// LOOP
 while(1) {
  //CE_PIN = 1;
  changed = readInput();
  if (changed) {
	// printf("a0=%d a1=%d a2=%d a3=%d\r\n", a0, a1, a2, a3);
  }
  if (bind) {
    throttle = a0;
    flags = 0xc0;
    /* Auto bind in 2.5 sec after turning on */
	//printf("bind\r\n");
    counter += direction;
    if (direction > 0) {
      if (counter > 256) direction = -1;
    } else {
      if (counter < 0) {
        direction = 1;
        counter = 0;
        bind = 0;
        flags = 0;
        printf("Bound1\r\n");
		blinkLED(2);
      }
    } /* */
    if (direction > 0) {
      if (throttle >= 255) direction = -1;
    } else {
      if (throttle == 0) {
        direction = 1;
        counter = 0;
        bind = 0;
        flags = 0;
        printf("Bound2\r\n");
		blinkLED(3);
     }
    }
  } else {
    throttle = a0;
    yaw = (a1 < 0x80 ? 0x7f - a1 : a1);
    roll = (a2 < 0x80 ? 0x7f - a2 : a2);
    pitch = (a3 < 0x80 ? 0x7f - a3 : a3);

    // Blinking LED lights
    counter += direction;
    if (direction > 0) {
      if (counter > 255) {
        direction = -1;
        counter = 255;
        flags = 0x10;
      }
    } else {
      if (counter < 0) {
        direction = 1;
        counter = 0;
        flags = 0x00;
      }
    }
    //
  }
  if( KSA_PIN == 0) {
	  //_tone(200); // left
	  flags |= LEFT_BTN_PRESSED;
  }
  if( KSB_PIN == 0) {
	  // blinkLED(1); // right
	  flags |= RIGHT_BTN_PRESSED;
  }
  if( KSC_PIN == 0) {
	  //_tone(200); blinkLED(1);
	  flags |= KSC_BIT;
  }
  ksadc = analogRead(7);
  //printf("KSADC=%0x\r\n", ksadc);
  if( ksadc == 0x3ff) { flags |= 0x6; }
  else if( ksadc >= 0x9e && ksadc <= 0x9f ) { flags |= 0x1; }
  else if( ksadc >= 0x12f && ksadc <= 0x132) { flags |= 0x2; }
  else if( ksadc >= 0x1c4 && ksadc <= 0x1c9 ) { flags |= 0x3; }
  else if( ksadc >= 0x263 && ksadc <= 0x269 ) {flags |= 0x4;}
  else if( ksadc >= 0x319 && ksadc <= 0x31f ) {flags |= 0x5; }
  V202_command(throttle, yaw, pitch, roll, flags);
  // 
  //printf("Loop done:%ld wt:%ld\r\n", loop++,wdttimer);
 }
}

// Uart & putchar used for printf debug messages
void InitUart( void) {
	SCON = 0x5a; //set UART mode as 8-bit variable baudrate
	// Timer 2
	T2L = 0xDE;
	T2H = 0xFF;
	AUXR = 0x14; //T2 in 1T mode and run T2
	AUXR |= 0x01; //Select T2 as UART2 baud rate generator
}

void putchar(char c) {
	while(!TI);
	TI=0;
	SBUF = c;
}

