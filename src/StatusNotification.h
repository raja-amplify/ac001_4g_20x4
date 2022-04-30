// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef STATUSNOTIFICATION_H
#define STATUSNOTIFICATION_H

#include <EEPROM.h>

#include "OcppMessage.h"
#include "OcppEngine.h"
#include "ChargePointStatusService.h"
#include "TimeHelper.h"
#include "EEPROMLayout.h"

class StatusNotification : public OcppMessage {
private:
	ChargePointStatus currentStatus = ChargePointStatus::NOT_SET;
	int connectorId = 1;
	char timestamp[JSONDATE_LENGTH + 1] = {'\0'};
public:
	StatusNotification(ChargePointStatus currentStatus,int connectorId);

	StatusNotification();

	const char* getOcppOperationType();

	DynamicJsonDocument* createReq();

	void processConf(JsonObject payload);

	void processReq(JsonObject payload);

	DynamicJsonDocument* createConf();
};

#endif
