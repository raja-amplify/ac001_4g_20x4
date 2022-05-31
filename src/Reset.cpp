// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "Reset.h"
#include "OcppEngine.h"
#include "EVSE_A.h"

// extern bool flag_rebootRequired;
// extern bool flag_evseIsBooted;
// extern bool flag_evseReadIdTag;
// extern bool flag_evseAuthenticate;
// extern bool flag_evseStartTransaction;
// extern bool flag_evRequestsCharge;
// extern bool flag_evseStopTransaction;
// extern bool flag_evseUnauthorise;
// extern bool flag_evseSoftReset; //added new flag @Wamique


extern bool flag_evseReadIdTag_A;
extern bool flag_evseAuthenticate_A;
extern bool flag_evseStartTransaction_A;
extern bool flag_evRequestsCharge_A;
extern bool flag_evseStopTransaction_A;
extern bool flag_evseUnauthorise_A;
extern bool flag_evseSoftReset_A;
extern bool flag_rebootRequired_A;

extern bool flag_evseReadIdTag_B;
extern bool flag_evseAuthenticate_B;
extern bool flag_evseStartTransaction_B;
extern bool flag_evRequestsCharge_B;
extern bool flag_evseStopTransaction_B;
extern bool flag_evseUnauthorise_B;
extern bool flag_evseSoftReset_B;
extern bool flag_rebootRequired_B;

extern bool flag_evseReadIdTag_C;
extern bool flag_evseAuthenticate_C;
extern bool flag_evseStartTransaction_C;
extern bool flag_evRequestsCharge_C;
extern bool flag_evseStopTransaction_C;
extern bool flag_evseUnauthorise_C;
extern bool flag_evseSoftReset_C;
extern bool flag_rebootRequired_C;

extern uint8_t reasonForStop_A;
extern uint8_t reasonForStop_B;
extern uint8_t reasonForStop_C;

Reset::Reset() {

}

const char* Reset::getOcppOperationType(){
	return "Reset";
}

void Reset::processReq(JsonObject payload) {
	/*
	* Process the application data here. Note: you have to implement the device reset procedure in your client code. You have to set
	* a onSendConfListener in which you initiate a reset (e.g. calling ESP.reset() )
	*/
	const char *type = payload["type"] | "Invalid";
	if (!strcmp(type, "Hard")){
		Serial.print(F("[Reset] Warning: received request to perform hard reset, but this implementation is only capable of soft reset!\n"));
		//Hard_Reset(); To be implemented
		softReset();
		reasonForStop_A = 2;
		reasonForStop_B = 2;
		reasonForStop_C = 2;

	} else if (!strcmp(type, "Soft")){
		if(DEBUG_OUT) Serial.println(F("Soft Reset is Requested"));
		softReset();
		reasonForStop_A = 8;
		reasonForStop_B = 8;
		reasonForStop_C = 8;
	}
}

DynamicJsonDocument* Reset::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();
	payload["status"] = "Accepted";
	return doc;
}



/**********New Function added @Wamique****************/

void softReset(){
/*
	ChargePointStatus inferencedStatus = getChargePointStatusService()->inferenceStatus();
	if(inferencedStatus == ChargePointStatus::Preparing ||
	inferencedStatus == ChargePointStatus::SuspendedEVSE ||
	inferencedStatus == ChargePointStatus::SuspendedEV ||
	inferencedStatus == ChargePointStatus::Charging ){
		if(DEBUG_OUT) Serial.println("Current Status::Charging/Preparing/SuspendedEV");
		flag_evseReadIdTag = false;
		flag_evseAuthenticate = false;
		flag_evseStartTransaction = false;
		flag_evRequestsCharge = false;
		flag_evseStopTransaction = true;
		flag_evseUnauthorise = false;
		flag_evseSoftReset = true;
		flag_rebootRequired = false;
	} else if(inferencedStatus == ChargePointStatus::Available){
		flag_evseReadIdTag = false;
		flag_evseAuthenticate = false;
		flag_evseStartTransaction = false;
		flag_evRequestsCharge = false;
		flag_evseStopTransaction = false;
		flag_evseUnauthorise = false;
		flag_rebootRequired = true;
		if(DEBUG_OUT) Serial.println("Current::Available");
	}*/

	ChargePointStatus inferencedStatus_A = getChargePointStatusService_A()->inferenceStatus();
	ChargePointStatus inferencedStatus_B = getChargePointStatusService_B()->inferenceStatus();
	ChargePointStatus inferencedStatus_C = getChargePointStatusService_C()->inferenceStatus();

	if(inferencedStatus_A != ChargePointStatus::Available){
		Serial.println(F("EVSE_A Current status is preparing/charging/suspened"));
		flag_evseReadIdTag_A = false;
		flag_evseAuthenticate_A = false;
		flag_evseStartTransaction_A = false;
		flag_evRequestsCharge_A = false;
		flag_evseStopTransaction_A = true;
		flag_evseUnauthorise_A = false;
		flag_evseSoftReset_A = true;
		flag_rebootRequired_A = false;
	}else{
		flag_evseReadIdTag_A = false;
		flag_evseAuthenticate_A = false;
		flag_evseStartTransaction_A = false;
		flag_evRequestsCharge_A = false;
		flag_evseStopTransaction_A = false;
		flag_evseUnauthorise_A = false;
		flag_rebootRequired_A = true;
		if(DEBUG_OUT) Serial.println(F("EVSE_A Current status::Available"));
	}

	if(inferencedStatus_B != ChargePointStatus::Available){
		Serial.println(F("EVSE_B Current status is preparing/charging/suspened"));
		flag_evseReadIdTag_B = false;
		flag_evseAuthenticate_B = false;
		flag_evseStartTransaction_B = false;
		flag_evRequestsCharge_B = false;
		flag_evseStopTransaction_B = true;
		flag_evseUnauthorise_B = false;
		flag_evseSoftReset_B = true;
		flag_rebootRequired_B = false;
	}else{
		flag_evseReadIdTag_B = false;
		flag_evseAuthenticate_B = false;
		flag_evseStartTransaction_B = false;
		flag_evRequestsCharge_B = false;
		flag_evseStopTransaction_B = false;
		flag_evseUnauthorise_B = false;
		flag_rebootRequired_B = true;
		if(DEBUG_OUT) Serial.println(F("EVSE_B Current status::Available"));
	}

	if(inferencedStatus_C != ChargePointStatus::Available){
		Serial.println(F("EVSE_C Current status is preparing/charging/suspened"));
		flag_evseReadIdTag_C = false;
		flag_evseAuthenticate_C = false;
		flag_evseStartTransaction_C = false;
		flag_evRequestsCharge_C = false;
		flag_evseStopTransaction_C = true;
		flag_evseUnauthorise_C = false;
		flag_evseSoftReset_C = true;
		flag_rebootRequired_C = false;
	}else{
		flag_evseReadIdTag_C = false;
		flag_evseAuthenticate_C = false;
		flag_evseStartTransaction_C = false;
		flag_evRequestsCharge_C = false;
		flag_evseStopTransaction_C = false;
		flag_evseUnauthorise_C = false;
		flag_rebootRequired_C = true;
		if(DEBUG_OUT) Serial.println(F("EVSE_C Current status::Available"));
	}



}

