/*
 * main.h
 *
 * Created: 11-4-2021 18:43:09
 *  Author: joris
 */ 


#ifndef MAIN_H_
#define MAIN_H_

#define SPI_SS_bm             0x10 // bit mask for the SS pin
#define SPI_MOSI_bm           0x20 // bit mask for the MOSI pin
#define SPI_MISO_bm           0x40 // bit mask for the MISO pin
#define SPI_SCK_bm            0x80 // bit mask for the SCK pin

#define SPI_WAIT	while(!(SPIC.STATUS & SPI_IF_bm))	// wait for assertion of IF (transmit/receive completed)
#define SPI_CS_ON	PORTD.OUTCLR = SPI_SS_bm			// assert SS
#define SPI_CS_OFF	PORTD.OUTSET = SPI_SS_bm			// deassert SS



#endif /* MAIN_H_ */