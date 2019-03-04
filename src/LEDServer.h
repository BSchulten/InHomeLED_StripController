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
#include "AsyncUDP.h"
#include <FastLED.h>

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
        String LEDVariant;
        int numberLEDS;
        String extrafeatures;
        IPAddress* Slaves;
    private:
};

class serverState{
    public:
        int solidcolor_r;
        int solidcolor_g;
        int solidcolor_b;
        int mode;
        String lastRawMsg;
        bool OutputDone = false;
        int interruptCounter = 0;
        volatile int PCRCounter = 0;
        int framecounter = 0;
        long lastswitchtoggle = 0; 
        String UDPmsg;
    private:
};

class LEDServer{
    public:
        LEDServer(debugdisplay *disp);
        serverConfig* serverConfigData;
        LEDServer();
        int start();
        CRGB *leds;
        void UDPBroadcast();
        bool readWifiConfig();
        bool stop();
        static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len);
        static void handleMessage(AsyncWebSocketClient *client, uint8_t *rawdata, String msg);
        void IRAM_ATTR InternalTimerISR();
        bool update();
        static bool factoryreset();
        bool saveWifiConfig();
        bool enterAPMode();
        bool connectWifi();
        void InternalPCR();
        void handleUDPMessage(uint8_t* msg);

        AsyncWebServer *WebServer;
        AsyncWebSocket *wsServer;
        AsyncUDP *udp;
        serverState serverStateData;
    private:
        debugdisplay display;
        bool startMasterServer();
        bool startSlaveServer();
        

};

#endif