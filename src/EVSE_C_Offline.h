#ifndef EVSE_C_OFFLINE_H
#define EVSE_C_OFFLINE_H

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

typedef void (*OnBoot_C)();
typedef void (*OnReadUserId_C)();
typedef void (*OnSendHeartbeat_C)();
typedef void (*OnAuthentication_C)();
typedef void (*OnStartTransaction_C)();
typedef void (*OnStopTransaction_C)();
typedef void (*OnUnauthorizeUser_C)();

void EVSE_C_initialize();
void EVSE_C_setup();
void EVSE_C_setChargingLimit(float limit);

bool EVSE_C_EvRequestsCharge();

bool EVSE_C_EvIsPlugged();

void EVSE_C_setOnBoot(OnBoot_C onBt_C);
void EVSE_C_setOnReadUserId(OnReadUserId_C onReadUsrId_C);
void EVSE_C_setOnsendHeartbeat(OnSendHeartbeat_C onSendHeartbt_C);
void EVSE_C_setOnAuthentication(OnAuthentication_C onAuthenticatn_C);
void EVSE_C_setOnStartTransaction(OnStartTransaction_C onStartTransactn_C);
void EVSE_C_setOnStopTransaction(OnStopTransaction_C onStopTransactn_C);
void EVSE_C_setOnUnauthorizeUser(OnUnauthorizeUser_C onUnauthorizeUsr_C);
void displayMeterValuesC();

void EVSE_C_loop();
//void emergencyRelayClose_Loop();
void emergencyRelayClose_Loop_C();

void EVSE_C_getChargePointSerialNumber(String &out);
char *EVSE_C_getChargePointVendor();
char *EVSE_C_getChargePointModel();

String EVSE_C_getCurrnetIdTag(MFRC522 * mfrc522);
String EVSE_C_readRFID(MFRC522 * mfrc522);
void EVSE_C_stopTransactionByRfid();

void EVSE_C_StartCharging();
void EVSE_C_Suspended();
void EVSE_C_StopSession();

void EVSE_C_startOfflineTxn();
void EVSE_C_offline_Loop();
void EVSE_C_stopOfflineTxn();
void EVSE_C_LED_loop();

#endif