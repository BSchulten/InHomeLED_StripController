/*
  InHomeLED Settings file
*/
#ifndef DebugDISPOled_h_
#define DebugDISPOled_h_

#include "config.h"
#include "SSD1306.h"

class debugdisplay{
    public:
        SSD1306Wire *display;
        debugdisplay(void);
        void init(void);
        void print(int16_t x, int16_t y, String string);
        void printS(int16_t x, int16_t y, String string);
        void show(void);
        void clearScreen();
};

#endif