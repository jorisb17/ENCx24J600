#ifndef ENCX24J600_H_
#define ENCX24J600_H_

// masks
#define ENC_ESTAT_CLKRDY_bm		0x1000

#define ENC_ECON1_RXEN_bm		0x0001
#define ENC_ECON1_TXRTS_bm		0x0002

#define ENC_ECON1_DMANOSC_bm	0x0004
#define ENC_ECON1_DMACSSD_bm	0x0008
#define ENC_ECON1_DMACPY_bm		0x0010
#define ENC_ECON1_DMAST_bm		0x0020

#define ENC_ECON1_PKTDEC_bm		0x0100

#define ENC_ERXFCON_BCEN_bm		0x0001

#define ENC_ESTAT_PKTCNT_bm		0x00ff

// ENCx24J600 SFR's addresses
#define ERXST				0x04		// default 0x5340
#define ERXTAIL				0x06		// default 0x5fee

#define EDMAST				0x0a		// DMA start address
#define EDMALEN				0x0c		// DMA length
#define EDMACS				0x10		// checksum

#define EUDAST				0x16		// user-defined area start address
#define ERXFCON				0x34		// received filters control register

#define MAAADR3				0x60		// MAC address
#define MAAADR2				0x62
#define MAAADR1				0x64

#define ESTAT				0x1a		// Ethernet status register

#define ECON1				0x1e		// Ethernet control register(s)
#define ECON2				0x6e

#define EIE					0x72		// Ethernet interrupt enable register

#define EGPWRPT				0x88		// General purpose buffer write pointer
#define ERXRDPT				0x8a		// Receive buffer read pointer

#define ETXST				0x00		// Transmit data start pointer
#define ETXLEN				0x02		// Transmit buffer length pointer

#define WGPDATA				0x2a		// OP-code for write to general purpose buffer data register 
#define RRXDATA				0x2c		// OP-code for read from receive buffer data register 



#define DUMMY				0x00		// dummy write value for SPI read cycles

#define OK					0
#define ERR					-1			// general error
#define ENC_ERR_NOIPv4		-2			// received frame doesn't contain IPv4 packet
#define ENC_ERR_NOUDP		-3			// received frame doesn't contain UDP datagram
#define ENC_ERR_LONG_MSG	-4			// received data longer than allocated space

#define PROTOCOL_UDP		0x11

// ENCx24J600 SPI instructions
int8_t ENC_Init(void);
void ENC_SETETHRST(void);				// Reset
uint16_t ENC_RCRU(uint8_t);				// Read Control Register, Unbanked
void ENC_WCRU(uint8_t, uint16_t);		// Write Control Register, Unbanked
void ENC_BFSU(uint8_t, uint16_t);		// Bit Field Set, Unbanked
void ENC_BFCU(uint8_t, uint16_t);		// Bit Field Clear, Unbanked
void ENC_WGPWRPT(uint16_t);				// Write General Purpose Buffer Pointer
void ENC_SETTXRTS(void);				// Transmint packet
void ENC_CLREIE(void);					// disable interrupts
void ENC_SETEIE(void);					// (re)enable interrupts
void ENC_DMACKSUM(void);				// configure and start DMA checksum	


void ENC_SendUDPFrame(uint8_t*, uint8_t*, uint8_t*, uint16_t, uint16_t, uint16_t, uint16_t, uint8_t*);
void ENC_ReSendUDPFrame();
int8_t ENC_RdUDPFrame(uint8_t*, uint8_t*, uint16_t*, uint16_t*, uint16_t*, uint8_t**);
void GenerateIPv4HeaderChecksum(uint8_t*);
uint16_t GenerateUDPChecksum(uint8_t*, uint16_t, uint16_t, uint16_t);


#define UDP_HEADER_LEN			36		// length of transmitted UDP header

#define RCV_DATA_LEN			512		// maximum length of received buffer



#endif /* ENCX24J600_H_ */