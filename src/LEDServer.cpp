#include "LEDServer.h"

int freq = 100;
int RledChannel = 0;
int GledChannel = 1;
int BledChannel = 2;
int resolution = 8;


LEDServer *LEDServerInstance;
hw_timer_t *frametimer = NULL;
void IRAM_ATTR onTimer();

LEDServer::LEDServer(debugdisplay *disp)
{
    LEDServerInstance = this;
    display = *disp;
    serverConfigData = new serverConfig();
}

LEDServer::LEDServer()
{
    LEDServerInstance = this;
    serverConfigData = new serverConfig();
}

int LEDServer::start()
{ 
    
    //returns 1 for successfull start, anything else is an error
    display.clearScreen();
    Serial.println("");
    Serial.println("");
    Serial.println("");

    Serial.println("LedServer starting");

    if (this->readWifiConfig())
    {
        Serial.println("Config Read.");
    }
    else
    {
        Serial.println("Config read FAIL");
        return 2;
    }

    this->connectWifi();

    


    if (serverConfigData->LEDVariant == "WS")
    {
        this->leds = new CRGB[serverConfigData->numberLEDS];
        FastLED.addLeds<NEOPIXEL, PIN_WS2812>(leds, 10);
        FastLED.setBrightness(255);
        this->leds[0] = 0x00FF00;
        this->leds[1] = 0x00FF00;
        FastLED.show();
        Serial.println("LEDs init OK");
    }

    else if (serverConfigData->LEDVariant == "RGB"){
        ledcSetup(RledChannel, freq, resolution);
        ledcAttachPin(PIN_MOSFET_R, RledChannel);
        ledcSetup(GledChannel, freq, resolution);
        ledcAttachPin(PIN_MOSFET_G, GledChannel);
        ledcSetup(BledChannel, freq, resolution);
        ledcAttachPin(PIN_MOSFET_B, BledChannel);
        Serial.println("RGB LEDs active!");
    }

    if (serverConfigData->devicerole == "MASTER")
    {
        Serial.println("Device is Master");
        this->startMasterServer();
    }
    else if (serverConfigData->devicerole == "SLAVE")
    {
        Serial.println("Device is Slave");
        this->startSlaveServer();
    }

    return 1;
}

bool LEDServer::connectWifi(){
    if (serverConfigData->isdefault == true)
    {
        display.printS(60, 0, "Default");
        enterAPMode();
        return true;
    }
    else if (serverConfigData->stationSSID == "0")
    {
        display.printS(60, 0, "is 0");
        enterAPMode();
        return true;
    }
    else
    {
        WiFi.begin(serverConfigData->stationSSID.c_str(), serverConfigData->stationPWD.c_str());
        Serial.println("Connecting to Wifi:");
        Serial.println("SSID: " + serverConfigData->stationSSID);
        Serial.println("PWD: " + serverConfigData->stationPWD);
        long connectionTimeout = millis();
        long texttime = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            if (connectionTimeout + 20000 > millis())
            {
                if (texttime + 1000 < millis()){
                   Serial.println("Trying to Connect..."); 
                   display.clearScreen();
                   display.printS(0,0, "Connecting to Wifi...");
                   texttime = millis();
                }
                
            }
            else
            { //Connection Timed Out
                display.printS(60, 0, "Timeout");
                Serial.println("Connection Timed Out!");
                enterAPMode();
                return true;
            }
        }
        Serial.println("Connected!");
        display.printS(0, 40, "Connected to:");
        display.printS(0, 50, WiFi.SSID());
        return true;
    }
}

bool LEDServer::enterAPMode(){
    Serial.println("Starting ConfigAP");
    WiFi.softAP(serverConfigData->APSSID.c_str(), serverConfigData->APPWD.c_str());
    Serial.println("AP started!");
    Serial.println("IP: " + WiFi.softAPIP().toString());
    Serial.println("SSID: " + serverConfigData->APSSID);
    Serial.println("AP PWD: " + serverConfigData->APPWD);

    display.clearScreen();
    display.printS(0,0, "AP open: " + serverConfigData->APSSID);
    display.printS(0,10, "AP PWD: " + serverConfigData->APPWD);
    display.printS(0,20, "IP: " + WiFi.softAPIP().toString());
    return true;
}

bool LEDServer::startMasterServer()
{
    if (WiFi.status() == WL_CONNECTED){
        display.printS(0, 0, "Master@" + WiFi.localIP().toString());
        display.printS(0, 10, WiFi.SSID());
    }
   
    WebServer = new AsyncWebServer(80);
    wsServer = new AsyncWebSocket("/");
    udp = new AsyncUDP();
    wsServer->onEvent(this->onWsEvent);
    WebServer->addHandler(wsServer);

    WebServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/site.html");
    });
    WebServer->serveStatic("/stylesheet.css", SPIFFS, "/stylesheet.css");
    WebServer->serveStatic("/script.js", SPIFFS, "/script.js");
    WebServer->serveStatic("/jscolor.js", SPIFFS, "/jscolor.js");
    WebServer->serveStatic("/js/bootstrap.min.js", SPIFFS, "/js/bootstrap.min.js");
    WebServer->serveStatic("/css/bootstrap.min.css", SPIFFS, "css/bootstrap.min.css");


    WebServer->serveStatic("/wificonfigsite.html", SPIFFS, "/wificonfigsite.html");
    WebServer->serveStatic("/configscript.js", SPIFFS, "/configscript.js");

    WebServer->begin();
    Serial.println("Server started");

    frametimer = timerBegin(0, 80, true);
    timerAttachInterrupt(frametimer, &onTimer, true);
    timerAlarmWrite(frametimer, 33333, true);
    timerAlarmEnable(frametimer);
    return 1;
}

bool LEDServer::startSlaveServer()
{
    display.printS(0, 0, "Slave");
    udp = new AsyncUDP();
    if (udp->listenMulticast(IPAddress(239, 1, 2, 3), 1234))
    {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp->onPacket([](AsyncUDPPacket packet) {
            Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
            Serial.println();
        });
    }
    return true;
}

void IRAM_ATTR onTimer()
{
    LEDServerInstance->InternalTimerISR();
}

void IRAM_ATTR LEDServer::InternalTimerISR()
{
    this->serverStateData.interruptCounter++;
}

bool LEDServer::update()
{
    if (this->serverStateData.interruptCounter > 0)
    {
        this->serverStateData.interruptCounter = 0;

        //
        //  If new Color hasnt been Displayed yet
        //
        if (this->serverStateData.OutputDone == false)
        {
            //
            //  If Led Variant is WS
            //
            if (this->serverConfigData->LEDVariant == "WS")
            {
                for (int i = 0; i < this->serverConfigData->numberLEDS; i++)
                {
                    if ((this->serverStateData.solidcolor_r + this->serverStateData.solidcolor_g + this->serverStateData.solidcolor_b) > 5)
                    {
                        digitalWrite(PIN_POWERRELAY, HIGH);
                    }
                    else
                    {
                        digitalWrite(PIN_POWERRELAY, LOW);
                    }
                    this->leds[i].r = this->serverStateData.solidcolor_r;
                    this->leds[i].g = this->serverStateData.solidcolor_g;
                    this->leds[i].b = this->serverStateData.solidcolor_b;
                }
                FastLED.show();
                this->serverStateData.OutputDone = true;
                Serial.println("WS2812b ColorUpdate!");
                Serial.println("");
            }

            else if (this->serverConfigData->LEDVariant == "RGB"){
                ledcWrite(RledChannel, this->serverStateData.solidcolor_r);
                ledcWrite(GledChannel, this->serverStateData.solidcolor_g);
                ledcWrite(BledChannel, this->serverStateData.solidcolor_b);
                Serial.println(this->serverStateData.solidcolor_r);
                Serial.println(this->serverStateData.solidcolor_g);
                Serial.println(this->serverStateData.solidcolor_b);
                this->serverStateData.OutputDone = true;
                Serial.println("RGB ColorUpdate!");
            }
        }
    }
}

void LEDServer::UDPBroadcast()
{
    String test = "Broadcast" + (String)millis();
    udp->broadcastTo(test.c_str(), 1234);

    if (!udp->connected())
    {
        udp->connect(IPAddress(239, 1, 2, 3), 1234);
        Serial.println("UDP Connected");
    }
    udp->print("Print!");
    //udp->sendTo("Sentto", IPAddress(239,1,2,3), 1234);
    Serial.println("Broadcastet!");
}

bool LEDServer::readWifiConfig()
{
    File wificonfigfile = SPIFFS.open("/wificonfig.json", FILE_READ);
    if (!wificonfigfile)
    {
        display.printS(0, 30, "WIFICONFIG FAILED");
    }
    else
    {
        //display.printS(0, 20, "WIFICONFIG OK");
        Serial.println("WIFICONFIG OK");
        size_t size = wificonfigfile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        wificonfigfile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());
        serverConfigData->devicerole = json["DEVICEROLE"].as<String>();
        serverConfigData->isdefault = json["isDefault"];
        serverConfigData->APSSID = json["APSSID"].as<String>();
        serverConfigData->APPWD = json["APPWD"].as<String>();
        serverConfigData->stationSSID = json["stationSSID"].as<String>();
        serverConfigData->stationPWD = json["stationPWD"].as<String>();
        serverConfigData->recieverID = json["recieverID"].as<int>();
        serverConfigData->recieverNameTag = json["recieverNameTag"].as<String>();
        serverConfigData->LEDVariant = json["LEDVariant"].as<String>();
        serverConfigData->numberLEDS = json["numberLEDS"];

        Serial.print("DeviceConfig from JSON ");
        Serial.print("  devicerole:");
        Serial.print(serverConfigData->devicerole);
        Serial.print("  LEDVariant:");
        Serial.print(serverConfigData->LEDVariant);
        Serial.print("  recieverID:");
        Serial.print(serverConfigData->recieverID);
        Serial.print("  recieverNameTag:");
        Serial.print(serverConfigData->recieverNameTag);
        Serial.print("  recieverNameTag:");
        Serial.println(serverConfigData->numberLEDS);

        //display.printS(0, 30, "Json OK: " + serverConfigData->stationSSID);
        //Serial.println("Json OK: " + serverConfigData->stationSSID);
        wificonfigfile.close();
    }
    return true;
}

bool LEDServer::saveWifiConfig()
{
    File wificonfigfile = SPIFFS.open("/wificonfig.json", FILE_WRITE);
    if (wificonfigfile)
    {
        DynamicJsonBuffer jsonBuffer;
        JsonObject &savejson = jsonBuffer.createObject();

        savejson["isDefault"] = serverConfigData->isdefault;
        savejson["APSSID"] = serverConfigData->APSSID;
        savejson["APPWD"] = serverConfigData->APPWD;
        savejson["stationSSID"] = serverConfigData->stationSSID;
        savejson["stationPWD"] = serverConfigData->stationPWD;
        savejson["recieverID"] = serverConfigData->recieverID;
        savejson["recieverNameTag"] = serverConfigData->recieverNameTag;
        savejson["DEVICEROLE"] = serverConfigData->devicerole;
        savejson["numberLEDS"] = serverConfigData->numberLEDS;
        savejson["LEDVariant"] = serverConfigData->LEDVariant;
        savejson.printTo(wificonfigfile);
        wificonfigfile.close();

        /*####################
        Display new Config
        ######################*/
        display.clearScreen();
        display.print(0, 0, "New Config:");
        display.print(0, 10, "StationSSID: " + serverConfigData->stationSSID);
        display.print(0, 20, "stationPWD: " + serverConfigData->stationPWD);
        display.show();

        return true;
    }
    else
    {
        return false;
    }
}

bool LEDServer::factoryreset()
{
    File resetfile = SPIFFS.open("/resetconfig.json", FILE_READ);
    if (!resetfile)
    {
        Serial.println("Resetfile not readable");
        return false;
    }
    Serial.println("Resetfile Read");
    size_t size = resetfile.size();
    std::unique_ptr<char[]> buf(new char[size]);
    resetfile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());
    LEDServerInstance->serverConfigData->devicerole = json["DEVICEROLE"].as<String>();
    LEDServerInstance->serverConfigData->isdefault = json["isDefault"];
    LEDServerInstance->serverConfigData->APSSID = json["APSSID"].as<String>();
    LEDServerInstance->serverConfigData->APPWD = json["APPWD"].as<String>();
    LEDServerInstance->serverConfigData->stationSSID = json["stationSSID"].as<String>();
    LEDServerInstance->serverConfigData->stationPWD = json["stationPWD"].as<String>();
    LEDServerInstance->serverConfigData->recieverID = json["recieverID"].as<int>();
    LEDServerInstance->serverConfigData->recieverNameTag = json["recieverNameTag"].as<String>();
    LEDServerInstance->serverConfigData->LEDVariant = json["LEDVariant"].as<String>();
    LEDServerInstance->serverConfigData->numberLEDS = json["numberLEDS"].as<int>();
    LEDServerInstance->saveWifiConfig();
    Serial.println("Factory Reset complete! restarting...");
    Serial.println(json["LEDVariant"].as<String>());
    Serial.println(LEDServerInstance->serverConfigData->LEDVariant);
    Serial.println(LEDServerInstance->serverConfigData->numberLEDS);
    delay(3000);
    ESP.restart();
    return true;
}

void LEDServer::handleMessage(AsyncWebSocketClient *client, uint8_t *rawdata, String msg)
{
    client->text(msg);
    if (msg == "led_on")
    {
    }

    DynamicJsonBuffer JSONBuffer; //Memory pool
    JsonObject &parsed = JSONBuffer.parseObject(msg);

    if (!parsed.success())
    { //Check for errors in parsing
    }
    else if (parsed.success())
    {
        Serial.print("Command:  ");
        Serial.println(parsed["COMMAND"].as<String>());
        if (parsed["COMMAND"] == "reset")
        {
            client->printf("ESP32 restarting");
            Serial.println("Restart in 500ms");
            delay(500);
            ESP.restart();
        }
        else if (parsed["COMMAND"] == "facreset")
        {
            factoryreset();
        }
        else if (parsed["COMMAND"] == "config")
        {
            LEDServerInstance->serverConfigData->stationSSID = parsed["stationSSID"].as<String>();
            LEDServerInstance->serverConfigData->stationPWD = parsed["stationPWD"].as<String>();
            LEDServerInstance->serverConfigData->isdefault = parsed["isDefault"].as<boolean>();
            LEDServerInstance->serverConfigData->devicerole = parsed["isMaster"].as<String>();
            LEDServerInstance->serverConfigData->LEDVariant = parsed["LEDVariant"].as<String>();
            LEDServerInstance->serverConfigData->numberLEDS = parsed["numberLEDS"].as<int>();
            Serial.println((String) parsed["numberLEDS"].as<int>());
            LEDServerInstance->saveWifiConfig();
            Serial.println("Saved Config:");
            Serial.println("SSID:" + LEDServerInstance->serverConfigData->stationSSID);
            Serial.println("stationPWD:" + LEDServerInstance->serverConfigData->stationPWD);
            Serial.println("isDefault:" + LEDServerInstance->serverConfigData->isdefault);
            Serial.println("Devicerole:" + LEDServerInstance->serverConfigData->devicerole);
            Serial.println("LEDVariant:" + LEDServerInstance->serverConfigData->LEDVariant);
            Serial.println("numberLEDS:" + LEDServerInstance->serverConfigData->numberLEDS);
        }
        else if (parsed["COMMAND"] == "InputTransmit")
        {
            int rgbdata;
            sscanf((const char *)parsed["COLOR"], "%x", &rgbdata);
            LEDServerInstance->serverStateData.solidcolor_r = (rgbdata >> 16) & 0xFF;
            LEDServerInstance->serverStateData.solidcolor_g = (rgbdata >> 8) & 0xFF;
            LEDServerInstance->serverStateData.solidcolor_b = rgbdata & 0xFF;
            LEDServerInstance->serverStateData.OutputDone = false;
        }
    }
}

void LEDServer::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                          AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("Websocket Client [%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("Websocket Client [%s][%u] disconnect: %u\n", server->url(),
                      client->id());
    }
    else if (type == WS_EVT_DATA)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        String msg = "";
        if (info->final && info->index == 0 && info->len == len)
        {
            //the whole message is in a single frame and we got all of it's data

            if (info->opcode == WS_TEXT)
            {
                for (size_t i = 0; i < info->len; i++)
                {
                    msg += (char)data[i];
                }
            }
            else
            {
                char buff[3];
                for (size_t i = 0; i < info->len; i++)
                {
                    sprintf(buff, "%02x ", (uint8_t)data[i]);
                    msg += buff;
                }
            }
            Serial.printf("%s\n", msg.c_str());
            handleMessage(client, data, msg);
        }
        else
        {
            //message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0)
            {
                if (info->num == 0)
                {
                }
            }
            if (info->opcode == WS_TEXT)
            {
                for (size_t i = 0; i < info->len; i++)
                {
                    msg += (char)data[i];
                }
            }
            else
            {
                char buff[3];
                for (size_t i = 0; i < info->len; i++)
                {
                    msg += buff;
                }
            }
            if ((info->index + len) == info->len)
            {
                if (info->final)
                {
                    if (info->message_opcode == WS_TEXT)
                        client->text("I got your text message");
                    else
                        client->binary("I got your binary message");
                }
            }
        }
    }
}
