// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "SendLocalList.h"
#include "OcppEngine.h"
#include "Variants.h"

SendLocalList::SendLocalList() {
	
}


const char* SendLocalList::getOcppOperationType(){
	return "SendLocalList";
}

DynamicJsonDocument* SendLocalList::createReq() {
	
}

void SendLocalList::processConf(JsonObject payload){
	
	
	
}

void SendLocalList::processReq(JsonObject payload){
	
	//listVersion = payload["listVersion"];
	//localAuthorizationList[100] = payload["localAuthorizationList"];
	
}

DynamicJsonDocument* SendLocalList::createConf(){
	DynamicJsonDocument* doc = new DynamicJsonDocument(2 * JSON_OBJECT_SIZE(1));
	JsonObject payload = doc->to<JsonObject>();
	payload["status"] = "Accepted";
	//JsonObject idTagInfo = payload.createNestedObject("idTagInfo");
	//idTagInfo["status"] = "Accepted";
	return doc;
}
