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
void displayEnergyValues_Disp_AC(String, String, String, String);
void connAvail(uint8_t , String );
//void cloudConnect_Disp(bool);
void cloudConnect_Disp(short int);
bool checkForResponse_Disp();
void thanks_Disp(String);
void thanks_Disp_AC(String, String ,String );
void setHeader(String);
void startmastertxn();
int checkForPin();
bool checkForBtn();

#endif