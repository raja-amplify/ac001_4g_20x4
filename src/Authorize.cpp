// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Authorize.h"
#include "OcppEngine.h"
#include "Variants.h"

#if DISPLAY_ENABLED
extern bool flag_freeze;
#endif

Authorize::Authorize() {
	idTag = String("defaultCPIDTag"); //Use a default payload. In the typical use case of this library, you probably you don't even need Authorization at all
}

Authorize::Authorize(String &idTag) {
	this->idTag = String(idTag);
//	EEPROM.begin(sizeof(EEPROM_Data));
//	EEPROM.put(28,idTag);
//	EEPROM.commit();
//	EEPROM.end();
}

const char* Authorize::getOcppOperationType(){
	return "Authorize";
}

DynamicJsonDocument* Authorize::createReq() {
	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1) + (idTag.length() + 1));
	JsonObject payload = doc->to<JsonObject>();
	payload["idTag"] = idTag;
	return doc;
}

void Authorize::processConf(JsonObject payload){
	String idTagInfo = payload["idTagInfo"]["status"] | "Invalid";

	if (idTagInfo.equals("Accepted")) {
		if (DEBUG_OUT) Serial.print(F("[Authorize] Request has been accepted!\n"));

		// ChargePointStatusService *cpStatusService = getChargePointStatusService();
		// if (cpStatusService != NULL){
		// 	cpStatusService->authorize();
		// }

	} else {
		Serial.print(F("[Authorize] Request has been denied!"));
		#if DISPLAY_ENABLED
		flag_freeze = false;
		#endif
	}
}

void Authorize::processReq(JsonObject payload){
	/*
	* Ignore Contents of this Req-message, because this is for debug purposes only
	*/
}

DynamicJsonDocument* Authorize::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(2 * JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();
	JsonObject idTagInfo = payload.createNestedObject("idTagInfo");
	idTagInfo["status"] = "Accepted";
	return doc;
}
