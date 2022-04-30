// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef HEARTBEAT_H
#define HEARTBEAT_H

#include "Variants.h"

#include "OcppMessage.h"

class Heartbeat : public OcppMessage {
public:
  Heartbeat();

  const char* getOcppOperationType();

  DynamicJsonDocument* createReq();

  void processConf(JsonObject payload);

  void processReq(JsonObject payload);

  DynamicJsonDocument* createConf();
};

#endif
