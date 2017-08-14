#include "stc15.h"

// SPI interface
#define ADC2_PIN P1_2	// Connected Joystick 2 Vertical pot (Actually ADC2)
#define SS_PIN P1_2		// not connected 
#define MISO_PIN P1_3	// Connected nRF24L01 MISO pin
#define MOSI_PIN P1_4	// Connected nRF24L01 MOSI pin
#define SCLK_PIN P1_5	// Connected nRF24L01 SCK pin
#define ADC3_PIN P1_6	// Connected Joystick 2 Horizontal pot (Actually ADC6)
#define SCN_PIN P3_5	// Connected nRF24L01 SCN pin
#define CSN_PIN P3_5
#define CE_PIN P3_4		// Connected nRF24L01 CE pin
#define IRQ_PIN P3_3	// Connected nRF24L01 IRQ pin

#define KSADC_PIN P1_7	// Connected switch block SW1..SW8 (ADC7)
#define KSB_PIN P5_4	// Connected switch S2 - Right button
#define KSC_PIN P5_5	// Connected switch SW11
#define ADC1_PIN P1_1	// Connected Joystick 1 Horizontal pot (Actually ADC1)
#define ADC0_PIN P1_0	// Connected Joystick 1 Vertical pot (Actually ADC0)
#define LED_PIN P3_7	// Connected built-in Blue Led
#define FMQ_PIN P3_6	// Connected speaker
#define KSA_PIN P3_2	// Connected switch S1 - Left button
#define TXD1_PIN P3_1	// Connected serial port TXD
#define RXD1_PIN P3_0	// Connected serial port RXD
#define WDT_CLEAR()    (WDT_CONTR |= 1 << 4)
#ifndef FOSC
 #define FOSC 16000000L
#endif
#define BR(n)               (65536 - FOSC/4/(n))        //Master chip baud rate calculation formula

void _delay_ms(uint8_t ms);
void delay( uint8_t n);
void delayMicroseconds( uint8_t us);
