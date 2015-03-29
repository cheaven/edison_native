
// Arduino hooks
#include <Arduino.h>
#include <trace.h>
#include <interrupt.h>
#include <sys/stat.h>

#include "oled12864.h"


static void convert8BitToOLEDFB_12864(const unsigned char * rawBit, unsigned char * destBit, size_t pixelCount)
{

    for (int fbpos = 0; fbpos < pixelCount; ++fbpos) {
        unsigned char current8bit = 0;
#if 1  
        int colPos = fbpos & (128 - 1);
        int rowPos = (fbpos >> 7) << 3;
#else
        int colPos = fbpos % 128;
        int rowPos = (fbpos / 128) * 8;
#endif          
        current8bit |= rawBit[colPos + ((rowPos + 0)<<7) ]?(0x1<<0):0;
        current8bit |= rawBit[colPos + ((rowPos + 1)<<7) ]?(0x1<<1):0;
        current8bit |= rawBit[colPos + ((rowPos + 2)<<7) ]?(0x1<<2):0;
        current8bit |= rawBit[colPos + ((rowPos + 3)<<7) ]?(0x1<<3):0;
        current8bit |= rawBit[colPos + ((rowPos + 4)<<7) ]?(0x1<<4):0;
        current8bit |= rawBit[colPos + ((rowPos + 5)<<7) ]?(0x1<<5):0;
        current8bit |= rawBit[colPos + ((rowPos + 6)<<7) ]?(0x1<<6):0;
        current8bit |= rawBit[colPos + ((rowPos + 7)<<7) ]?(0x1<<7):0;
        
        destBit[fbpos] = current8bit;
    }
}


OLED12864::OLED12864()
{

}

OLED12864::~OLED12864()
{

}

void OLED12864::init()
{
    // set pin mode...

    pinMode(OLED_SPI_CS, OUTPUT);   
    pinMode(OLED_SPI_CLK, OUTPUT);  
    pinMode(OLED_SPI_MOSI, OUTPUT);     
    digitalWrite(OLED_SPI_CLK, 1);
    digitalWrite(OLED_SPI_MOSI, 1);
    digitalWrite(OLED_SPI_CS, 1);
    digitalWrite(OLED_SPI_CS, 0);

    pinMode(OLED_CMD, OUTPUT);    
    digitalWrite(OLED_CMD, 1);

    delay(1);


	digitalWrite(OLED_SPI_CLK,1);
	
    //OLED_RST_Clr();
	
    delay(500);
	
    //OLED_RST_Set();
	
	txCmd(0xAE);//--turn off oled panel
	
	txCmd(0x02);//---set low column address
	txCmd(0x10);//---set high column address
	
	txCmd(0x40);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	
	txCmd(0xB0);//--set contrast control register
	
	txCmd(0x81); // Set SEG Output Current Brightness
	txCmd(0xFF);//--Set SEG/Column Mapping     0xa0???? 0xa1??
	
	txCmd(0xA1);//Set COM/Row Scan Direction   0xc0???? 0xc8??
	
	txCmd(0xA6);//--set normal display
	
	txCmd(0xA8);//--set multiplex ratio(1 to 64)
	txCmd(0x3F);//--1/64 duty
	
	txCmd(0xAD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	txCmd(0x8B);//-not offset
	
	txCmd(0x33);//--set display clock divide ratio/oscillator frequency
	
	txCmd(0xC8);//--set divide ratio, Set Clock as 100 Frames/Sec
	
	txCmd(0xD3);//--set pre-charge period
	txCmd(0x00);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	
	txCmd(0xD5);//--set com pins hardware configuration
	txCmd(0x80);
	
	txCmd(0xD9);//--set vcomh
	txCmd(0x1F);//Set VCOM Deselect Level
	
	txCmd(0xDA);//-Set Page Addressing Mode (0x00/0x01/0x02)
	txCmd(0x12);//
	
	txCmd(0xDB);//--set Charge Pump enable/disable
	txCmd(0x40);//--set(0x10) disable
    
	txCmd(0xAF);//--turn on oled panel
	
	clear();
	setPos(0,0);
}

    
void OLED12864::clear()
{
    fillPattern(0);
}

void OLED12864::fillData8bit(const void * data)
{
    convert8BitToOLEDFB_12864((const unsigned char *)data, renderBuffer, SCREEN_PIXEL_WIDTH*SCREEN_PIXEL_HEIGHT/8);
    fillData(renderBuffer);
}

void OLED12864::fillData(const void * data)
{
    unsigned int j=0;
    unsigned char x,y;
    const unsigned char * byte = (const unsigned char *)data;
  
    for(y=0;y<SCREEN_PIXEL_HEIGHT/8;y++)
    {
        setPos(0,y);
        for(x=0;x<SCREEN_PIXEL_WIDTH;x++)
        {
            txData(byte[j++]);
        }
    }

}


void OLED12864::fillPattern(unsigned char pattern)
{
	unsigned char y,x;
	for(y=0;y<SCREEN_PIXEL_HEIGHT/8;y++)
	{
		txCmd(0xb0 | y);
		txCmd(0x01);
		txCmd(0x10);
		for(x=0;x<SCREEN_PIXEL_WIDTH;x++)
		{
			txData(pattern);
		}
	}
}



void OLED12864::setPos(int x, int y)
{
        x+=2;
	txCmd(0xb0 + y);
	txCmd(((x&0xf0)>>4)| 0x10);
	txCmd((x&0x0f));
}

void OLED12864::txCmd(unsigned char cmd)
{
    unsigned char i;
    digitalWrite(OLED_SPI_CS, 0);
    digitalWrite(OLED_CMD, 0);

    for(i=0;i<8;i++)
    {
        if((cmd << i) & 0x80)
        {
            digitalWrite(OLED_SPI_MOSI,1);
        } else {
            digitalWrite(OLED_SPI_MOSI,0);
        }

        digitalWrite(OLED_SPI_CLK,0);
        digitalWrite(OLED_SPI_CLK,1);
    }
    digitalWrite(OLED_SPI_CS, 1);

}

void OLED12864::txData(unsigned char cmd)
{
    unsigned char i;
    digitalWrite(OLED_SPI_CS, 0);
    digitalWrite(OLED_CMD, 1);

    for(i=0;i<8;i++)
    {
        if((cmd << i) & 0x80)
        {
            digitalWrite(OLED_SPI_MOSI,1);
        } else {
            digitalWrite(OLED_SPI_MOSI,0);
        }

        digitalWrite(OLED_SPI_CLK,0);
        digitalWrite(OLED_SPI_CLK,1);
    }
    digitalWrite(OLED_SPI_CS, 1);

}
