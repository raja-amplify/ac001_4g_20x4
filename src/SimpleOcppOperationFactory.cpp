// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "SimpleOcppOperationFactory.h"

#include "Authorize.h"
#include "BootNotification.h"
#include "GetLocalListVersion.h"
#include "Heartbeat.h"
#include "MeterValues.h"
#include "GetConfiguration.h"
#include "SetChargingProfile.h"
#include "StatusNotification.h"
#include "StartTransaction.h"
#include "StopTransaction.h"
#include "TriggerMessage.h"
#include "RemoteStartTransaction.h"
#include "RemoteStopTransaction.h"
#include "Reset.h"
// #include "SendLocalList.h"

#include "OcppEngine.h"

#include <string.h>

OnReceiveReqListener onAuthorizeRequest;
void setOnAuthorizeRequestListener(OnReceiveReqListener listener){
  onAuthorizeRequest = listener;
}

OnReceiveReqListener onBootNotificationRequest;
void setOnBootNotificationRequestListener(OnReceiveReqListener listener){
  onBootNotificationRequest = listener;
}

OnReceiveReqListener onTargetValuesRequest;
void setOnTargetValuesRequestListener(OnReceiveReqListener listener) {
  onTargetValuesRequest = listener;
}

OnReceiveReqListener onSetChargingProfileRequest;
void setOnSetChargingProfileRequestListener(OnReceiveReqListener listener){
  onSetChargingProfileRequest = listener;
}

OnReceiveReqListener onStartTransactionRequest;
void setOnStartTransactionRequestListener(OnReceiveReqListener listener){
  onStartTransactionRequest = listener;
}

OnReceiveReqListener onTriggerMessageRequest;
void setOnTriggerMessageRequestListener(OnReceiveReqListener listener){
  onTriggerMessageRequest = listener;
}

OnReceiveReqListener onRemoteStartTransactionReceiveRequest;
void setOnRemoteStartTransactionReceiveRequestListener(OnReceiveReqListener listener) {
  onRemoteStartTransactionReceiveRequest = listener;
}

OnSendConfListener onRemoteStartTransactionSendConf;
void setOnRemoteStartTransactionSendConfListener(OnSendConfListener listener){
  onRemoteStartTransactionSendConf = listener;
}

OnReceiveReqListener onRemoteStopTransactionReceiveRequest;
void setOnRemoteStopTransactionReceiveRequestListener(OnReceiveReqListener listener) {
  onRemoteStopTransactionReceiveRequest = listener;
}

OnSendConfListener onRemoteStopTransactionSendConf;
void setOnRemoteStopTransactionSendConfListener(OnSendConfListener listener){
  onRemoteStopTransactionSendConf = listener;
}

OnSendConfListener onResetSendConf;
void setOnResetSendConfListener(OnSendConfListener listener){
  onResetSendConf = listener;
}
/*
OnReceiveReqListener onGetLocalListVersionReceiveRequest;
void setOnGetLocalListVersionReceiveRequestListener(OnReceiveReqListener listener) {
  onGetLocalListVersionReceiveRequest = listener;
}

OnSendConfListener onGetLocalListVersionSendConf;
void setOnGetLocalListVersionSendConfListener(OnSendConfListener listener){
  onGetLocalListVersionSendConf = listener;
}

OnReceiveReqListener onSendLocalListReceiveRequest;
void setOnSendLocalListReceiveRequestListener(OnReceiveReqListener listener) {
  onSendLocalListReceiveRequest = listener;
}

OnSendConfListener onSendLocalListSendConf;
void setOnSendLocalListSendConfListener(OnSendConfListener listener){
  onSendLocalListSendConf = listener;
}
*/

OnReceiveReqListener onGetConfigurationReceiveRequest;
void setOnGetConfigurationReceiveRequestListener(OnReceiveReqListener listener) {
  onGetConfigurationReceiveRequest = listener;
}

OnSendConfListener onGetConfigurationSendConf;
void setOnGetConfigurationSendConfListener(OnSendConfListener listener){
  onGetConfigurationSendConf = listener;
}

OcppOperation* makeFromTriggerMessage(WebSocketsClient *ws, JsonObject payload) {

  //int connectorID = payload["connectorId"]; <-- not used in this implementation
  const char *messageType = payload["requestedMessage"];

  if (DEBUG_OUT) {
    Serial.print(F("[SimpleOcppOperationFactory] makeFromTriggerMessage for message type "));
    Serial.print(messageType);
    Serial.print(F("\n"));
  }

  return makeOcppOperation(ws, messageType);
}

OcppOperation *makeFromJson(WebSocketsClient *ws, JsonDocument *json) {
  const char* messageType = (*json)[2];
  return makeOcppOperation(ws, messageType);
}

OcppOperation *makeOcppOperation(WebSocketsClient *ws, const char *messageType) {
  OcppOperation *operation = makeOcppOperation(ws);
  OcppMessage *msg = NULL;

  if (!strcmp(messageType, "Authorize")) {
    msg = new Authorize();
    operation->setOnReceiveReqListener(onAuthorizeRequest);
  } else if (!strcmp(messageType, "BootNotification")) {
    msg = new BootNotification();
    operation->setOnReceiveReqListener(onBootNotificationRequest);
  } else if (!strcmp(messageType, "Heartbeat")) {
    msg = new Heartbeat();
  } else if (!strcmp(messageType, "MeterValues")) {
    msg = new MeterValues();
  } else if (!strcmp(messageType, "SetChargingProfile")) {
    msg = new SetChargingProfile(getSmartChargingService());
    operation->setOnReceiveReqListener(onSetChargingProfileRequest);
  } else if (!strcmp(messageType, "StatusNotification")) {
    msg = new StatusNotification();
  } else if (!strcmp(messageType, "StartTransaction")) {
    msg = new StartTransaction();
    operation->setOnReceiveReqListener(onStartTransactionRequest);
  } else if (!strcmp(messageType, "StopTransaction")) {
    msg = new StopTransaction();
  } else if (!strcmp(messageType, "TriggerMessage")) {
    msg = new TriggerMessage(ws);
    operation->setOnReceiveReqListener(onTriggerMessageRequest);
  } else if (!strcmp(messageType, "RemoteStartTransaction")) {
    msg = new RemoteStartTransaction();
    operation->setOnReceiveReqListener(onRemoteStartTransactionReceiveRequest);
    if (onRemoteStartTransactionSendConf == NULL) 
      Serial.print(F("[SimpleOcppOperationFactory] Warning: RemoteStartTransaction is without effect when the sendConf listener is not set. Set a listener which initiates the StartTransaction operation.\n"));
    operation->setOnSendConfListener(onRemoteStartTransactionSendConf);
  } else if (!strcmp(messageType, "RemoteStopTransaction")) {
    msg = new RemoteStopTransaction();
    operation->setOnReceiveReqListener(onRemoteStopTransactionReceiveRequest);
    if (onRemoteStopTransactionSendConf == NULL) 
      Serial.print(F("[SimpleOcppOperationFactory] Warning: RemoteStopTransaction is without effect when the sendConf listener is not set. Set a listener which initiates the StopTransaction operation.\n"));
    operation->setOnSendConfListener(onRemoteStopTransactionSendConf);
  } /*else if (!strcmp(messageType, "GetLocalListVersion")) {
    msg = new GetLocalListVersion();
    operation->setOnReceiveReqListener(onGetLocalListVersionReceiveRequest);
    if (onGetLocalListVersionSendConf == NULL) 
      Serial.print(F("[SimpleOcppOperationFactory] Warning: GetLocalListVersion is without effect when the sendConf listener is not set. Set a listener which initiates the GetLocalListVersion operation.\n"));
    operation->setOnSendConfListener(onGetLocalListVersionSendConf);
  } else if (!strcmp(messageType, "SendLocalList")) {
    msg = new SendLocalList();
    operation->setOnReceiveReqListener(onSendLocalListReceiveRequest);
    if (onSendLocalListSendConf == NULL) 
      Serial.print(F("[SimpleOcppOperationFactory] Warning: SendLocalList is without effect when the sendConf listener is not set. Set a listener which initiates the SendLocalList operation.\n"));
    operation->setOnSendConfListener(onSendLocalListSendConf);
  } */else if (!strcmp(messageType, "Reset")) {
    msg = new Reset();
    if (onResetSendConf == NULL)
      Serial.print(F("[SimpleOcppOperationFactory] Warning: Reset is without effect when the sendConf listener is not set. Set a listener which resets your device.\n"));
    operation->setOnSendConfListener(onResetSendConf);
  } else if (!strcmp(messageType, "GetConfiguration")) {
  /**/
    msg = new GetConfiguration();
    if (onGetConfigurationSendConf == NULL)
      Serial.print(F("[SimpleOcppOperationFactory] Warning: GetConfiguration is without effect when the sendConf listener is not set. Set a listener which sends getConfiguration.\n"));
    operation->setOnSendConfListener(onGetConfigurationSendConf);
    /**/
  } else {
    Serial.print(F("[SimpleOcppOperationFactory] Operation not supported"));
      //TODO reply error code
  }

  if (msg == NULL) {
    delete operation;
    return NULL;
  } else {
    operation->setOcppMessage(msg);
    return operation;
  }
}

OcppOperation* makeOcppOperation(WebSocketsClient *ws, OcppMessage *msg){
  if (msg == NULL) {
    Serial.print(F("[SimpleOcppOperationFactory] in makeOcppOperation(webSocket, ocppMessage): ocppMessage is null!\n"));
    return NULL;
  }
  OcppOperation *operation = makeOcppOperation(ws);
  operation->setOcppMessage(msg);
  return operation;
}

OcppOperation* makeOcppOperation(WebSocketsClient *ws){
  OcppOperation *result = new OcppOperation(ws);
  return result;
}
