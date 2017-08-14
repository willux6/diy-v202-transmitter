STC 2.4GHz Open Source DIY transmitter: V202 protocol

Program for Open Source STC15W408AS transmitter module which uses V202 protocol and uses nRF24L01 2.4 GHz Radio module.
STC15W408AS module uses Hardware SPI to communicate with nRF24L01.
Transmitter hardware module has bug - MISO and MOSI connections should be changed to use hardware SPI.


V202 protocol software was ported from Arduino to C51. This project uses sdcc compiler (compile.bat).
Programmer: stc-isp-15xx-v6.86.exe over CP2102 USB to Serial module. MCU module should programmed to use 16MHz clock.
