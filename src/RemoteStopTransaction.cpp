// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "RemoteStopTransaction.h"
#include "OcppEngine.h"

extern bool flag_evseReadIdTag_A;
extern bool flag_evseAuthenticate_A;
extern bool flag_evseStartTransaction_A;
extern bool flag_evRequestsCharge_A;
extern bool flag_evseStopTransaction_A;
extern bool flag_evseUnauthorise_A;

extern bool flag_evseReadIdTag_B;
extern bool flag_evseAuthenticate_B;
extern bool flag_evseStartTransaction_B;
extern bool flag_evRequestsCharge_B;
extern bool flag_evseStopTransaction_B;
extern bool flag_evseUnauthorise_B;

extern bool flag_evseReadIdTag_C;
extern bool flag_evseAuthenticate_C;
extern bool flag_evseStartTransaction_C;
extern bool flag_evRequestsCharge_C;
extern bool flag_evseStopTransaction_C;
extern bool flag_evseUnauthorise_C;

RemoteStopTransaction::RemoteStopTransaction() {

}

const char* RemoteStopTransaction::getOcppOperationType(){
	return "RemoteStopTransaction";
}


// sample message: [2,"9f639cdf-8a81-406c-a77e-60dff3cb93eb","RemoteStopTransaction",{"transactionId":2042}]
void RemoteStopTransaction::processReq(JsonObject payload) {
	transactionId = payload["transactionId"];

	if(transactionId == getChargePointStatusService_A()-> getTransactionId()){
		flag_evseReadIdTag_A = false;
		flag_evseAuthenticate_A = false;
		flag_evseStartTransaction_A = false;
		flag_evRequestsCharge_A = false;
		flag_evseStopTransaction_A = true;
		flag_evseUnauthorise_A = false;
	}else if(transactionId == getChargePointStatusService_B()->getTransactionId()){
		flag_evseReadIdTag_B = false;
		flag_evseAuthenticate_B = false;
		flag_evseStartTransaction_B = false;
		flag_evRequestsCharge_B = false;
		flag_evseStopTransaction_B = true;
		flag_evseUnauthorise_B = false;
	}else if(transactionId == getChargePointStatusService_C()->getTransactionId()){
		flag_evseReadIdTag_C = false;
		flag_evseAuthenticate_C = false;
		flag_evseStartTransaction_C = false;
		flag_evRequestsCharge_C = false;
		flag_evseStopTransaction_C = true;
		flag_evseUnauthorise_C = false;
	}else{
		Serial.println(F("****************************"));
		Serial.println(F("Transaction ID is incorrect"));
		Serial.println(F("Running Transaction Id: "));
		Serial.println(String(getChargePointStatusService_A()->getTransactionId()));
		Serial.println(String(getChargePointStatusService_B()->getTransactionId()));
		Serial.println(String(getChargePointStatusService_C()->getTransactionId()));
		Serial.println(F("****************************"));
	}

/*	if (transactionId == getChargePointStatusService()->getTransactionId()){
		flag_evseReadIdTag = false;
		flag_evseAuthenticate = false;
		flag_evseStartTransaction = false;
		flag_evRequestsCharge = false;
		flag_evseStopTransaction = true;
		flag_evseUnauthorise = false;
		//getChargePointStatusService()->stopTransaction();
	}*/
	
}

DynamicJsonDocument* RemoteStopTransaction::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();
	payload["status"] = "Accepted";
	return doc;
}

DynamicJsonDocument* RemoteStopTransaction::createReq() {
	DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();

	payload["idTag"] = "fefed1d19876";

	return doc;
}

void RemoteStopTransaction::processConf(JsonObject payload){
	String status = payload["status"] | "Invalid";

	if (status.equals("Accepted")) {
		if (DEBUG_OUT) Serial.print(F("[RemoteStopTransaction] Request has been accepted!\n"));
		} else {
			Serial.print(F("[RemoteStopTransaction] Request has been denied!"));
	}
}
