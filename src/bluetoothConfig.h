#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BluetoothSerial.h> //@mwh
#include<EEPROM.h> //@mwh
#include<ArduinoJson.h> //@mwh
#include<WiFi.h>
#include <Preferences.h>
//void wifiConnectWithStoredCredential();
void bluetooth_Loop();
void eepromStoreCred();
bool VerifyCred(String message);
void startingBTConfig();

#endif