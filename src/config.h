/*
  InHomeLED Settings file
*/
#ifndef config_h_
#define config_h_

/* 
*   SetupConfigs
*/

#define I2CDisplay_CONNECTED true
#define SERIALOUT true


/* 
*   DisplayType Setup
*   Uncommend the right Type of Display
*/
//#define DISPLAYTYPE  GEOMETRY_128_32
#define DISPLAYTYPE  GEOMETRY_128_64

/*
*
*   Pin Configs go here, defaults for LEDControler v1
*
*/

#define PIN_SCL 21
#define PIN_SDA 22
#define PIN_MOSFET_R 17 // 4
#define PIN_MOSFET_G 18 // 17
#define PIN_MOSFET_B 19 // 27
#define PIN_WS2812 25
#define PIN_OUTPUTENABLE 19
#define PIN_POWERRELAY 16


#endif