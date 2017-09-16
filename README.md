<b>STC 2.4G Open Source DIY transmitter: V202 protocol</b>

Program for <a href="https://www.rcgroups.com/forums/showthread.php?2702433-STC-DIY-2-4G-Open-Source-Remote-Control-Transmitter-C51-Development-Board">Chinese Open Source STC15W408AS mcu based transmitter</a> module with nRF24L01 2.4 GHz Radio module.
STC15W408AS module uses Hardware SPI to communicate with nRF24L01 and messaging protocol V202.
As experiments proved the transmitter hardware module has bug - MISO and MOSI connections should be changed to use hardware SPI.

V202 protocol software was ported from Arduino to C51. This project is compiled with sdcc compiler (compile.bat).
Programmer: I used stc-isp-15xx-v6.86.exe over CP2102 USB to Serial module. MCU module should programmed to use 16MHz clock.

This module is tested only with generic Arduino Nano v3 based receiver module (for example this: https://github.com/execuc/v202-receiver)
