#ifndef SPIFFSconfig_h_
#define SPIFFSconfig_h_

#include "config.h"
#include "DebugDISPOled.h"
#include "FS.h"
#include "SPIFFS.h"
#include <SPIFFSEditor.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class configSave{
    public:
        bool isdefault;
        String APSSID;
        String APPWD;
        String stationSSID;
        String stationPWD;
        int recieverID;
        String recieverNameTag;
    private:
};

class wifiConfigManager{
    public:
    debugdisplay display;
    configSave *configData;
    AsyncWebServer *configserver;
    AsyncWebSocket *configws;
    wifiConfigManager(debugdisplay *disp);
    bool readWifiConfig();
    bool saveWifiConfig();
    bool enterSetupAPMode();
    bool handleSetup();
    bool connectWifi();
    private:
};



#endif