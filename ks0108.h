#include <inttypes.h>
#include <avr/pgmspace.h>

#ifndef	KS0108_H
#define KS0108_H

// Ports
#define LCD_CMD_PORT		PORTC		// Command Output Register   OK
#define LCD_CMD_DIR			DDRC		// Data Direction Register for Command Port

#define LCD_DATA_IN			PINA		// Data Input Register   PA7 pin44
#define LCD_DATA_OUT		PORTA		// Data Output Register
#define LCD_DATA_DIR		DDRA		// Data Direction Register for Data Port

// Function Paramters
#define INCREMENT_X			0
#define NO_INCREMENT_X		1

// Command Port Bits
#define D_I					0x00		// D/I Bit Number
#define R_W					0x01		// R/W Bit Number
#define EN					0x02		// EN Bit Number
#define CSEL1				0x03		// CS1 Bit Number
#define CSEL2				0x04		// CS2 Bit Number

// Chips
#define CHIP1				0x00
#define CHIP2				0x01

// Commands
#define LCD_ON				0x3F
#define LCD_OFF				0x3E
#define LCD_SET_ADD			0x40
#define LCD_SET_PAGE		0xB8
#define LCD_DISP_START		0xC0

// Fill Modes
#define BLACK				0xFF
#define CLEAR				0x00

// Uncomment for slow drawing
// #define DEBUG

struct displayPos {
	uint8_t x;
	uint8_t y;
	uint8_t page;
};

struct font {
	uint8_t width;
	uint8_t height;
	PGM_P charData;
};

// Function Prototypes
void ks0108Enable(void);
void ks0108Fill(uint8_t mode);
void ks0108GotoXY(uint8_t, uint8_t);
void ks0108Init(void);
void ks0108SetDot(uint8_t, uint8_t);
void ks0108ClearDot(uint8_t, uint8_t);
void ks0108PutChar(char c, struct font font);
void ks0108NewLine(uint8_t fontHeight, uint8_t offset);
void ks0108PutString(char *string, struct font font);
void ks0108PutStringP(PGM_P string, struct font font);
char ks0108ReadData(uint8_t incXAdd);
void ks0108WriteCommand(uint8_t cmd, uint8_t chip);
void ks0108WriteData(uint8_t data);

/************************************************************************/
/* Stefan Test															*/

void ks0108ClearLine(uint8_t line); // 1 - 8

/************************************************************************/

//UART

//void USART_vInit(void);
//void USART_vSendByte(uint8_t u8Data);
//uint8_t USART_vReceiveByte();

// Tim
void receiveImage();
void drawImage(int xStart, int yStart, int width, int heigth, uint8_t *data);
void pgm_drawImage(int xStart, int yStart, int width, int height, uint8_t *data);
void triggerLeds(uint8_t leds);
void alive();

#endif
