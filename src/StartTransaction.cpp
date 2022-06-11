// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "StartTransaction.h"
#include "TimeHelper.h"
#include "OcppEngine.h"
#include "MeteringService.h"

int globalmeterstartA = 0;
unsigned long st_timeA = 0;
int globalmeterstartB = 0;
unsigned long st_timeB = 0;
int globalmeterstartC = 0;
unsigned long st_timeC = 0;


/*
* @brief: These flags are for display to show only one stop txn.
*/
bool flag_start_txn_A = false;
bool flag_start_txn_B = false;
bool flag_start_txn_C = false;

StartTransaction::StartTransaction() {
	/*if (getChargePointStatusService() != NULL) {
		if (!getChargePointStatusService()->getIdTag().isEmpty()) {
			idTag = String(getChargePointStatusService()->getIdTag());
			connectorId = getChargePointStatusService()->getConnectorId();
		}
	}*/
	//if (idTag.isEmpty()) idTag = String("wrongIDTag"); //Use a default payload. In the typical use case of this library, you probably you don't even need Authorization at all
}

StartTransaction::StartTransaction(String &idTag) {
	this->idTag = String(idTag);
}

StartTransaction::StartTransaction(String &idTag, int &connectorId) {
	this->idTag = String(idTag);
	this->connectorId = connectorId;
}


const char* StartTransaction::getOcppOperationType(){
	return "StartTransaction";
}

DynamicJsonDocument* StartTransaction::createReq() {
	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(5) + (JSONDATE_LENGTH + 1) + (idTag.length() + 1));
	JsonObject payload = doc->to<JsonObject>();

	payload["connectorId"] = connectorId;
	MeteringService* meteringService = getMeteringService();
	if (meteringService != NULL) {
		if(connectorId == 1){
			payload["meterStart"] = meteringService->currentEnergy_A();
			globalmeterstartA = payload["meterStart"];
			flag_start_txn_A = true;
		st_timeA = millis();
		}else if(connectorId ==2){
			payload["meterStart"] = meteringService->currentEnergy_B();
			globalmeterstartB = payload["meterStart"];
			flag_start_txn_B = true;
		st_timeB = millis();
		}else if(connectorId == 3){
			payload["meterStart"] = meteringService->currentEnergy_C();
			globalmeterstartC = payload["meterStart"];
			flag_start_txn_C = true;
		st_timeC = millis();
		}
	}
	char timestamp[JSONDATE_LENGTH + 1] = {'\0'};
	getJsonDateStringFromGivenTime(timestamp, JSONDATE_LENGTH + 1, now());
	payload["timestamp"] = timestamp;
	payload["idTag"] = idTag;

	return doc;
}

void StartTransaction::processConf(JsonObject payload) {

	const char* idTagInfoStatus = payload["idTagInfo"]["status"] | "Invalid";
	int transactionId = payload["transactionId"] | -1;
	//EEPROM.begin(sizeof(EEPROM_Data));
	//EEPROM.put(68, transactionId);
	//EEPROM.commit();
	//EEPROM.end();

	if (!strcmp(idTagInfoStatus, "Accepted")) {
		if (DEBUG_OUT) Serial.print(F("[StartTransaction] Request has been accepted!\n"));

		// ChargePointStatusService *cpStatusService = getChargePointStatusService();
		// if (cpStatusService != NULL){
		// 	cpStatusService->startTransaction(transactionId);
		// 	cpStatusService->startEnergyOffer();
		// }
		
		SmartChargingService *scService = getSmartChargingService();
		if (scService != NULL){
			scService->beginChargingNow();
		}

	} else {
		Serial.print(F("[StartTransaction] Request has been denied!\n"));
	}
}


void StartTransaction::processReq(JsonObject payload) {

	/**
	* Ignore Contents of this Req-message, because this is for debug purposes only
	*/

}

DynamicJsonDocument* StartTransaction::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2));
	JsonObject payload = doc->to<JsonObject>();

	JsonObject idTagInfo = payload.createNestedObject("idTagInfo");
	idTagInfo["status"] = "Accepted";
	payload["transactionId"] = 123456; //sample data for debug purpose

	return doc;
}
