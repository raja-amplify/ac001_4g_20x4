// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "ChargePointStatusService.h"
#include "StatusNotification.h"
#include "OcppEngine.h"
#include "SimpleOcppOperationFactory.h"
#include "LCD_I2C.h"

#include <string.h>

extern bool webSocketConncted;
extern bool wifi_connect;
extern bool gsm_connect;
extern bool isInternetConnected;

extern LCD_I2C lcd;

ChargePointStatusService::ChargePointStatusService(){}  //dummy constructor
 
ChargePointStatusService::ChargePointStatusService(WebSocketsClient *webSocket)
	: webSocket(webSocket){
	setChargePointStatusService(this);
}



ChargePointStatus ChargePointStatusService::inferenceStatus() {
	if (!authorized) {
		if (emergencyRelayClose){
			return ChargePointStatus::Faulted;
		} else {
			return ChargePointStatus::Available;
		}

	} else if (!transactionRunning) {
		return ChargePointStatus::Preparing;
	} else {
		//Transaction is currently running
		if (emergencyRelayClose){
			return ChargePointStatus::Faulted;
		} else {

				if (!evDrawsEnergy) {
					return ChargePointStatus::SuspendedEV;
				}
				if (!evseOffersEnergy) {
					return ChargePointStatus::SuspendedEVSE;
				}
				return ChargePointStatus::Charging;
		}
	} 
}



void ChargePointStatusService::loop() {
	if (DEBUG_OUT) Serial.println(("[ChargePointStatusService] for Connector ID:"+String(connectorId)));

	ChargePointStatus inferencedStatus = inferenceStatus();

	if (inferencedStatus != currentStatus) {
		currentStatus = inferencedStatus;
		#if LCD_ENABLED
        lcd.clear();
 		lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
			lcd.print("****processing****");
		#endif

		 if (!authorized) {

		if (emergencyRelayClose){

			//return ChargePointStatus::Faulted;
			#if LCD_ENABLED
        lcd.clear();
 		lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
			lcd.print("STATUS: FAULTED");
			if(getChargePointStatusService_A()->getOverVoltage() == true){
				lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
			lcd.print("ERROR: OVERVOLTAGE");
	}else if(getChargePointStatusService_A()->getUnderVoltage() == true){
		if(eic.GetLineVoltageA() < 170 && eic.GetLineVoltageA() > 50){
			lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
			lcd.print("ERROR: NO EARTH");
			
		}else{
			lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
			lcd.print("ERROR: U.V TRANSIENT");
		}

	}else if(getChargePointStatusService_A()->getUnderCurrent() == true){
		lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
			lcd.print("ERROR: UNDERCURRENT");

	}else if(getChargePointStatusService_A()->getOverCurrent() == true){
		
lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
			lcd.print("ERROR: OVERCURRENT");
	}else if(getChargePointStatusService_A()->getUnderTemperature() == true){
		lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
			lcd.print("ERROR: OVERTEMP");

	}else if(getChargePointStatusService_A()->getOverTemperature() == true){
		lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
			lcd.print("ERROR: UNDERTEMP");
	}
	else if(emergencyRelayClose){
		lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
			lcd.print("ERROR: EMERGENCY");
	}
	
	#endif

		} else {

			//return ChargePointStatus::Available;
			#if LCD_ENABLED
        lcd.clear();
 		lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
			lcd.print("STATUS: AVAILABLE");
			lcd.setCursor(0, 1);
			lcd.print("TAP RFID/SCAN QR");
			lcd.setCursor(0, 2);
			lcd.print("CONNECTION");
			lcd.setCursor(0, 3);
			 if(wifi_connect)
			 lcd.print("cloud: wifi");
            else if(gsm_connect)
			lcd.print("cloud: 4G");
			else
			lcd.print("cloud: offline");
			#endif
			

		}

	} else if (!transactionRunning) {
		
	
		//return ChargePointStatus::Preparing;
	} else {
		#if LCD_ENABLED
		//lcd.setCursor(0, 0);
		//Transaction is currently running
		if (emergencyRelayClose){
			//return ChargePointStatus::Faulted;
			//lcd.print("STATUS: FAULTED");
		} else {
				
				if (!evDrawsEnergy) {
					//return ChargePointStatus::SuspendedEV;
					lcd.clear();
 					lcd.setCursor(0, 0);
					lcd.print("STATUS: SUSPENDEDEV");
					lcd.setCursor(0, 1);
					lcd.print("KWH:");
					lcd.setCursor(5, 1);
					lcd.print("");
					lcd.setCursor(0, 2);
					lcd.print("TIME");
					lcd.setCursor(5, 1);
					lcd.print("");
				}
				if (!evseOffersEnergy) {
					//return ChargePointStatus::SuspendedEVSE;
					lcd.clear();
 					lcd.setCursor(0, 0);
					lcd.print("STATUS: SUSPENDEDEVSE");
				}
				#endif
				//return ChargePointStatus::Charging;
				/*lcd.clear();
 				lcd.setCursor(0, 0);
				lcd.print("STATUS: CHARGING");*/
		}
	} 

		#if DISPLAY_ENABLED
		displayStatus((int)currentStatus);
		#endif
		if (DEBUG_OUT) Serial.print(("[ChargePointStatusService] Status changed for Connector ID:"+String(connectorId)));

		//fire StatusNotification
		//TODO check for online condition: Only inform CS about status change if CP is online
		//TODO check for too short duration condition: Only inform CS about status change if it lasted for longer than MinimumStatusDuration
		OcppOperation *statusNotification = makeOcppOperation(webSocket,
		new StatusNotification(currentStatus, connectorId));
		initiateOcppOperation(statusNotification);
	}
}

void ChargePointStatusService::authorize(String &idTag, int connectorId){
	this->idTag = String(idTag);
	this->connectorId = connectorId;
	authorize();
}

void ChargePointStatusService::authorize(String &idTag){
	this->idTag = String(idTag);
	authorize();
}

void ChargePointStatusService::authorize(){
	if (authorized == true){
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: authorized twice or didn't unauthorize before\n"));
		return;
	}
	authorized = true;
}

int &ChargePointStatusService::getConnectorId() {
	return connectorId;
}

 void ChargePointStatusService::setConnectorId(int connectorId) {
 	this->connectorId = connectorId;
 }
String &ChargePointStatusService::getIdTag() {
	return idTag;
}

bool ChargePointStatusService::getOverVoltage(){
	return overVoltage;
}

void ChargePointStatusService::setOverVoltage(bool ov){
	this->overVoltage = ov;
}

bool ChargePointStatusService::getUnderVoltage(){
	return underVoltage;
}

void ChargePointStatusService::setUnderVoltage(bool uv){
	this->underVoltage = uv;
}

bool ChargePointStatusService::getOverTemperature(){
	return overTemperature;
}

void ChargePointStatusService::setOverTemperature(bool ot){
	this->overTemperature = ot;
}

bool ChargePointStatusService::getUnderTemperature(){
	return underTemperature;
}

void ChargePointStatusService::setUnderTemperature(bool ut){
	this->underTemperature = ut;
}

bool ChargePointStatusService::getOverCurrent(){
	return overCurrent;
}

void ChargePointStatusService::setOverCurrent(bool oc){
	this->overCurrent = oc;
}

bool ChargePointStatusService::getUnderCurrent(){
	return underCurrent;
}

void ChargePointStatusService::setUnderCurrent(bool uc){
	this->underCurrent = uc;
}

bool ChargePointStatusService::getEmergencyRelayClose(){
	return emergencyRelayClose;
}
void ChargePointStatusService::setEmergencyRelayClose(bool erc){
	this->emergencyRelayClose = erc;
}


void ChargePointStatusService::unauthorize(){
	if (authorized == false){
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: unauthorized twice or didn't authorize before\n"));
		return;
	}
	if(DEBUG_OUT) Serial.print(F("[ChargePointStatusService] idTag value will be cleared.\n"));
	this->idTag.clear();
	authorized = false;
}

void ChargePointStatusService::startTransaction(int transId){
	if (transactionRunning == true){
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: started transaction twice or didn't stop transaction before\n"));
	}
	transactionId = transId;
	transactionRunning = true;
}

void ChargePointStatusService::boot() {
	//TODO Review: Is it necessary to check in inferenceStatus(), if the CP is booted at all? Problably not ...
}

void ChargePointStatusService::stopTransaction(){
	if (transactionRunning == false){
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: stopped transaction twice or didn't start transaction before\n"));
	}
	transactionRunning = false;
	transactionId = -1;
	//EEPROM.begin(sizeof(EEPROM_Data));
	//EEPROM.put(68,transactionId);
	//EEPROM.commit();
	//EEPROM.end();
}

int ChargePointStatusService::getTransactionId() {
	if (transactionId < 0) {
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: getTransactionId() returns invalid transactionId. Have you called it after stopTransaction()? (can only be called before) Have you called it before startTransaction?\n"));
	}
	if (transactionRunning == false) {
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: getTransactionId() called, but there is no transaction running. Have you called it after stopTransaction()? (can only be called before) Have you called it before startTransaction?\n"));
	}
	return transactionId;
}

void ChargePointStatusService::startEvDrawsEnergy(){
	if (evDrawsEnergy == true){
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: startEvDrawsEnergy called twice or didn't call stopEvDrawsEnergy before\n"));
	}
	evDrawsEnergy = true;
}

void ChargePointStatusService::stopEvDrawsEnergy(){
	if (evDrawsEnergy == false){
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: stopEvDrawsEnergy called twice or didn't call startEvDrawsEnergy before\n"));
	}
	evDrawsEnergy = false;
}
void ChargePointStatusService::startEnergyOffer(){
	if (evseOffersEnergy == true){
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: startEnergyOffer called twice or didn't call stopEnergyOffer before\n"));
	}
	evseOffersEnergy = true;
}

void ChargePointStatusService::stopEnergyOffer(){
	if (evseOffersEnergy == false){
		if (DEBUG_OUT) Serial.print(F("[ChargePointStatusService] Warning: stopEnergyOffer called twice or didn't call startEnergyOffer before\n"));
	}
	evseOffersEnergy = false;
}

/*****Added new Definition @wamique***********/
bool ChargePointStatusService::getEvDrawsEnergy(){
	return evDrawsEnergy;
}

/*********************************************/