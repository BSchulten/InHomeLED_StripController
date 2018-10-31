#ifndef LEDServer_h_
#define LEDServer_h_

#include "config.h"
#include "DebugDISPOled.h"
#include "FS.h"
#include "SPIFFS.h"
#include <SPIFFSEditor.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

class serverConfig{
    public:
        bool isdefault;
        String APSSID;
        String APPWD;
        String stationSSID;
        String stationPWD;
        int recieverID;
        String recieverNameTag;
        String devicerole;
    private:
};

class LEDServer{
    public:
        LEDServer(debugdisplay *disp);
        serverConfig *serverConfigData;
        LEDServer();
        int start();
        bool startMasterServer();
        bool startSlaveServer();
        bool readWifiConfig();
        bool stop();
        static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len);
        static void handleMessage(AsyncWebSocketClient *client, uint8_t *rawdata, String msg);
        AsyncWebServer *WebServer;
        AsyncWebSocket *wsServer;
    private:
        debugdisplay display;
};

#endif