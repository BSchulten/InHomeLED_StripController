#include "DebugDISPOled.h"

debugdisplay::debugdisplay(void){
    this->display = new SSD1306Wire(0x3c, PIN_SCL, PIN_SDA,  DISPLAYTYPE);
    
}

void debugdisplay::init(void){
    display->init();
    display->flipScreenVertically();
	display->setFont(ArialMT_Plain_10);
	display->setColor(WHITE);
	display->setTextAlignment(TEXT_ALIGN_LEFT);
    display->drawString(0, 0, "Display OK");
    display->display();
}

void debugdisplay::print(int16_t x, int16_t y, String string){ //x-> right  y->down
    display->drawString(x , y, string);
}

void debugdisplay::show(void){
    display->display();
}

void debugdisplay::printS(int16_t x, int16_t y, String string){ //x-> right  y->down
    display->drawString(x , y, string);
    display->display();
}

void debugdisplay::clearScreen(){
    display->clear();
}