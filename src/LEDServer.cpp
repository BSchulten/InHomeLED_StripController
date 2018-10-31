#include "LEDServer.h"

LEDServer::LEDServer(debugdisplay *disp)
{
    display = *disp;
    serverConfigData = new serverConfig();
}

LEDServer::LEDServer()
{
    serverConfigData = new serverConfig();
}

int LEDServer::start(){ //returns 1 for successfull start, anything else is an error
    if (this->readWifiConfig()){
        Serial.println("Read Config");
    }
    else {
        Serial.println("Config read FAIL");
        return 2;
    }
    
    if (serverConfigData->devicerole == "MASTER"){
        Serial.println("Device is Master");
        this->startMasterServer();
    }
    else if (serverConfigData->devicerole == "SLAVE"){
        Serial.println("Device is Slave");
    }
}

bool  LEDServer::startMasterServer(){
    WebServer = new AsyncWebServer(80);
    wsServer = new AsyncWebSocket("/");

    wsServer->onEvent(this->onWsEvent);
    WebServer->addHandler(wsServer);

    WebServer->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
	{
		request->send(SPIFFS, "/site.html");
	});
	WebServer->serveStatic("/stylesheet.css", SPIFFS, "/stylesheet.css");
	WebServer->serveStatic("/script.js", SPIFFS, "/script.js");
	WebServer->serveStatic("/jscolor.js", SPIFFS, "/jscolor.js");
	WebServer->serveStatic("/js/bootstrap.min.js", SPIFFS, "/js/bootstrap.min.js");
	WebServer->serveStatic("/css/bootstrap.min.css", SPIFFS, "css/bootstrap.min.css");
	WebServer->begin();

    Serial.println("Server started");

    return 1;
}


bool LEDServer::readWifiConfig()
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
        serverConfigData->devicerole = json["DEVICEROLE"].as<String>();
        serverConfigData->isdefault = json["isDefault"];
        serverConfigData->APSSID = json["APSSID"].as<String>();
        serverConfigData->APPWD = json["APPWD"].as<String>();
        serverConfigData->stationSSID = json["stationSSID"].as<String>();
        serverConfigData->stationPWD = json["stationPWD"].as<String>();
        serverConfigData->recieverID = json["recieverID"].as<int>();
        serverConfigData->recieverNameTag = json["recieverNameTag"].as<String>();
        
        display.printS(0, 30, "Json OK: " + serverConfigData->stationSSID);
        wificonfigfile.close();
    }
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
        if (parsed["command"] == "reset")
        {
            client->printf("ESP32 restarting");
            delay(500);
            ESP.restart();
        }
        else if (parsed["command"] == "config")
        {
        }
        else
        {
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

void LEDServer::broadcastUDPTest(){

}