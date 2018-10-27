#include "SPIFFSconfig.h"
wifiConfigManager *configmanagerhelper;
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len);

wifiConfigManager::wifiConfigManager(debugdisplay *disp)
{
    display = *disp;
    configmanagerhelper = this;
}

bool wifiConfigManager::readWifiConfig()
{
    File wificonfigfile = SPIFFS.open("/wificonfig.json", FILE_READ);
    if (!wificonfigfile)
    {
        display.printS(0, 20, "WIFICONFIG FAILED");
    }
    else
    {
        display.printS(0, 20, "WIFICONFIG OK");

        size_t size = wificonfigfile.size();
        std::unique_ptr<char[]> buf(new char[size]);
        wificonfigfile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject &json = jsonBuffer.parseObject(buf.get());

        configData.isdefault = json["isDefault"];
        configData.APSSID = json["APSSID"].as<String>();
        configData.APPWD = json["APPWD"].as<String>();
        configData.stationSSID = json["stationSSID"].as<String>();
        configData.stationPWD = json["stationPWD"].as<String>();
        configData.recieverID = json["recieverID"].as<int>();
        configData.recieverNameTag = json["recieverNameTag"].as<String>();

        display.printS(0, 30, "Json OK: " + configmanagerhelper->configData.stationSSID);
        wificonfigfile.close();
    }
    return true;
}

bool wifiConfigManager::saveWifiConfig()
{
    File wificonfigfile = SPIFFS.open("/wificonfig.json", FILE_WRITE);
    if (wificonfigfile)
    {
        DynamicJsonBuffer jsonBuffer;
        JsonObject &savejson = jsonBuffer.createObject();

        savejson["isDefault"] = configData.isdefault;
        savejson["APSSID"] = configData.APSSID;
        savejson["APPWD"] = configData.APPWD;
        savejson["stationSSID"] = configData.stationSSID;
        savejson["stationPWD"] = configData.stationPWD;
        savejson["recieverID"] = configData.recieverID;
        savejson["recieverNameTag"] = configData.recieverNameTag;
        savejson.printTo(wificonfigfile);
        wificonfigfile.close();

        /*####################
        Display new Config
        ######################*/
        display.clearScreen();
        display.print(0, 0, "New Config:");
        display.print(0, 10, "StationSSID: " + configData.stationSSID);
        display.print(0, 20, "stationPWD: " + configData.stationPWD);
        display.show();

        return true;
    }
    else
    {
        return false;
    }
}

bool wifiConfigManager::enterSetupAPMode()
{
    Serial.println("Starting ConfigAP");
    WiFi.softAP(configData.APSSID.c_str(), configData.APPWD.c_str());
    display.printS(0, 40, "AP running:" + configData.APSSID);
    configserver = new AsyncWebServer(80);
    configws = new AsyncWebSocket("/");

    configws->onEvent(onWsEvent);
    configserver->addHandler(configws);

    configserver->on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/wificonfigsite.html");
    });
    configserver->on("/html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/wificonfigsite.html", "text/html", false);
    });
    configserver->serveStatic("/configscript.js", SPIFFS, "/configscript.js");
    configserver->begin();
    display.printS(0, 50, WiFi.softAPIP().toString());
    Serial.println("ConfigAP started!");
    Serial.println("SSID: " + WiFi.softAPIP().toString());
    Serial.println("SSID: " + configData.APPWD);
    return true;
}

bool wifiConfigManager::handleSetup()
{
    this->readWifiConfig();
    if (configData.isdefault == true)
    {
        display.printS(60, 0, "Default");
        enterSetupAPMode();
        return true;
    }
    else if (configData.stationSSID == "0")
    {
        display.printS(60, 0, "is 0");
        enterSetupAPMode();
        return true;
    }
    else
    {
        WiFi.begin(configData.stationSSID.c_str(), configData.stationPWD.c_str());
        Serial.println("Connecting to Wifi:");
        Serial.println("SSID: " + configData.stationSSID);
        Serial.println("PWD: " + configData.stationPWD);
        long connectionTimeout = millis();
        long texttime = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            if (connectionTimeout + 20000 > millis())
            {
                if (texttime + 1000 < millis()){
                   Serial.println("Trying to Connect..."); 
                   texttime = millis();
                }
                
            }
            else
            { //Connection Timed Out
                display.printS(60, 0, "Timeout");
                Serial.println("Connection Timed Out!");
                enterSetupAPMode();
                return true;
            }
        }
        Serial.println("Connected!");
        display.printS(0, 40, "Connected to:");
        display.printS(0, 50, WiFi.SSID());
        return true;
    }
    return true;
}

bool wifiConfigManager::connectWifi()
{
    this->readWifiConfig();
    if (configData.isdefault == true)
    {
        display.printS(60, 0, "Default");
        WiFi.softAP(configData.APSSID.c_str(), configData.APPWD.c_str());
        display.printS(0, 40, "AP running:" + configData.APSSID);
        return false;
    }
    else if (configData.stationSSID == "0")
    {
        display.printS(60, 0, "is 0");
        WiFi.softAP(configData.APSSID.c_str(), configData.APPWD.c_str());
        display.printS(0, 40, "AP running:" + configData.APSSID);
        return false;
    }
    else
    {
        WiFi.begin(configData.stationSSID.c_str(), configData.stationPWD.c_str());
        long connectionTimeout = millis();
        while (WiFi.status() != WL_CONNECTED)
        {
            if (connectionTimeout + 15000 > millis())
            {
                //Waiting for Connection
            }
            else
            { //Connection Timed Out
                display.printS(60, 0, "Timeout");
                WiFi.softAP(configData.APSSID.c_str(), configData.APPWD.c_str());
                display.printS(0, 40, "AP running:" + configData.APSSID);
                return false;
            }
        }
        display.printS(0, 40, "Connected to:");
        display.printS(0, 50, WiFi.SSID());
        return true;
    }
}

void handleMessage(AsyncWebSocketClient *client, uint8_t *rawdata, String msg)
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
        if (parsed["command"] == "reset")
        {
            client->printf("ESP32 restarting");
            delay(500);
            ESP.restart();
        }
        else if (parsed["command"] == "config")
        {
            configmanagerhelper->configData.stationSSID = parsed["stationSSID"].as<String>();
            configmanagerhelper->configData.stationPWD = parsed["stationPWD"].as<String>();
            configmanagerhelper->configData.isdefault = parsed["isDefault"].as<boolean>();
            configmanagerhelper->saveWifiConfig();
            Serial.println("Saved Config:");
            Serial.println("SSID:" + configmanagerhelper->configData.stationSSID);
            Serial.println("stationPWD:" + configmanagerhelper->configData.stationPWD);
            Serial.println("isDefault:" + configmanagerhelper->configData.isdefault);
            
        }
        else
        {
        }
    }
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
               AwsEventType type, void *arg, uint8_t *data, size_t len)
{
    if (type == WS_EVT_CONNECT)
    {
        Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
        client->printf("Hello Client %u :)", client->id());
        client->ping();
    }
    else if (type == WS_EVT_DISCONNECT)
    {
        Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(),
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

/*
*
* Legacy Stuff
*
*/

String processor(const String &var)
{
    if (var == "PLACEHOLDER")
    {
        int n = WiFi.scanNetworks();
        String networkstring;
        for (int i = 0; i < n; i++)
        {
            networkstring += "<p> ";
            networkstring += WiFi.SSID(i);
            networkstring += " Signal Strenght: ";
            networkstring += WiFi.RSSI(i);
            networkstring += "</p> ";
        }
        return networkstring;
    }
    return String();
}