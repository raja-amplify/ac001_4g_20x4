#ifndef EVSE_B_OFFLINE_H
#define EVSE_B_OFFLINE_H

#include <Arduino.h>
#include<Preferences.h>
#include<WiFi.h>
#include "ChargePointStatusService.h"
#include "SimpleOcppOperationFactory.h"
#include "OcppEngine.h"
#include "Variants.h"
#include "Peripherals.h"

//OCPP message classes
#include "OcppOperation.h"
#include "OcppMessage.h"
#include "Authorize.h"
#include "BootNotification.h"
#include "Heartbeat.h"
#include "StartTransaction.h"
#include "StopTransaction.h"
#include "DataTransfer.h"
#include "CustomGsm.h"


typedef void (*OnBoot_B)();
typedef void (*OnReadUserId_B)();
typedef void (*OnSendHeartbeat_B)();
typedef void (*OnAuthentication_B)();
typedef void (*OnStartTransaction_B)();
typedef void (*OnStopTransaction_B)();
typedef void (*OnUnauthorizeUser_B)();

void EVSE_B_initialize();
void EVSE_B_setup();
void EVSE_B_setChargingLimit(float limit);

bool EVSE_B_EvRequestsCharge();

bool EVSE_B_EvIsPlugged();

void EVSE_B_setOnBoot(OnBoot_B onBt_B);
void EVSE_B_setOnReadUserId(OnReadUserId_B onReadUsrId_B);
void EVSE_B_setOnsendHeartbeat(OnSendHeartbeat_B onSendHeartbt_B);
void EVSE_B_setOnAuthentication(OnAuthentication_B onAuthenticatn_B);
void EVSE_B_setOnStartTransaction(OnStartTransaction_B onStartTransactn_B);
void EVSE_B_setOnStopTransaction(OnStopTransaction_B onStopTransactn_B);
void EVSE_B_setOnUnauthorizeUser(OnUnauthorizeUser_B onUnauthorizeUsr_B);

void displayMeterValuesB();

void EVSE_B_loop();
//void emergencyRelayClose_Loop();

void EVSE_B_getChargePointSerialNumber(String &out);
char *EVSE_B_getChargePointVendor();
char *EVSE_B_getChargePointModel();

String EVSE_B_getCurrnetIdTag(MFRC522 * mfrc522);
String EVSE_B_readRFID(MFRC522 * mfrc522);
void EVSE_B_stopTransactionByRfid();
void emergencyRelayClose_Loop_B();

void EVSE_B_StartCharging();
void EVSE_B_Suspended();
void EVSE_B_StopSession();

void EVSE_B_startOfflineTxn();
void EVSE_B_offline_Loop();
void EVSE_B_stopOfflineTxn();
void EVSE_B_LED_loop();

#endif