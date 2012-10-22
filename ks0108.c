#include <inttypes.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "ks0108.h"

struct displayPos pos;

void ks0108Enable(void) {
	volatile uint8_t i;
	
	LCD_CMD_PORT |= 0x01 << EN;						// EN high level width: min. 450ns
	asm volatile("nop\n\t"
				 "nop\n\t"
				 "nop\n\t"
				 ::);
	LCD_CMD_PORT &= ~(0x01 << EN);
	for(i=0; i<8; i++);								// a little delay loop (faster than reading the busy flag)
}

void ks0108Fill(uint8_t mode) {
	uint8_t i, j;
	
	for(i=0; i<8; i++) {							// page address
		ks0108GotoXY(0, i*8);
		for(j=0; j<128; j++)						// x address
			ks0108WriteData(mode);					// write data
	}
}

void ks0108GotoXY(uint8_t x, uint8_t y) {
	uint8_t chip = CHIP1, cmd;
	
	if(x > 127) x = 0;								// ensure that coordinates are legal
	if(y > 63)  y = 0;
	
	pos.x = x;										// save new coordinates
	pos.y = y;
	pos.page = y/8;
	
	if(x >= 64) {									// select the right chip
		x -= 64;
		chip = CHIP2;
	}
	cmd = LCD_SET_ADD | x;
	ks0108WriteCommand(cmd, chip);					// set x address on active chip
	
	cmd = LCD_SET_PAGE | pos.page;					// set y address on both chips
	ks0108WriteCommand(cmd, CHIP1);
	ks0108WriteCommand(cmd, CHIP2);
}

void ks0108Init(void) {
	pos.x = 0;
	pos.y = 0;
	pos.page = 0;
	
	LCD_CMD_DIR |= ((1<<D_I)|(1<<R_W)|(1<<EN)|(1<<CSEL1)|(1<<CSEL2))	;								// command port is output
	ks0108WriteCommand(LCD_ON, CHIP1);				// power on
	ks0108WriteCommand(LCD_ON, CHIP2);
	
	ks0108WriteCommand(LCD_DISP_START, CHIP1);		// display start line = 0
	ks0108WriteCommand(LCD_DISP_START, CHIP2);
	ks0108Fill(CLEAR);								// display clear
	ks0108GotoXY(0,0);
}

void ks0108SetDot(uint8_t x, uint8_t y) {
	uint8_t data;
	
	ks0108GotoXY(x, y);								// read data from display memory
	data = ks0108ReadData(NO_INCREMENT_X);			// dummy read
	data = ks0108ReadData(NO_INCREMENT_X);			// "real" read
	
	data |= 0x01 << (y % 8);						// set dot
	
	ks0108WriteData(data);							// write data back to display
}

void ks0108ClearDot(uint8_t x, uint8_t y) {
	uint8_t data;
	
	ks0108GotoXY(x, y);								// read data from display memory
	data = ks0108ReadData(NO_INCREMENT_X);			// dummy read
	data = ks0108ReadData(NO_INCREMENT_X);			// "real" read
	
	data &= ~(0x01 << (y % 8));						// clear dot
	
	ks0108WriteData(data);							// write data back to display
}

void ks0108PutChar(char c, struct font font) {
	uint16_t index;
	uint8_t pages, tmp, i, j, xPos, yPos;
	
	if(c == '\n')
		ks0108NewLine(font.height, 0);
	if(c < 32)										// ignore escape characters
		return;
	
	xPos = pos.x;									// save old coordinates
	yPos = pos.y;
	
	c -= 32;
	pages = font.height/8;							// calculate pages
	if(font.height%8 != 0)
		pages++;
	
	index = c*font.width*pages;						// get the needed array index

	for(j=0; j<pages; j++) {
		for(i=j; i<font.width*pages; i+=pages) {
			tmp = pgm_read_byte(font.charData+index+i);
			ks0108WriteData(tmp);					// write Character-Data
		}
		ks0108GotoXY(xPos, pos.y+8);
	}
	ks0108GotoXY(pos.x+font.width, yPos);			// go to the upper right corner
}

void ks0108NewLine(uint8_t fontHeight, uint8_t offset) {
	if(pos.y+fontHeight < 64)
		ks0108GotoXY(offset, pos.y+fontHeight);
	else
		ks0108GotoXY(offset, 0);
}

void ks0108PutString(char *string, struct font font) {
	uint8_t startx=pos.x, i=0;
	char c = string[0];
	
	while(c != 0) {
		if(c == '\n')
			ks0108NewLine(font.height, startx);
		else
			ks0108PutChar(c, font);
		c = string[++i];
	}
}

void ks0108PutStringP(PGM_P string, struct font font) {
	uint8_t startx=pos.x;
	char c = pgm_read_byte(string);
	
	while(c != 0) {
		if(c == '\n')
			ks0108NewLine(font.height, startx);
		else
			ks0108PutChar(c, font);
		
		c = pgm_read_byte(++string);
	}
}

char ks0108ReadData(uint8_t incXAdd) {
	uint8_t data;
	volatile uint8_t i;
	
	LCD_DATA_OUT = 0x00;
	LCD_DATA_DIR = 0x00;							// data port is input
	
	if(pos.x < 64) {
		LCD_CMD_PORT &= ~(0x01 << CSEL2);			// deselect chip 2
		LCD_CMD_PORT |= 0x01 << CSEL1;				// select chip 1
	} else if(pos.x >= 64) {
		LCD_CMD_PORT &= ~(0x01 << CSEL1);			// deselect chip 1
		LCD_CMD_PORT |= 0x01 << CSEL2;				// select chip 2
	}
	
	LCD_CMD_PORT |= 0x01 << D_I;					// D/I = 1
	LCD_CMD_PORT |= 0x01 << R_W;					// R/W = 1
	
	
	LCD_CMD_PORT |= 0x01 << EN;						// EN high level width: min. 450ns
	asm volatile("nop\n\t"
				 "nop\n\t"
				 "nop\n\t"
				 ::);
	
	data = LCD_DATA_IN;								// read Data			 
	
	LCD_CMD_PORT &= ~(0x01 << EN);
	for(i=0; i<8; i++);								// a little delay loop (faster than reading the busy flag)
	
	LCD_DATA_DIR = 0xFF;
	
	
	if(incXAdd == INCREMENT_X)						// display x-address is incremented automatically
		pos.x++;
	else
		ks0108GotoXY(pos.x, pos.y);
	
	return data;
}

void ks0108WriteCommand(uint8_t cmd, uint8_t chip) {
	if(chip == CHIP1) {
		LCD_CMD_PORT &= ~(0x01 << CSEL2);			// deselect chip 2
		LCD_CMD_PORT |= 0x01 << CSEL1;				// select chip 1
	} else if(chip == CHIP2) {
		LCD_CMD_PORT &= ~(0x01 << CSEL1);			// deselect chip 1
		LCD_CMD_PORT |= 0x01 << CSEL2;				// select chip 2
	}
	
	LCD_CMD_PORT &= ~(0x01 << D_I);					// D/I = 0
	LCD_CMD_PORT &= ~(0x01 << R_W);					// R/W = 0
	LCD_DATA_DIR = 0xFF;							// data port is output
	LCD_DATA_OUT = cmd;								// write command
	ks0108Enable();									// enable
	 
	LCD_DATA_OUT = 0x00;
}

void ks0108WriteData(uint8_t data) {
	uint8_t displayData, yOffset, cmdPort;

#ifdef DEBUG
	volatile uint16_t i;
	for(i=0; i<5000; i++);
#endif

	if(pos.x >= 128)
		return;

	if(pos.x < 64) {
		LCD_CMD_PORT &= ~(0x01 << CSEL2);			// deselect chip 2
		LCD_CMD_PORT |= 0x01 << CSEL1;				// select chip 1
	} else if(pos.x >= 64) {
		LCD_CMD_PORT &= ~(0x01 << CSEL1);			// deselect chip 1
		LCD_CMD_PORT |= 0x01 << CSEL2;				// select chip 2
	}
	if(pos.x == 64)									// chip2 X-address = 0
		ks0108WriteCommand(LCD_SET_ADD, CHIP2);
	
	LCD_CMD_PORT |= 0x01 << D_I;					// D/I = 1
	LCD_CMD_PORT &= ~(0x01 << R_W);					// R/W = 0
	LCD_DATA_DIR = 0xFF;							// data port is output
	
	
	yOffset = pos.y%8;
	if(yOffset != 0) {
		// first page
		cmdPort = LCD_CMD_PORT;						// save command port
		displayData = ks0108ReadData(NO_INCREMENT_X);	// dummy read
		displayData = ks0108ReadData(NO_INCREMENT_X);	// "real" read
		
		LCD_CMD_PORT = cmdPort;						// restore command port
		LCD_DATA_DIR = 0xFF;						// data port is output
		
		displayData |= data << yOffset;
		LCD_DATA_OUT = displayData;					// write data
		ks0108Enable();								// enable
		
		// second page
		ks0108GotoXY(pos.x, pos.y+8);
		
		displayData = ks0108ReadData(NO_INCREMENT_X);	// dummy read
		displayData = ks0108ReadData(NO_INCREMENT_X);	// "real" read
		
		LCD_CMD_PORT = cmdPort;						// restore command port
		LCD_DATA_DIR = 0xFF;						// data port is output
		
		displayData |= data >> (8-yOffset);
		LCD_DATA_OUT = displayData;					// write data
		ks0108Enable();								// enable
		
		ks0108GotoXY(pos.x+1, pos.y-8);
	} else {
		LCD_DATA_OUT = data;						// write data
		ks0108Enable();								// enable
		pos.x++;
	}
	LCD_DATA_OUT = 0x00;
}


void ks0108ClearLine(uint8_t line) // 0 - 7
{
	ks0108GotoXY(0,line*8);
	for(int i=0;i<128;i++)
		ks0108WriteData(0);
}	
