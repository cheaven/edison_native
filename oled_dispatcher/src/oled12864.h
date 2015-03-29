#pragma once

#define OLED_SPI_CLK  13
#define OLED_SPI_CS   9
#define OLED_SPI_MOSI 11
#define OLED_CMD      8

class OLED12864 {

public:
    enum {
        SCREEN_PIXEL_WIDTH = 128,
        SCREEN_PIXEL_HEIGHT = 64,
    };

    OLED12864();
    ~OLED12864();

    void init();
    void clear();
    void fillData(const void * data);
    void fillData8bit(const void * data);
    
    void fillPattern(unsigned char pattern);

    void setPos(int x, int y);
    
protected:
    void txCmd(unsigned char cmd);
    void txData(unsigned char cmd);
    unsigned char renderBuffer[SCREEN_PIXEL_WIDTH*SCREEN_PIXEL_HEIGHT/8];
};

