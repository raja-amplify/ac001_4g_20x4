// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef EVSE_A_OFFLINE_H
#define EVSE_A_OFFLINE_H

#include <Arduino.h>
#include<Preferences.h>
#include<WiFi.h>
#include "ChargePointStatusService.h"
#include "OcppEngine.h"
#include "Variants.h"
#include "Peripherals.h"
#include "SimpleOcppOperationFactory.h"

//OCPP message classes
#include "OcppOperation.h"
#include "OcppMessage.h"
#include "Authorize.h"
#include "BootNotification.h"
#include "Heartbeat.h"
#include "StartTransaction.h"
#include "StopTransaction.h"
#include "DataTransfer.h"
#include <EEPROM.h>
#include "CustomGsm.h"

/*
#if STEVE

#define WS_HOST "13.233.136.157"
#define PORT 8080

#if WIFI_ENABLED || ETHERNET_ENABLED
#define WS_URL_PREFIX "ws://13.233.136.157:8080/steve/websocket/CentralSystemService/"
#endif

#if GSM_ENABLED
#define WS_URL_PREFIX "ws://steve/websocket/CentralSystemService/" //omit Host address
#endif

#define WS_PROTOCOL "ocpp1.6"

#endif
/*******************/

/*
#if EVSECEREBRO

#define WS_HOST "188.165.223.94"
#define PORT 8140

#if WIFI_ENABLED || ETHERNET_ENABLED
#define WS_URL_PREFIX "ws://evcp.evsecerebro.com/websocket/amplify/"
#endif

#if GSM_ENABLED
#define WS_URL_PREFIX "ws://websocket/amplify/" //omit Host address
#endif

#define WS_PROTOCOL "ocpp1.6"

#endif

/*
#define RELAY_1 4
#define RELAY_2 5
#define RELAY_HIGH HIGH
#define RELAY_LOW LOW
*/

// https://cs.nyu.edu/courses/spring12/CSCI-GA.3033-014/Assignment1/function_pointers.html
typedef void (*OnBoot_A)();
typedef void (*OnReadUserId_A)();
typedef void (*OnSendHeartbeat_A)();
typedef void (*OnAuthentication_A)();
typedef void (*OnStartTransaction_A)();
typedef void (*OnStopTransaction_A)();
typedef void (*OnUnauthorizeUser_A)();

void EVSE_A_initialize();

void EVSE_A_setup();

void EVSE_A_setChargingLimit(float limit);

//not used
bool EVSE_A_EvRequestsCharge();

//not used
bool EVSE_A_EvIsPlugged();

//new names defined by @Pulkit. Might break the build.
void EVSE_A_setOnBoot(OnBoot_A onBt_A);
void EVSE_A_setOnReadUserId(OnReadUserId_A onReadUsrId_A);
void EVSE_A_setOnsendHeartbeat(OnSendHeartbeat_A onSendHeartbt_A);
void EVSE_A_setOnAuthentication(OnAuthentication_A onAuthenticatn_A);
void EVSE_A_setOnStartTransaction(OnStartTransaction_A onStartTransactn_A);
void EVSE_A_setOnStopTransaction(OnStopTransaction_A onStopTransactn_A);
void EVSE_A_setOnUnauthorizeUser(OnUnauthorizeUser_A onUnauthorizeUsr_A);
void displayMeterValues();

//not used
//void EVSE_afterEvPlug (AfterEvPlug afterEvPlug);
//
void EVSE_A_loop();
void emergencyRelayClose_Loop_A_Offl();
//other details.
void EVSE_A_getChargePointSerialNumber(String &out);
char *EVSE_A_getChargePointVendor();
char *EVSE_A_getChargePointModel();

//to read the ID Tag value.
String EVSE_A_getCurrnetIdTag(MFRC522 * mfrc522);
String EVSE_A_readRFID(MFRC522 * mfrc522);
void EVSE_A_stopTransactionByRfid();

void EVSE_A_StartCharging();
void EVSE_A_Suspended();
void EVSE_A_StopSession();

void EVSE_A_startOfflineTxn();
void EVSE_A_offline_Loop();
void EVSE_A_stopOfflineTxn();
void EVSE_A_LED_loop();
void showTxn_Finish();



#endif
