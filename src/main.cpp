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


debugdisplay display;
wifiConfigManager configmanager(&display);


void initSPIFFS();

void setup() {
    display.init();
    initSPIFFS();

    configmanager.handleSetup();
}

void loop() {
    
}

void initSPIFFS(){
    if (SPIFFS.begin(false, "/spiffs", 10)){
        display.printS(0,10, "SPIFFS OK");
    }
    else {
        display.printS(0,10, "SPIFFS FAIL!");
    }
}

