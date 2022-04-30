// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef GETLOCALLISTVERSION_H
#define GETLOCALLISTVERSION_H

#include "OcppMessage.h"

class GetLocalListVersion : public OcppMessage {
private:
	int localListVersionNumber;
public:
	GetLocalListVersion();

	const char* getOcppOperationType();

	/*DynamicJsonDocument* createReq();*/

	/*void processConf(JsonObject payload);*/

	void processReq(JsonObject payload);

	DynamicJsonDocument* createConf();

};

#endif
