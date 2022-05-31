// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "BootNotification.h"
#include "EVSE_A.h"
#include "OcppEngine.h"

#include <string.h>
#include "TimeHelper.h"


BootNotification::BootNotification() {

}

const char* BootNotification::getOcppOperationType(){
	return "BootNotification";
}

DynamicJsonDocument* BootNotification::createReq() {
	String cpSerial = String('\0');
	EVSE_A_getChargePointSerialNumber(cpSerial);

	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(4) + strlen(EVSE_A_getChargePointVendor()) + 1 + cpSerial.length() + 1 + strlen(EVSE_A_getChargePointModel()) + 1);
	JsonObject payload = doc->to<JsonObject>();
	payload["chargePointVendor"] = EVSE_A_getChargePointVendor();
	payload["chargePointSerialNumber"] = cpSerial;
	payload["chargePointModel"] = EVSE_A_getChargePointModel();
	payload["firmwareVersion"] = VERSION;
	return doc;
}

void BootNotification::processConf(JsonObject payload){
	const char* currentTime = payload["currentTime"] | "Invalid";
	if (strcmp(currentTime, "Invalid")) {
		setTimeFromJsonDateString(currentTime);
	} else {
		Serial.print(F("[BootNotification] Error reading time string. Expect format like 2020-02-01T20:53:32.486Z\n"));
	}

	//int interval = payload["interval"] | 86400; //not used in this implementation

	const char* status = payload["status"] | "Invalid";

	if (!strcmp(status, "Accepted")) {
		if (DEBUG_OUT) Serial.print(F("[BootNotification] Request has been accepted!\n"));
		if (getChargePointStatusService_A() != NULL) {
			getChargePointStatusService_A()->boot();
		}
	} else {
		Serial.print(F("[BootNotification] Request unsuccessful!\n"));
	}
}

void BootNotification::processReq(JsonObject payload){
	/*
	* Ignore Contents of this Req-message, because this is for debug purposes only
	*/
}

DynamicJsonDocument* BootNotification::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(3) + (JSONDATE_LENGTH + 1));
	JsonObject payload = doc->to<JsonObject>();
	char currentTime[JSONDATE_LENGTH + 1] = {'\0'};
	getJsonDateStringFromSystemTime(currentTime, JSONDATE_LENGTH);
	payload["currentTime"] =  currentTime; //currentTime
	payload["interval"] = 86400; //heartbeat send interval - not relevant for JSON variant of OCPP so send dummy value that likely won't break
	payload["status"] = "Accepted";
	return doc;
}
