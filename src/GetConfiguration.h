// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef GETCONFIGURATION_H
#define GETCONFIGURATION_H

#include "OcppMessage.h"

class GetConfiguration : public OcppMessage {
private:

public:
  GetConfiguration();

  const char* getOcppOperationType();

  DynamicJsonDocument* createReq();

  void processConf(JsonObject payload);
  
  DynamicJsonDocument* createConf();

};

#endif
