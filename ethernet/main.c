/*
 * ethernet.c
 *
 * Created: 11-4-2021 18:04:02
 * Author : joris
 */ 
#define F_CPU 32000000UL		// 32MHz clock

#include <avr/io.h>
#include <avr/interrupt.h>
#include "ENCx24J600.h"

// Define UDP datagram fields
uint8_t SourceAddr[4], DestAddr[4];
uint16_t SourcePort, DestPort, Len;
uint8_t *data;

uint8_t PC_IPAddr[] = {192,168,1,10};
uint8_t PC_MACAddr[] = {0x00,0x23,0x7d,0x00,0x8a,0x08};
//uint8_t PC_MACAddr[] = {0xff,0xff,0xff,0xff,0xff,0xff};
uint8_t uC_IPAddr[] = {192,168,1,11};

void Init();
void SPID_INIT();

int main(void)
{
	ENC_Init();
	sei();
	
	uint8_t d[5] = {1,2,3,4,5};
    /* Replace with your application code */
    while (1) 
    {
    }
}

void Init()
{
	OSC.XOSCCTRL = OSC_FRQRANGE_12TO16_gc |                   // Select frequency range
	OSC_XOSCSEL_XTAL_16KCLK_gc;                // Select start-up time
	OSC.CTRL |= OSC_XOSCEN_bm;                                // Enable oscillator
	while ( ! (OSC.STATUS & OSC_XOSCRDY_bm) );                // Wait for oscillator is ready

	OSC.PLLCTRL = OSC_PLLSRC_XOSC_gc | (OSC_PLLFAC_gm & 2);   // Select PLL source and multipl. factor
	OSC.CTRL |= OSC_PLLEN_bm;                                 // Enable PLL
	while ( ! (OSC.STATUS & OSC_PLLRDY_bm) );                 // Wait for PLL is ready

	CCP = CCP_IOREG_gc;                                       // Security signature to modify clock
	CLK.CTRL = CLK_SCLKSEL_PLL_gc;                            // Select system clock source
	OSC.CTRL &= ~OSC_RC2MEN_bm;                               // Turn off 2MHz internal oscillator
	OSC.CTRL &= ~OSC_RC32MEN_bm;                              // Turn off 32MHz internal oscillator

	SPID_Init();
}

void SPID_Init()
{
	// configure SS, MOSI and SCK as output.
	PORTD.DIR = SPI_SS_bm | SPI_MOSI_bm | SPI_SCK_bm;
	SPI_CS_OFF;

	// SPI_D - Master mode 00, Clk_per / 4 (8MHz)
	SPID.CTRL= SPI_PRESCALER_DIV4_gc | SPI_ENABLE_bm | SPI_MASTER_bm | SPI_MODE_0_gc;
}

ISR(PORTC_INT0_vect)
{
	ENC_CLREIE();		// disable ENC interrupts (INT line goes inactive)

	// read packet
	if(ENC_RdUDPFrame(SourceAddr, DestAddr, &SourcePort, &DestPort, &Len, &data) == OK)
	{	// if it is correct UDP frame, send it back
		ENC_SendUDPFrame(uC_IPAddr, PC_IPAddr, PC_MACAddr, 11000, 11000, 0, Len, data);
		free(data);		// free allocated memory
	}
	
	ENC_SETEIE();		// enable ENC interrupts (if interrupt is pending INT line goes active again)
}



