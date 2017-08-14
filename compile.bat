sdcc -mmcs51 --model-large --iram-size 512 -c adc.c
sdcc -mmcs51 --model-large --iram-size 512 -c spi.c
sdcc -mmcs51 --model-large --iram-size 512 -c nRF24L01.c
sdcc -mmcs51 --model-large --iram-size 512 -c V202.c
sdcc -mmcs51 --model-large --iram-size 512 V202tx_stc.c V202.rel nRF24L01.rel spi.rel adc.rel
packihx V202tx_stc.ihx >main.hex
