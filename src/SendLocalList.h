// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef SENDLOCALLIST_H
#define SENDLOCALLIST_H

#include "OcppMessage.h"

class SendLocalList : public OcppMessage {
private:
	String idTag;
public:
	SendLocalList();

	const char* getOcppOperationType();

	DynamicJsonDocument* createReq();

	void processConf(JsonObject payload);

	void processReq(JsonObject payload);

	DynamicJsonDocument* createConf();

};

#endif
