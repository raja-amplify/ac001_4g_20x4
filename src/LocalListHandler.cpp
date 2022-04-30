// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "LocalListHandler.h"

IdTagInfoClass idTagObject[40];//no of Id Tags

void IdTagInfoClass::setIdTagInfo (String _idTag, String _idTagStatus, String _idExpDate){
	this->idTag=_idTag;
    this->idTagStatus=_idTagStatus;
    this->idExpDate=_idExpDate;
}

void IdTagInfoClass::getIdTagInfo(){
	Serial.print("Idtag : ");
	Serial.println(this->idTag);
	Serial.print("IdtagStatus : ");
	Serial.println(this->idTagStatus);
	Serial.print("ExpDate : ");
	Serial.println(this->idExpDate);
}

void IdTagInfoClass::processJson(char *json){

	int i=0;
	DynamicJsonDocument doc(4096);
	deserializeJson(doc, json);

	int messageType = doc[0]; // 2
	const char* transactionID = doc[1]; // "dbf3633b-6cd8-4328-bcc8-de146f1bacef"
	const char* ocppMessageType = doc[2]; // "SendLocalList"
	
	JsonObject localListJson = doc[3];
	int listVersionNumber = localListJson["listVersion"]; // 1
	Serial.print("List Version Number: ");
	Serial.println(listVersionNumber);

	JsonArray localAuthorizationList = localListJson["localAuthorizationList"];

	Serial.println("localAuthorizationList");
	
	//IdTagInfoClass idTagInfo = NULL;
	
	for(i=0; i<localAuthorizationList.size(); i++){
		//Serial.print("\nObjects:");
		//Serial.println(i+1);
		
		//idTagInfoClass = new IdTagInfoClass(localAuthorizationList[i]["idTag"], localAuthorizationList[i]["idTagInfo"]["status"], localAuthorizationList[i]["idTagInfo"]["expiryDate"]);
		//idTagInfoClass.getIdTagInfo();
		idTagObject[i].setIdTagInfo(localAuthorizationList[i]["idTag"], localAuthorizationList[i]["idTagInfo"]["status"], localAuthorizationList[i]["idTagInfo"]["expiryDate"]);
		idTagObject[i].getIdTagInfo();

		}

}

