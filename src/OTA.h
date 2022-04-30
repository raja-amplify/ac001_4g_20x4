#ifndef OTA_H
#define OTA_H
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>
#include"OcppEngine.h"
#include"ChargePointStatusService.h"


void setupOTA();
void performUpdate(WiFiClient& updateSource, size_t updateSize);
void ota_Loop();

#endif