// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "RemoteStartTransaction.h"
#include "OcppEngine.h"

extern bool flag_freeze;
extern bool flag_unfreeze;

RemoteStartTransaction::RemoteStartTransaction() {

}

const char* RemoteStartTransaction::getOcppOperationType(){
	return "RemoteStartTransaction";
}

void RemoteStartTransaction::processReq(JsonObject payload) {
	idTag = String(payload["idTag"].as<String>());
	connectorId = payload["connectorId"].as<int>();
	Serial.println("Connector ID: "+ String(connectorId));
	//Serial.println(String(getChargePointStatusService_A()->inferenceStatus());
	if(connectorId == 1 && getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Available){
		#if DISPLAY_ENABLED
		flag_freeze = true;
		#endif
		getChargePointStatusService_A()->authorize(idTag,connectorId);
	}else if(connectorId == 2 && getChargePointStatusService_B()->inferenceStatus() == ChargePointStatus::Available){
		#if DISPLAY_ENABLED
		flag_freeze = true;
		#endif
		getChargePointStatusService_B()->authorize(idTag,connectorId);
	}else if(connectorId ==3 && getChargePointStatusService_C()->inferenceStatus() == ChargePointStatus::Available){
		#if DISPLAY_ENABLED
		flag_freeze = true;
		#endif
		getChargePointStatusService_C()->authorize(idTag,connectorId);
	}else{
		#if DISPLAY_ENABLED
		flag_freeze = false;
		flag_unfreeze = true;
		#endif
		Serial.println(F("Unable to start txn Connector is busy"));
	}
	/*
	if(getChargePointStatusService_A()->transactionId == -1){
		if(getChargePointStatusService_B()->connectorId != connectorId && getChargePointStatusService_C()->connectorId != connectorId){
			getChargePointStatusService_A()->authorize(idTag,connectorId);    //authorizing twice needed to be improvise
		}else{ 
			Serial.println("[A] Unable to start txn as Connector is busy");
		}
	}else if(getChargePointStatusService_B->transactionId == -1){
		if(getChargePointStatusService_A()->connectorId != connectorId && getChargePointStatusService_C()->connectorId != connectorId){
			getChargePointStatusService_B()->authorize(idTag,connectorId);    //authorizing twice needed to be improvise
		}else{
			Serial.println("[B] Unable to start txn as Connector is busy");
		}
	}else if(getChargePointStatusService_C()->transactionId == -1){
		if(getChargePointStatusService_A()->connectorId != connectorId && getChargePointStatusService_B()->connectorId != connectorId){
			getChargePointStatusService_C()->authorize(idTag,connectorId);    //authorizing twice needed to be improvise
		}else{
			Serial.println("[B] Unable to start txn as Connector is busy");
		}
	}else{
		Serial.println("ALL connectors are busy");
	}*/
//	getChargePointStatusService()->authorize(idTag, connectorId);
}

DynamicJsonDocument* RemoteStartTransaction::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();
	payload["status"] = "Accepted";
	return doc;
}

DynamicJsonDocument* RemoteStartTransaction::createReq() {
	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();

	payload["idTag"] = "fefed1d19876";
	payload["connectorId"] = 1;

	return doc;
}

void RemoteStartTransaction::processConf(JsonObject payload){
	String status = payload["status"] | "Invalid";

	if (status.equals("Accepted")) {
		if (DEBUG_OUT) Serial.print(F("[RemoteStartTransaction] Request has been accepted!\n"));
		} else {
			Serial.print(F("[RemoteStartTransaction] Request has been denied!"));
	}
}
