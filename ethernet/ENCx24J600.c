// ********************************************************************************
// * ENCx24J600.c
// *
// * Created: 16.12.2016. 11:45:48
// *  Author: © Davor Antonic, 2017
// *  edited by Joris for the aTxmega265A3U
// ********************************************************************************
// * You have a royalty-free right to use, modify, reproduce and distribute
// * the ENCx24J600.c functions (and/or any modified version) in any way
// * you find useful, provided that you agree that Author has no warranty,
// * obligations or liability.
// * I will appreciate that you give me the credit if you find the code useful
// * and also send me information about bugs you find, derived products etc.
// *
// * Functions for interfacing with ENCx24J600 Ethernet controller are derived from
// * information provided in device data sheet (1).
// * You may find additional information at (2) AND (3)
// *
// * (1) ENC424J600/624J600 Data Sheet, Microchip, 2010
// * (2) MOJ RESEARCH GATE ÈLANAK
// * (3) moj web

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include "ENCx24J600.h"
#include "main.h"

static int16_t NextPacketPointer;	// pointer to the next packet in receive buffer


// Initialize ENCx24J600
// Assumes SPI interface on Port D, interrupt line connected to Pin 0. SPI should be initialized
// before calling ENC_Init (function SPIC_Init in SPI.c)
int8_t ENC_Init()
{
	// Wait for ENC SPI interface to initialize
	do
	{
		ENC_WCRU(EUDAST,0x1234);
	} while (ENC_RCRU(EUDAST) != 0x1234);
	
	// Wait for stable clock
	while (!(ENC_RCRU(ESTAT) & ENC_ESTAT_CLKRDY_bm));

	// Reset
	ENC_SETETHRST();
	_delay_us(50);

	// Check that EUDAST returned to default value
	if (ENC_RCRU(EUDAST) != 0x0000) return ERR;
	
	// wait at least 256 us for PHY initialization
	_delay_us(500);


	// Enable Ethernet, LED stretching, automatic MAC Address transmission, transmit and receive logic
	ENC_WCRU(ECON2,0xe000);

	// Initialize 'NextPacketPointer' to ERXST
	NextPacketPointer = ENC_RCRU(ERXST);

	// Disable reception of broadcast (ff-ff-ff-ff-ff-ff) frames - only frames having correct MAC address will be accepted
	ENC_BFCU(ERXFCON, ENC_ERXFCON_BCEN_bm);

	// Enable reception
	ENC_BFSU(ECON1, ENC_ECON1_RXEN_bm);

	// Interrupt control - PORT C, Pin 0
	PORTD.PIN0CTRL = PORT_OPC_PULLUP_gc | PORT_ISC_FALLING_gc;	// falling edge
	PORTD.INT0MASK = PIN0_bm;
	PORTD.INTCTRL = PORT_INT0LVL_MED_gc;						// medium priority

	PMIC.CTRL |= PMIC_MEDLVLEN_bm;								// enable medium level interrupts

	// Enable ENC interrupts
	ENC_SETEIE();

	return OK;
}


// ENCx24J600 System reset
void ENC_SETETHRST()
{
	uint8_t dummy;

	SPI_CS_ON;

	SPIC.DATA = 0xca;	// op code
	SPI_WAIT;
	dummy = SPIC.DATA;	// to clear Interrupt Flag

	SPI_CS_OFF;
}


// Enables ENCx24J600 interrupt system and packet received interrupt
void ENC_SETEIE()
{
	ENC_WCRU(EIE, 0x8040);		// set INTIE and PKTIE (packet received interrupt enable)
}

// Disables ENCx24J600 interrupt system
void ENC_CLREIE()
{
	uint8_t dummy;

	SPI_CS_ON;

	SPIC.DATA = 0xee;	// op code
	SPI_WAIT;
	dummy = SPIC.DATA;

	SPI_CS_OFF;
}

// Configure and start DMA checksum
void ENC_DMACKSUM()
{
	uint8_t dummy;

	SPI_CS_ON;

	SPIC.DATA = 0xd8;	// op code
	SPI_WAIT;
	dummy = SPIC.DATA;

	SPI_CS_OFF;
}


// Read Control Register Unbanked
// Fetches content of 16-bit ENCx24J600 register. Common registers are defined
// in ENCx24J600.h
uint16_t ENC_RCRU(uint8_t addr)
{
	uint8_t dummy, hi, lo;

	SPI_CS_ON;

	SPIC.DATA = 0x20;	// op code
	SPI_WAIT;

	SPIC.DATA = addr;	// register address
	SPI_WAIT;
	
	SPIC.DATA = DUMMY;
	SPI_WAIT;
	lo = SPIC.DATA;
	
	SPIC.DATA = DUMMY;
	SPI_WAIT;
	hi = SPIC.DATA;
	
	SPI_CS_OFF;
	
	return lo + (hi<<8);
}


// Write Control Register Unbanked
// Writes to 16-bit ENCx24J600 register. Common registers are defined
// in ENCx24J600.h
void ENC_WCRU(uint8_t addr, uint16_t data)
{
	uint8_t hi = (data>>8);
	uint8_t lo = data - (hi<<8);
	uint8_t dummy;

	SPI_CS_ON;

	SPIC.DATA = 0x22;	// op code
	SPI_WAIT;

	SPIC.DATA = addr;	// register address
	SPI_WAIT;

	SPIC.DATA = lo;
	SPI_WAIT;

	SPIC.DATA = hi;
	SPI_WAIT;
	dummy = SPIC.DATA;	// to clear Interrupt Flag

	SPI_CS_OFF;
}


// Bit Field Set, Unbanked
void ENC_BFSU(uint8_t addr, uint16_t mask)
{
	uint8_t hi = (mask>>8);
	uint8_t lo = mask - (hi<<8);
	uint8_t dummy;

	SPI_CS_ON;

	SPIC.DATA = 0x24;	// op code
	SPI_WAIT;

	SPIC.DATA = addr;	// register address
	SPI_WAIT;

	SPIC.DATA = lo;
	SPI_WAIT;

	SPIC.DATA = hi;
	SPI_WAIT;
	dummy = SPIC.DATA;	// to clear Interrupt Flag

	SPI_CS_OFF;
}


// Bit Field Clear, Unbanked
void ENC_BFCU(uint8_t addr, uint16_t mask)
{
	uint8_t hi = (mask>>8);
	uint8_t lo = mask - (hi<<8);
	uint8_t dummy;

	SPI_CS_ON;

	SPIC.DATA = 0x26;	// op code
	SPI_WAIT;

	SPIC.DATA = addr;	// register address
	SPI_WAIT;

	SPIC.DATA = lo;
	SPI_WAIT;

	SPIC.DATA = hi;
	SPI_WAIT;
	dummy = SPIC.DATA;	// to clear Interrupt Flag

	SPI_CS_OFF;
}


// Write General Purpose Buffer Write Pointer (EGPWRPT)
// Area in general purpose buffer is used to prepare packets for transmission.
void ENC_WGPWRPT(uint16_t BuffAddr)
{
	uint8_t hi = (BuffAddr>>8);
	uint8_t lo = BuffAddr - (hi<<8);
	uint8_t dummy;

	SPI_CS_ON;

	SPIC.DATA = 0x6c;	// op code
	SPI_WAIT;

	SPIC.DATA = lo;
	SPI_WAIT;

	SPIC.DATA = hi;
	SPI_WAIT;
	dummy = SPIC.DATA;	// to clear Interrupt Flag

	SPI_CS_OFF;
}

// Request Packet Transmission
void ENC_SETTXRTS()
{
	uint8_t dummy;

	SPI_CS_ON;

	SPIC.DATA = 0xd4;	// op code
	SPI_WAIT;
	dummy = SPIC.DATA;	// to clear Interrupt Flag

	SPI_CS_OFF;
}



// Construct and transmit UDP frame. Wait for completion of ongoing transmission before starting transmission.
// Care should be taken to change BuffAddr from previous datagram, otherwise datagram being transmitted
// could be corrupted.
// Parameters:
//			SourceIPAddr, DestIPAddr	- Source and destination IP addresses. Should be defined as uint8_t[4].
//			DestMACAddr					- Destination MAC address. Should be defined as uint8_t[6]. Broadcast address (ff-ff-ff-ff-ff-ff) could be used.
//										  ENC is initialized to automatically insert source MAC address into transmitting frame.
//			SourcePort, DestPort		- Source and destination ports. If source port is not used it should be 0.
//			BuffAddr					- Start address in general purpose buffer.
//			Len							- Length of Data field of UDP datagram. Total length of Ethernet frame to be transmitted is calculated inside function.
//			Data						- uint8_t array containing data. Maximum length is 1472 bytes (to satisfy max Ethernet frame payload limit of 1500 bytes).
//										  Minimum length is 0 bytes.
void ENC_SendUDPFrame(uint8_t *SourceIPAddr, uint8_t *DestIPAddr, uint8_t *DestMACAddr, uint16_t SourcePort, uint16_t DestPort, uint16_t BuffAddr, uint16_t Len, uint8_t *data)
{
	uint8_t dummy;
	uint8_t Header[UDP_HEADER_LEN];
	uint8_t UDPPseudoHeader[20];	// pseudo header for checksum calculation

	// set General Purpose Buffer Write Pointer (EGPWRPT) and start address (ETXST).
	ENC_WGPWRPT(BuffAddr);
	ENC_WCRU(ETXST, BuffAddr);

	// len => ETXLEN (total number of bytes in Tx buffer: Ethernet frame header (8) + IPv4 header (20) + UDP header (8) + UDP data (Len) )
	ENC_WCRU(ETXLEN, UDP_HEADER_LEN + Len);

	// Ethernet header
	// destination MAC
	uint8_t headIdx = 0;
	uint8_t psHeadIdx = 0;
	for (uint8_t i = 0; i < 6; i++)
	{
		Header[headIdx++] = DestMACAddr[i];
	}
	// Ethertype (0x0800)
	Header[headIdx++] = 0x08;
	Header[headIdx++] = 0x00;

	// IPv4 header
	// Version and IHL
	Header[headIdx++] = 0x45;	// version 4, header length 5 long words
	// DSCP and ECN
	Header[headIdx++] = 0x00;
	// Total length: IPv4 header (20) + UDP header (8) + UDP data (Len)
	uint16_t totalLen = 28 + Len;
	Header[headIdx++] = totalLen>>8;		// hi byte
	Header[headIdx++] = totalLen & 0xff;	// lo byte
	// ID, flags and fragment offset
	for (uint8_t i = 0; i < 4; i++)
	{
		Header[headIdx++] = 0x00;
	}
	// Time to live
	Header[headIdx++] = 0x80;
	// Protocol
	Header[headIdx++] = 0x11;	// UDP
	// Header checksum
	Header[headIdx++] = 0x00;
	Header[headIdx++] = 0x00;
	// Source IP
	for (uint8_t i = 0; i < 4; i++)
	{
		Header[headIdx++] = SourceIPAddr[i];
		UDPPseudoHeader[psHeadIdx++] = SourceIPAddr[i];
	}
	// Destination IP
	for (uint8_t i = 0; i < 4; i++)
	{
		Header[headIdx++] = DestIPAddr[i];
		UDPPseudoHeader[psHeadIdx++] = DestIPAddr[i];
	}

	// Calculate IPv4 header checksum
	GenerateIPv4HeaderChecksum(Header + 8);		// skip Ethernet header

	// UDP header
	Header[headIdx++] = SourcePort>>8;
	Header[headIdx++] = SourcePort & 0xff;
	Header[headIdx++] = DestPort>>8;
	Header[headIdx++] = DestPort & 0xff;
	Header[headIdx++] = (Len+8)>>8;
	Header[headIdx++] = (Len+8) & 0xff;
	uint16_t chksumAddr = BuffAddr + headIdx;	// address of checksum field in buffer
	Header[headIdx++] = 0x00;		// UDP checksum placeholder
	Header[headIdx++] = 0x00;

	// rest of pseudoheader
	UDPPseudoHeader[psHeadIdx++] = 0x00;
	UDPPseudoHeader[psHeadIdx++] = 0x11;
	UDPPseudoHeader[psHeadIdx++] = (Len+8)>>8;
	UDPPseudoHeader[psHeadIdx++] = (Len+8) & 0xff;
	UDPPseudoHeader[psHeadIdx++] = SourcePort>>8;
	UDPPseudoHeader[psHeadIdx++] = SourcePort & 0xff;
	UDPPseudoHeader[psHeadIdx++] = DestPort>>8;
	UDPPseudoHeader[psHeadIdx++] = DestPort & 0xff;
	UDPPseudoHeader[psHeadIdx++] = (Len+8)>>8;
	UDPPseudoHeader[psHeadIdx++] = (Len+8) & 0xff;
	UDPPseudoHeader[psHeadIdx++] = 0x00;	// checksum placeholder
	UDPPseudoHeader[psHeadIdx++] = 0x00;

	
	// write data to buffer (send op code followed by n data bytes (CS asserted)
	SPI_CS_ON;
	SPIC.DATA = WGPDATA;
	SPI_WAIT;
	// Header
	for (uint8_t i = 0; i < UDP_HEADER_LEN; i++)
	{
		SPIC.DATA = Header[i];
		SPI_WAIT;
	}

	// Data
	for (uint16_t i = 0; i < Len; i++)
	{
		SPIC.DATA = data[i];
		SPI_WAIT;
	}
	dummy = SPIC.DATA;	// to clear Interrupt Flag
	SPI_CS_OFF;

	// generate and write checksum to transmit buffer
	int16_t checksum = GenerateUDPChecksum(UDPPseudoHeader, 20, BuffAddr + UDP_HEADER_LEN, Len);
	ENC_WGPWRPT(chksumAddr);
	SPI_CS_ON;
	SPIC.DATA = WGPDATA;
	SPI_WAIT;
	SPIC.DATA = checksum>>8;
	SPI_WAIT;
	SPIC.DATA = checksum & 0xff;
	SPI_WAIT;
	dummy = SPIC.DATA;	// to clear Interrupt Flag
	SPI_CS_OFF;


	// wait for completion of ongoing transmission
	while (ENC_RCRU(ECON1) & ENC_ECON1_TXRTS_bm);

	ENC_SETTXRTS();		// start transmission

}


// Resend the last sent UDP datagram
void ENC_ReSendUDPFrame()
{
	// wait for completion of ongoing transmission
	while (ENC_RCRU(ECON1) & ENC_ECON1_TXRTS_bm);

	ENC_SETTXRTS();		// start transmission
}


// Read UDP frame from read buffer. Returns number of data bytes read or -1 if frame is not an UDP frame. Allocates
// memory required to store the data part of UDP frame. Checksum is ignored.
// Prior to calling this function it is necessary to check that frame is available, either by polling the PKTCNT
// bits (ESTAT<7:0>) for a non-zero value, or putting ENC_RdUDPFrame in ISR(PORTC_INT0_vect) interrupt routine.
// Function updates 'NextPacketPointer'.
// Parameters for returning values:
//		SourceAddr	- source IP address (4 bytes array)
//		DestAddr	- destination IP address (IP address allocates to ENC)
//		SourcePort	- source port (0 if not used)
//		DestPort	- destination port
//		Data		- received data
// Since 'data' array is dynamically allocated IT IS NECESSARY TO FREE IT WHEN NO LONGER NEEDED.
int8_t ENC_RdUDPFrame(uint8_t *SourceAddr, uint8_t *DestAddr, uint16_t *SourcePort, uint16_t *DestPort, uint16_t *Len, uint8_t **Data)
{
	uint8_t dummy;
	uint8_t lo, hi;
	int8_t errorCode = 0;

	// wait for packet reception (PacKeTCouNT > 0) => check interrupt at PC.0
	//while (!(ENC_RCRU(ESTAT) & ENC_ESTAT_PKTCNT_bm));

	// NextPacketPointer => ERXRDPT (receive buffer read pointer)
	ENC_WCRU(ERXRDPT, NextPacketPointer);

	SPI_CS_ON;
	SPIC.DATA = RRXDATA;	// command for sequential reading from receive buffer
	SPI_WAIT;
	// read address of the next packet and write to NextPacketPointer
	// lo byte
	SPIC.DATA = DUMMY;
	SPI_WAIT;
	lo = SPIC.DATA;
	// hi byte
	SPIC.DATA = DUMMY;
	SPI_WAIT;
	hi = SPIC.DATA;
	NextPacketPointer = lo + ((uint16_t)hi<<8);

	// read Receive Status Vector (6 bytes)
	uint8_t RSV[6];
	for(uint8_t i = 0; i < 6; i++)
	{
		SPIC.DATA = DUMMY;
		SPI_WAIT;
		RSV[i] = SPIC.DATA;
	}
	// int16_t count = ((uint16_t)(RSV[0])<<8) + RSV[1];	// total packet size

	// Ethernet header
	// discard destination and source MAC addresses
	for (uint8_t i = 0; i < 12; i++)
	{
		SPIC.DATA = DUMMY;
		SPI_WAIT;
		dummy = SPIC.DATA;
	}
	// Ethertype
	SPIC.DATA = DUMMY;
	SPI_WAIT;
	hi = SPIC.DATA;
	SPIC.DATA = DUMMY;
	SPI_WAIT;
	lo = SPIC.DATA;
	if(hi==0x08 && lo==0)	// IPv4 frame ?
	{
		// Read IPv4 header
		uint8_t IPv4Header[20];		// 20 bytes header - options, if present, will be ignored
		for(uint8_t i = 0; i < 20; i++)
		{
			SPIC.DATA = DUMMY;
			SPI_WAIT;
			IPv4Header[i] = SPIC.DATA;
		}

		uint8_t hlen = 4 * (IPv4Header[0] & 0x0f);	// header length in bytes
		uint8_t version = (IPv4Header[0]>>4) & 0x0f;
		// skip options, if exist
		for(uint8_t i = 20; i < hlen; i++)
		{
			SPIC.DATA = DUMMY;
			SPI_WAIT;
			dummy = SPIC.DATA;
		}

		if (version == 4)	// IPv4
		{
			// total length
			uint16_t totalLen = ((uint16_t)(IPv4Header[2])<<8) + IPv4Header[3];

			// Check higher level protocol
			if (IPv4Header[9] == PROTOCOL_UDP)
			{
				// header checksum - IPv4Header[10..11] (CALCULATE through ENC DMA)
				// source IP
				SourceAddr[0] = IPv4Header[12];
				SourceAddr[1] = IPv4Header[13];
				SourceAddr[2] = IPv4Header[14];
				SourceAddr[3] = IPv4Header[15];

				// destination IP
				DestAddr[0] = IPv4Header[16];
				DestAddr[1] = IPv4Header[17];
				DestAddr[2] = IPv4Header[18];
				DestAddr[3] = IPv4Header[19];

				uint8_t UDPHeader[8];
				for(uint8_t i = 0; i < 8; i++)
				{
					SPIC.DATA = DUMMY;
					SPI_WAIT;
					UDPHeader[i] = SPIC.DATA;
				}
				*SourcePort = ((uint16_t)(UDPHeader[0])<<8) + UDPHeader[1];
				*DestPort = ((uint16_t)(UDPHeader[2])<<8) + UDPHeader[3];
				*Len = ((uint16_t)(UDPHeader[4])<<8) + UDPHeader[5] - 8;
				// Checksum (ignored)

				// Data - check size of receive buffer
				*Data = malloc(*Len);
				// receive data
				for(uint16_t i = 0; i < *Len; i++)
				{
					SPIC.DATA = DUMMY;
					SPI_WAIT;
					Data[i] = SPIC.DATA;
				}
			}
			else errorCode = ENC_ERR_NOUDP;

		}
		else errorCode = ENC_ERR_NOIPv4;

	}
	else errorCode = ENC_ERR_NOIPv4;


	SPI_CS_OFF;		// terminate command for sequential reading from receive buffer

	//update RXTAIL pointer
	int16_t newTail;
	if (NextPacketPointer == 0) newTail = 0x5ffe;
	else newTail = NextPacketPointer - 2;
	ENC_WCRU(ERXTAIL, newTail);

	// Decrement PKTCNT by asserting ECON1.PKTDEC
	//ENC_WCRU(ECON1, ENC_RCRU(ECON1) | ENC_ECON1_PKTDEC_bm);
	ENC_BFSU(ECON1, ENC_ECON1_PKTDEC_bm);

	return errorCode;
}



// Calculate IPv4 Header checksum and update header
// Used to send proper IPv4 packet
void GenerateIPv4HeaderChecksum(uint8_t *Header)
{
	// clear checksum
	Header[10] = 0;
	Header[11] = 0;
	uint32_t sum = 0;
	uint8_t hlen = 4 * (Header[0] & 0x0f);	// header length in bytes
	// sum
	for(uint8_t i = 0; i < hlen; i+=2)
	{
		sum += ((uint16_t)(Header[i])<<8) + Header[i+1];
	}
	// add carry
	uint8_t carry;
	while (((carry = (sum & 0xffff0000) >> 16)) != 0)
	{
		sum &= 0x0000ffff;
		sum += carry;
	}
	uint16_t checksum = ~sum;
	Header[10] = checksum >> 8;			// hi byte
	Header[11] = checksum & 0x00ff;		// lo byte
}


// Calculate UDP checksum
// UDP pseudoheader checksum is calculated inside function and data checksum is calculated using ENCx24J600
// DMA controller. Algorithm is described in (1) and (2)
// Parameters:
//			Header			- UDP pseudoheader
//			HLen			- pseudoheader length
//			DataStartAddr	- start address of data in general purpose buffer
//			DLen			- data length in bytes
uint16_t GenerateUDPChecksum(uint8_t *Header, uint16_t HLen, uint16_t DataStartAddr, uint16_t DLen)
{
	volatile uint32_t sum = 0;
	uint8_t carry;
	volatile uint16_t headSum, dataSum, checksum;

	if (DLen > 0)	// if data field is empty skip calculation od data checksum
	{
		// Initialize DMA calculation of data checksum:
		// Set EDMAST to the start address
		ENC_WCRU(EDMAST, DataStartAddr);
		// Set EDMALEN to the length of the input data
		ENC_WCRU(EDMALEN, DLen);
		// Clear DMACPY (ECON1<4>) to prevent a copy operation.
		// Clear DMANOCS (ECON1<2>) to select a	checksum calculation.
		// Clear DMACSSD (ECON1<3>) to use the default seed of 0000h.
		// Set DMAST to initiate the operation
		ENC_DMACKSUM();
	}

	// Calculate Header checksum. ENC simultaneously calculates data checksum.
	for(uint8_t i = 0; i < HLen; i+=2)
	{
		sum += ((uint16_t)(Header[i])<<8) + Header[i+1];
	}
	// add carry
	while (((carry = (sum & 0xffff0000) >> 16)) != 0)
	{
		sum &= 0x0000ffff;
		sum += carry;
	}
	headSum = sum;

	if (DLen > 0)
	{
		// Wait for ENC DMA to finish data checksum calculation
		while (ENC_RCRU(ECON1) & ENC_ECON1_DMAST_bm);

		// Read data checksum
		dataSum = ENC_RCRU(EDMACS);	// LO and HI byte swapped !?
		uint8_t lo = dataSum & 0x00ff;
		uint8_t hi = dataSum >> 8;
		dataSum = ~((lo<<8) + hi);
	}
	else dataSum = 0;

	// Combine header and data checksum
	sum = (uint32_t)headSum + dataSum;		// 32 bit sum, otherwise carry will be discarded
	// add carry
	while (((carry = (sum & 0xffff0000) >> 16)) != 0)
	{
		sum &= 0x0000ffff;
		sum += carry;
	}
	sum = ~sum;
	checksum = sum != 0 ? sum : 0xffff;		// positive zero should be converted to negative zero

	return checksum;
}