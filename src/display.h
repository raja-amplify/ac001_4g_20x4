#if DISPLAY_ENABLED

#ifndef DISPLAY_H
#define DISPLAY_H

#include<SoftwareSerial.h>
#include<ArduinoJson.h>
#include<StreamUtils.h>

void setupDisplay_Disp();
void defaultScreen_Disp();
void statusOfCharger_Disp(String);
void authenticatingRFID_Disp();
void authoriseStatus_Disp(bool);
void displayEnergyValues_Disp(String, String, String);
void cloudConnect_Disp(bool);
bool checkForResponse_Disp();
void thanks_Disp(String value);
void startmastertxn();
int checkForPin();
bool checkForBtn();

#endif
#endif