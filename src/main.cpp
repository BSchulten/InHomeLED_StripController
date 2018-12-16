#include <Arduino.h>
#include <WiFi.h>
#include "FS.h"
#include <ESPmDNS.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <SPIFFSEditor.h>
#include <ArduinoJson.h>

#include "config.h"
#include "DebugDISPOled.h"
#include "SPIFFSconfig.h"
#include "LEDServer.h"



/*
*    
*/
void initSPIFFS();
void setupPINS();


/*
*    Global Objects
*/
debugdisplay display;
wifiConfigManager configmanager(&display);
LEDServer mainserver(&display);

/*
*   System Functions
*/
void setup() {
    if (SERIALOUT == true){
        Serial.begin(115200);
    }
    setupPINS();
    display.init();
    initSPIFFS();
    //configmanager.handleSetup();
    //Serial.println(WiFi.SSID());
    //Serial.println(WiFi.localIP());
    mainserver.start();
    
}

long xtime = 0;
void loop() {
    if (xtime + 1000 < millis()){
        //mainserver.UDPBroadcast();
        xtime = millis();
    }

    mainserver.update();
}


/*
*    Helper Functions
*/
void initSPIFFS(){
    if (SPIFFS.begin(false, "/spiffs", 10)){
        //display.printS(0,10, "SPIFFS OK");
        Serial.println("SPIFFS OK");
    }
    else {
        //display.printS(0,10, "SPIFFS FAIL!");
        Serial.println("SPIFFS FAIL");
    }
}

void setupPINS(){
    pinMode(PIN_POWERRELAY, OUTPUT);
    digitalWrite(PIN_POWERRELAY, LOW);

    pinMode(PIN_OUTPUTENABLE, OUTPUT);
    digitalWrite(PIN_OUTPUTENABLE, LOW);
}


