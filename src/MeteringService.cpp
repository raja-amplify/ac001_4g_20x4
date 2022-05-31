// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "MeteringService.h"
#include "OcppOperation.h"
#include "MeterValues.h"
#include "OcppEngine.h"
#include "SimpleOcppOperationFactory.h"
#include "display.h"
#include <EEPROM.h>

#include "LCD_I2C.h"
extern LCD_I2C lcd;

bool flag_MeteringIsInitialised = false;
Preferences energymeter;

MeteringService::MeteringService(WebSocketsClient *webSocket)
      : webSocket(webSocket) {
  
  sampleTimeA 	= LinkedList<time_t>();
  sampleTimeB 	= LinkedList<time_t>();
  sampleTimeC 	= LinkedList<time_t>();
  powerA 		= LinkedList<float>();
  powerB 		= LinkedList<float>();
  powerC 		= LinkedList<float>();
  energyA 		= LinkedList<float>();
  energyB 		= LinkedList<float>();
  energyC 		= LinkedList<float>();
  voltageA 		= LinkedList<float>();
  voltageB 		= LinkedList<float>();
  voltageC 		= LinkedList<float>();
  currentA 		= LinkedList<float>();
  currentB 		= LinkedList<float>();
  currentC 		= LinkedList<float>();
  temperature 	= LinkedList<float>();

  setMeteringSerivce(this); //make MeteringService available through Ocpp Engine

}

void MeteringService::addDataPoint(time_t currentTime, float currentPower, float currentEnergy, float currentVoltage, float currentCurrent, float currentTemperature, int connector){
//	Serial.println("connector: " + String(connector) + "TXID" + String(getChargePointStatusService_A()->getTransactionId()));
	if(connector == 1 && getChargePointStatusService_A()->getTransactionId() == -1){
		lastSampleTimeA = currentTime;

	}else if(connector == 2 && getChargePointStatusService_B()->getTransactionId() == -1){
		lastSampleTimeB = currentTime;

	}else if(connector == 3 && getChargePointStatusService_C()->getTransactionId() == -1){
		lastSampleTimeC = currentTime;

	}else if(connector == 1 && getChargePointStatusService_A() != NULL && getChargePointStatusService_A()->getConnectorId() == 1 && getChargePointStatusService_A()->getEvDrawsEnergy() == true){
		sampleTimeA.add(currentTime);
		powerA.add(currentPower);
		energyA.add(currentEnergy);
		voltageA.add(currentVoltage);
		currentA.add(currentCurrent);
		temperature.add(currentTemperature);
		//phaseA.add(currentPhase);

		lastSampleTimeA = currentTime;

		if (powerA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {

				flushLinkedListValues(connector);
		}

	}else if(connector == 2 && getChargePointStatusService_B() != NULL && getChargePointStatusService_B()->getConnectorId() == 2 && getChargePointStatusService_B()->getEvDrawsEnergy() == true){

		sampleTimeB.add(currentTime);
		powerB.add(currentPower);
		energyB.add(currentEnergy);
		voltageB.add(currentVoltage);
		currentB.add(currentCurrent);
		temperature.add(currentTemperature);
		//phaseB.add(currentPhase);

		lastSampleTimeB = currentTime;
		//lastPower = currentPower;

		/*
		* Check if to send all the meter values to the server
		*/
		if (powerB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {
				
				flushLinkedListValues(connector);
		}



	}else if(connector == 3 && getChargePointStatusService_C() != NULL && getChargePointStatusService_C()->getConnectorId() == 3 && getChargePointStatusService_C()->getEvDrawsEnergy() == true){

		sampleTimeC.add(currentTime);
		powerC.add(currentPower);
		energyC.add(currentEnergy);
		voltageC.add(currentVoltage);
		currentC.add(currentCurrent);
		temperature.add(currentTemperature);
		//phaseC.add(currentPhase);

		lastSampleTimeC = currentTime;
		//lastPower = currentPower;
		/*
		* Check if to send all the meter values to the server
		*/
		if (powerC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {
				
				flushLinkedListValues(connector);
		}
		
	}else{

		Serial.println("[Metering Serivce] Error Occurred Connector ID and chargepoint mismatch");
		Serial.println("Requested Connector ID: "+String(connector));
		Serial.println("chargepointA: "+ String(getChargePointStatusService_A()->getConnectorId()));
		Serial.println("chargepointB: "+ String(getChargePointStatusService_B()->getConnectorId()));
		Serial.println("chargepointC: "+ String(getChargePointStatusService_C()->getConnectorId()));

	}
}

#if 0
void MeteringService::addDataPoint(time_t currentTime, float currentPower, float currentEnergy, float currentVoltage, float currentCurrent, float currentTemperature, int connector){
	

	if(connector == 1 && getChargePointStatusService_A() != NULL && getChargePointStatusService_A()->getConnectorId() == 1){
		sampleTimeA.add(currentTime);
		powerA.add(currentPower);
		energyA.add(currentEnergy);
		voltageA.add(currentVoltage);
		currentA.add(currentCurrent);
		temperature.add(currentTemperature);

		lastSampleTimeA = currentTime;


		if (powerA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {

				flushLinkedListValues(connector);
		}

	}else if(connector == 2 && getChargePointStatusService_B() != NULL && getChargePointStatusService_B()->getConnectorId() == 2){

		sampleTimeB.add(currentTime);
		powerB.add(currentPower);
		energyB.add(currentEnergy);
		voltageB.add(currentVoltage);
		currentB.add(currentCurrent);
		temperature.add(currentTemperature);

		lastSampleTimeB = currentTime;
		//lastPower = currentPower;

		/*
		* Check if to send all the meter values to the server
		*/
		if (powerB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {
				
				flushLinkedListValues(connector);
		}



	}else if(connector == 3 && getChargePointStatusService_C() != NULL && getChargePointStatusService_C()->getConnectorId() == 3){

		sampleTimeC.add(currentTime);
		powerC.add(currentPower);
		energyC.add(currentEnergy);
		voltageC.add(currentVoltage);
		currentC.add(currentCurrent);
		temperature.add(currentTemperature);

		lastSampleTimeC = currentTime;
		//lastPower = currentPower;

		/*
		* Check if to send all the meter values to the server
		*/
		if (powerC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {
				
				flushLinkedListValues(connector);
		}
		


	}else{

		Serial.println("[Metering Serivce] Error Occurred Connector ID and chargepoint mismatch");
		Serial.println("Requested Connector ID: "+String(connector));
		Serial.println("chargepointA: "+ String(getChargePointStatusService_A()->getConnectorId()));
		Serial.println("chargepointB: "+ String(getChargePointStatusService_B()->getConnectorId()));
		Serial.println("chargepointC: "+ String(getChargePointStatusService_C()->getConnectorId()));

	}


}
#endif
/*

	if (getChargePointStatusService_A() != NULL && getChargePointStatusService_A()->getTransactionId() == -1){  // for time being
		sampleTimeA.add(currentTime);
		power.add(currentPower);
		energyA.add(currentEnergy);
		voltageA.add(currentVoltage);
		currentA.add(currentCurrent);
		temperature.add(currentTemperature);
		
		//EEPROM.begin(sizeof(EEPROM_Data));
		//EEPROM.put(4, currentEnergy);   
		//EEPROM.commit();
		//EEPROM.end();
		

		lastSampleTimeA = currentTime;
		//lastPower = currentPower;

		
		if (power.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {
		flushLinkedListValues();
		}			
	} else 	if (getChargePointStatusService_A() != NULL && getChargePointStatusService_A()->getConnectorId() == 1){
		sampleTimeA.add(currentTime);
		power.add(currentPower);
		energyA.add(currentEnergy);
		voltageA.add(currentVoltage);
		currentA.add(currentCurrent);
		temperature.add(currentTemperature);
		

		// EEPROM.begin(sizeof(EEPROM_Data));
		// EEPROM.put(4, currentEnergy);   
		// EEPROM.commit();
		// EEPROM.end();
		

		lastSampleTimeA = currentTime;
		//lastPower = currentPower;

		
		if (power.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentA.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {
		flushLinkedListValues();
		}			
	} else if (getChargePointStatusService_B() != NULL && getChargePointStatusService_B()->getConnectorId() == 2){
		sampleTimeB.add(currentTime);
		power.add(currentPower);
		energyB.add(currentEnergy);
		voltageB.add(currentVoltage);
		currentB.add(currentCurrent);
		temperature.add(currentTemperature);

		lastSampleTimeB = currentTime;
		//lastPower = currentPower;

		if (power.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentB.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {
		flushLinkedListValues();
		}			
	} else 	if (getChargePointStatusService_C() != NULL && getChargePointStatusService_C()->getConnectorId() == 3){
		sampleTimeC.add(currentTime);
		power.add(currentPower);
		energyC.add(currentEnergy);
		voltageC.add(currentVoltage);
		currentC.add(currentCurrent);
		temperature.add(currentTemperature);

		lastSampleTimeC = currentTime;
		//lastPower = currentPower;

		
		if (power.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& energyC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& voltageC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& currentC.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH
			&& temperature.size() >= METER_VALUES_SAMPLED_DATA_MAX_LENGTH) {
		flushLinkedListValues();
		}			
	} 
	if (DEBUG_OUT) Serial.println("It is NULL");
}*/


void MeteringService::flushLinkedListValues(int connector) {

	if(connector == 1){

		if(powerA.size() == 0 && energyA.size() == 0 && voltageA.size() == 0 && currentA.size() == 0 && temperature.size() == 0) return;
		
		OcppOperation *meterValues = makeOcppOperation(webSocket,
		new MeterValues(connector, &sampleTimeA, &powerA, &energyA, &voltageA, &currentA, &temperature)); //have to pass connector
		initiateOcppOperation(meterValues);
		sampleTimeA.clear();
		powerA.clear();
		energyA.clear();
		voltageA.clear();
		currentA.clear();
		temperature.clear();
	
	}else if(connector == 2){

		if(powerB.size() == 0 && energyB.size() == 0 && voltageB.size() == 0 && currentB.size() == 0 && temperature.size() == 0) return;

		OcppOperation *meterValues = makeOcppOperation(webSocket,
		new MeterValues(connector, &sampleTimeB, &powerB, &energyB, &voltageB, &currentB, &temperature)); //have to pass connector
		initiateOcppOperation(meterValues);
		sampleTimeB.clear();
		powerB.clear();
		energyB.clear();
		voltageB.clear();
		currentB.clear();
		temperature.clear();
	
	}else if(connector == 3){

		if(powerC.size() == 0 && energyC.size() == 0 && voltageC.size() == 0 && currentC.size() == 0 && temperature.size() == 0) return;

		OcppOperation *meterValues = makeOcppOperation(webSocket,
		new MeterValues(connector, &sampleTimeC, &powerC, &energyC, &voltageC, &currentC, &temperature));
		initiateOcppOperation(meterValues);
		sampleTimeC.clear();
		powerC.clear();
		energyC.clear();
		voltageC.clear();
		currentC.clear();
		temperature.clear();

	}else{

		Serial.println("[MeteringService] Error in Flushed link list");
	}

} /*

  if (getChargePointStatusService_A() != NULL) {   //for time being
    if (getChargePointStatusService_A()->getTransactionId() == -1){ //for time being
		if (power.size() == 0 && energyA.size() == 0 && voltageA.size() == 0 && currentA.size() == 0 && temperature.size() == 0) return; //Nothing to report

		OcppOperation *meterValues = makeOcppOperation(webSocket,
		new MeterValues(&sampleTimeA, &power, &energyA, &voltageA, &currentA, &temperature));
		initiateOcppOperation(meterValues);
		sampleTimeA.clear();
		power.clear();
		energyA.clear();
		voltageA.clear();
		currentA.clear();
		temperature.clear();
	} else if (getChargePointStatusService_A()->getConnectorId() == 1){
		if (power.size() == 0 && energyA.size() == 0 && voltageA.size() == 0 && currentA.size() == 0 && temperature.size() == 0) return; //Nothing to report

		if (getChargePointStatusService_A()->getTransactionId() < 0) {      
		  sampleTimeA.clear();
		  power.clear();
		  energyA.clear();
		  voltageA.clear();
		  currentA.clear();
		  temperature.clear();
		  return;
		}
		OcppOperation *meterValues = makeOcppOperation(webSocket,
		new MeterValues(&sampleTimeA, &power, &energyA, &voltageA, &currentA, &temperature));
		initiateOcppOperation(meterValues);
		sampleTimeA.clear();
		power.clear();
		energyA.clear();
		voltageA.clear();
		currentA.clear();
		temperature.clear();
	} else if (getChargePointStatusService_B()->getConnectorId() == 2){
		if (power.size() == 0 && energyB.size() == 0 && voltageB.size() == 0 && currentB.size() == 0 && temperature.size() == 0) return; //Nothing to report
		if (getChargePointStatusService_B()->getTransactionId() < 0) {      
		  sampleTimeB.clear();
		  power.clear();
		  energyB.clear();
		  voltageB.clear();
		  currentB.clear();
		  temperature.clear();
		  return;
		}	
		OcppOperation *meterValues = makeOcppOperation(webSocket,
		new MeterValues(&sampleTimeB, &power, &energyB, &voltageB, &currentB, &temperature));
		initiateOcppOperation(meterValues);
		sampleTimeB.clear();
		power.clear();
		energyB.clear();
		voltageB.clear();
		currentB.clear();
		temperature.clear();
		
	} else if (getChargePointStatusService_C()->getConnectorId() == 3){
		if (power.size() == 0 && energyC.size() == 0 && voltageC.size() == 0 && currentC.size() == 0 && temperature.size() == 0) return; //Nothing to report
		if (getChargePointStatusService_C()->getTransactionId() < 0) {      
		  sampleTimeC.clear();
		  power.clear();
		  energyC.clear();
		  voltageC.clear();
		  currentC.clear();
		  temperature.clear();
		  return;
		}
		OcppOperation *meterValues = makeOcppOperation(webSocket,
		new MeterValues(&sampleTimeC, &power, &energyC, &voltageC, &currentC, &temperature));
		initiateOcppOperation(meterValues);
		sampleTimeC.clear();
		power.clear();
		energyC.clear();
		voltageC.clear();
		currentC.clear();
		temperature.clear();
		
	}
  }
}
*/

void MeteringService::loop(){

  /*
   * Calculate energy consumption which finally should be reportet to the Central Station in a MeterValues.req.
   * This code uses the EVSE's own energy register, if available (i.e. if energySampler is set). Otherwise it
   * uses the power sampler.
   * If no powerSampler is available, estimate the energy consumption taking the Charging Schedule and CP Status
   * into account.
   */
   
   
   
   //@bug: fix it for multiple connectors.
   if (DEBUG_OUT_M) Serial.println(F("The last sample time is: "));
   if (DEBUG_OUT_M) Serial.println(lastSampleTimeA + METER_VALUE_SAMPLE_INTERVAL);
   if (DEBUG_OUT_M) Serial.println(F("The now() is: "));
   if (DEBUG_OUT_M) Serial.println(now());
   
   if ((now() >= (time_t) METER_VALUE_SAMPLE_INTERVAL + lastSampleTimeA)) {
	  if (energyASampler != NULL 
		&& powerASampler != NULL
		&& voltageASampler != NULL
		&& currentASampler != NULL
		&& temperatureSampler != NULL){

			time_t sampledTimeA = now();
			time_t deltaA = sampledTimeA - lastSampleTimeA;
			float sampledVoltage_A = voltageASampler();			
			float sampledCurrent_A = currentASampler();
			float sampledPower_A   = powerASampler(sampledVoltage_A, sampledCurrent_A);			
			float sampledEnergy_A  = energyASampler(sampledVoltage_A, sampledCurrent_A, deltaA);
			float sampledTemperature = temperatureSampler();
			addDataPoint(sampledTimeA, sampledPower_A, sampledEnergy_A, sampledVoltage_A, sampledCurrent_A, sampledTemperature,1); //1 = conector ID


			time_t sampledTimeB = now();
			time_t deltaB = sampledTimeB - lastSampleTimeB;
			float sampledVoltage_B = voltageBSampler();
			float sampledCurrent_B = currentBSampler();
			float sampledPower_B   = powerBSampler(sampledVoltage_B, sampledCurrent_B);
			float sampledEnergy_B  = energyBSampler(sampledVoltage_B, sampledCurrent_B, deltaB);
			addDataPoint(sampledTimeB, sampledPower_B, sampledEnergy_B, sampledVoltage_B, sampledCurrent_B, sampledTemperature,2); //1 = conector ID

			
			time_t sampledTimeC = now();
			time_t deltaC = sampledTimeC - lastSampleTimeC;
			float sampledVoltage_C = voltageCSampler();
			float sampledCurrent_C = currentCSampler();
			float sampledPower_C   = powerCSampler(sampledVoltage_C, sampledCurrent_C);
			float sampledEnergy_C  = energyCSampler(sampledVoltage_C, sampledCurrent_C, deltaC);		
			addDataPoint(sampledTimeC, sampledPower_C, sampledEnergy_C, sampledVoltage_C, sampledCurrent_C, sampledTemperature,3); //1 = conector ID
			if (DEBUG_OUT_M) Serial.println(F("Inside if cond."));
			//addDataPoint(sampledTimeA, 0, 0,sampledVoltage, sampledCurrent, sampledTemperature);
		}
	}
}



void MeteringService::init(MeteringService *meteringService){
  
  flag_MeteringIsInitialised = true;
	
  meteringService->lastSampleTimeA = now(); //0 means not charging right now
  meteringService->lastSampleTimeB = now(); //0 means not charging right now
  meteringService->lastSampleTimeC = now(); //0 means not charging right now
  
   /*
   * initialize EEPROM
   */
  int isInitialized;
  float currEnergy_A;
  float currEnergy_B;
  float currEnergy_C;

//Replacing EEROM implementation with preferences
  energymeter.begin("MeterData",false);                //opening nmespace with R/W access
  isInitialized = energymeter.getInt("isInitialized",0);
  Serial.println("[Metering init] isInitialized: "+ String(isInitialized));

  currEnergy_A = energymeter.getFloat("currEnergy_A",0);
  Serial.println("[Metering init] currEnergy_A: "+ String(currEnergy_A));

  currEnergy_B = energymeter.getFloat("currEnergy_B",0);
  Serial.println("[Metering init] currEnergy_B: "+ String(currEnergy_B));

  currEnergy_C = energymeter.getFloat("currEnergy_C",0);
  Serial.println("[Metering init] currEnergy_C: " + String(currEnergy_C));

  if(isInitialized == 22121996){
  	if(currEnergy_A > 1000000){
  		energymeter.putFloat("currEnergy_A",0);
  		Serial.println("[Metering init] Reinitialized currEnergy_A");
  	}

  	if(currEnergy_B > 1000000){
  		energymeter.putFloat("currEnergy_B",0);
  		Serial.println("[Metering init] Reinitialized currEnergy_B");
  	}

  	if(currEnergy_C > 1000000){
  		energymeter.putFloat("currEnergy_C",0);
  		Serial.println("[Metering init] Reinitialized currEnergy_C");
  	}
  }else if(isInitialized != 22121996){

  	energymeter.putInt("isInitialized",22121996);
  	energymeter.putFloat("currEnergy_A",0);
  	energymeter.putFloat("currEnergy_B",0);
  	energymeter.putFloat("currEnergy_C",0);

  	Serial.println("[Metering init] Initialized Meter Data");
  }

/*
  EEPROM.begin(sizeof(EEPROM_Data));
  EEPROM.get(0,isInitialized);
  EEPROM.get(4,currEnergy);
  if (isInitialized == 22121996){
  	if (currEnergy > 100000){
	  	EEPROM.put(0, 0);
	  	EEPROM.put(4, 0.0f);
  	}
	
  } else if (isInitialized != 22121996){   //hove to correct
	  isInitialized = 22121996;
	  EEPROM.put(0, isInitialized);
	  EEPROM.put(4, 0.0f);
  }
  
  EEPROM.commit();
  EEPROM.end();

  Serial.println("[Mertering init] isInitialized "+ String(isInitialized));
  Serial.println("[Mertering init] current energy: "+ String(currEnergy));*/

  meteringService->setVoltageASampler([]() {
    return (float) eic.GetLineVoltageA();
  });
  meteringService->setVoltageBSampler([]() {
    return (float) eic.GetLineVoltageB();
  });
  meteringService->setVoltageCSampler([]() {
    return (float) eic.GetLineVoltageC();
  });
  meteringService->setCurrentASampler([]() {
    return (float) eic.GetLineCurrentA(); 
  });
  meteringService->setCurrentBSampler([]() {
    return (float) eic.GetLineCurrentB(); 
  });
  meteringService->setCurrentCSampler([]() {
    return (float) eic.GetLineCurrentC(); 
  });
  meteringService->setEnergyASampler([](float volt_A, float current_A, time_t delta_A) {

  	float lastEnergy_A = energymeter.getFloat("currEnergy_A",0);
  	float finalEnergy_A = lastEnergy_A + ((float)(volt_A * current_A * ((float)delta_A)))/3600; //Whr

  	//placing energy value back in EEPROM
  	energymeter.putFloat("currEnergy_A",finalEnergy_A);

  	Serial.println("[EnergyASampler] currEnergy_A: "+ String(finalEnergy_A));

  	return roundingFloat(finalEnergy_A);
  	/*
	EEPROM.begin(sizeof(EEPROM_Data));
	float lastEnergy_A;
	EEPROM.get(4,lastEnergy_A);
	float finalEnergy_A = lastEnergy_A + ((float) (volt*current*((float)delta)))/3600;
	EEPROM.put(4, finalEnergy_A);
	EEPROM.commit();
	EEPROM.end();
	if (DEBUG_OUT) Serial.println("EEPROM Energy_A Register Value: ");
	if (DEBUG_OUT) Serial.println(finalEnergy_A);
	return finalEnergy_A;
	*/
  });

  meteringService->setPowerASampler([](float volt_A, float current_A) {
	return roundingFloat(volt_A*current_A/1000);
  });

  meteringService->setPowerBSampler([](float volt_B, float current_B) {
	return roundingFloat(volt_B*current_B/1000);
  });

 meteringService->setPowerCSampler([](float volt_C, float current_C) {
	return roundingFloat(volt_C*current_C/1000);
  });

  meteringService->setEnergyBSampler([](float volt_B, float current_B, time_t delta_B) {

  	float lastEnergy_B = energymeter.getFloat("currEnergy_B",0);
  	float finalEnergy_B = lastEnergy_B + ((float)(volt_B * current_B * ((float)delta_B)))/3600; //Whr

  	//placing energy value back in EEPROM
  	energymeter.putFloat("currEnergy_B",finalEnergy_B);

  	Serial.println("[EnergyBSampler] currEnergy_B: "+ String(finalEnergy_B));

  	return roundingFloat(finalEnergy_B);

  	/*
  	EEPROM.begin(sizeof(EEPROM_Data));
	float lastEnergy_B;
	EEPROM.get(40,lastEnergy_B);
	float finalEnergy_B = lastEnergy_B + ((float) (volt*current*((float)delta)))/3600;
	EEPROM.put(40, finalEnergy_B);
	EEPROM.commit();
	EEPROM.end();
	if (DEBUG_OUT) Serial.println("EEPROM Energy_B Register Value: ");
	if (DEBUG_OUT) Serial.println(finalEnergy_B);
	return finalEnergy_B;
//    return (float) eic.GetLineCurrentC(); //@bug: change this to energy.*/
  });


  meteringService->setEnergyCSampler([](float volt_C, float current_C, time_t delta_C) {
  	
  	float lastEnergy_C = energymeter.getFloat("currEnergy_C",0);
  	float finalEnergy_C = lastEnergy_C + ((float)(volt_C * current_C * ((float)delta_C)))/3600; //Whr

  	//placing energy value back in EEPROM
  	energymeter.putFloat("currEnergy_C",finalEnergy_C);

  	Serial.println("[EnergyCSampler] currEnergy_C: "+ String(finalEnergy_C));

  	return roundingFloat(finalEnergy_C);

  	/*
  	EEPROM.begin(sizeof(EEPROM_Data));
	float lastEnergy_C;
	EEPROM.get(64,lastEnergy_C);
	float finalEnergy_C = lastEnergy_C + ((float) (volt*current*((float)delta)))/3600;
	EEPROM.put(64, finalEnergy_C);
	EEPROM.commit();
	EEPROM.end();
	if (DEBUG_OUT) Serial.println("EEPROM Energy_C Register Value: ");
	if (DEBUG_OUT) Serial.println(finalEnergy_C);
	return finalEnergy_C;

 //   return (float) eic.GetLineCurrentC(); //@bug: change this to energy.*/
  });


  meteringService->setTemperatureSampler([]() {
    return (float) eic.GetTemperature(); //example values. Put your own power meter in heres
  });
  
  timer_init = false;
	
}
float MeteringService::currentEnergy_B(){
	
	float lastEnergy_B = energymeter.getFloat("currEnergy_B", 0);

	float volt_B = voltageBSampler();
	float curr_B = currentBSampler();

	time_t delta_B = now() - lastSampleTimeB;

	float finalEnergy_B = energyBSampler(volt_B, curr_B, delta_B);

	energymeter.putFloat("currEnergy_B", finalEnergy_B);

	if(DEBUG_OUT) Serial.println("The last Energy_B is: " + String(lastEnergy_B));
	if(DEBUG_OUT) Serial.println("The current Energy_B is: " + String(finalEnergy_B));

	return finalEnergy_B;

}


float MeteringService::currentEnergy_C(){

	float lastEnergy_C = energymeter.getFloat("currEnergy_C", 0);

	float volt_C = voltageCSampler();
	float curr_C = currentCSampler();

	time_t delta_C = now() - lastSampleTimeC;

	float finalEnergy_C = energyCSampler(volt_C, curr_C, delta_C);

	energymeter.putFloat("currEnergy_C", finalEnergy_C);

	if(DEBUG_OUT) Serial.println("The last Energy_C is: " + String(lastEnergy_C));
	if(DEBUG_OUT) Serial.println("The current Energy_C is: " + String(finalEnergy_C));

	return finalEnergy_C;
}

float MeteringService::currentEnergy_A(){

	float lastEnergy_A = energymeter.getFloat("currEnergy_A",0);

	float volt_A = voltageASampler();
	float curr_A = currentASampler();

	time_t delta_A = now() - lastSampleTimeA;

	float finalEnergy_A = energyASampler(volt_A, curr_A, delta_A);

	energymeter.putFloat("currEnergy_A", finalEnergy_A);

	if(DEBUG_OUT) Serial.println("The last Energy_A is: " + String(lastEnergy_A));
	if(DEBUG_OUT) Serial.println("The current Energy_A is: " + String(finalEnergy_A));

	return finalEnergy_A;



	/*
	EEPROM.begin(sizeof(EEPROM_Data));
	float lastEnergy;
	EEPROM.get(4,lastEnergy);

	float volt = voltageASampler();
	float curr = currentASampler();
	time_t delta = now() - lastSampleTimeA;
	float finalEnergy = energyASampler(volt, curr, delta);
	EEPROM.put(4, finalEnergy);
	EEPROM.commit();
	EEPROM.end();
	if (DEBUG_OUT_M) Serial.println("The last Energy is is: ");
		if (DEBUG_OUT_M) Serial.println(lastEnergy);
if (DEBUG_OUT_M) Serial.println("The Final Energy is is: ");
	if (DEBUG_OUT_M) Serial.println(finalEnergy);

	return finalEnergy;*/
	
}

/*void MeteringService::setPowerSampler(float (*ps)()){
  this->powerSampler = ps;
}*/

void MeteringService::setEnergyASampler(float (*es_A)(float volt, float current, time_t delta)){
  this->energyASampler = es_A;
}

void MeteringService::setEnergyBSampler(float (*es_B)(float volt, float current, time_t delta)){
  this->energyBSampler = es_B;
}

void MeteringService::setEnergyCSampler(float (*es_C)(float volt, float current, time_t delta)){
  this->energyCSampler = es_C;
}

void MeteringService::setPowerASampler(float (*ps_A)(float volt, float current)){
  this->powerASampler = ps_A;
}

void MeteringService::setPowerBSampler(float (*ps_B)(float volt, float current)){
  this->powerBSampler = ps_B;
}

void MeteringService::setPowerCSampler(float (*ps_C)(float volt, float current)){
  this->powerCSampler = ps_C;
}

void MeteringService::setVoltageASampler(float (*vs)()){
  this->voltageASampler = vs;
}

void MeteringService::setVoltageBSampler(float (*vs)()){
  this->voltageBSampler = vs;
}

void MeteringService::setVoltageCSampler(float (*vs)()){
  this->voltageCSampler = vs;
}

void MeteringService::setCurrentASampler(float (*cs)()){
  this->currentASampler = cs;
}

void MeteringService::setCurrentBSampler(float (*cs)()){
  this->currentBSampler = cs;
}

void MeteringService::setCurrentCSampler(float (*cs)()){
  this->currentCSampler = cs;
}

void MeteringService::setTemperatureSampler(float (*ts)()){
  this->temperatureSampler = ts;
}

float roundingFloat(float var)    //rounding to 2 decimal point
{
    // 37.66666 * 100 =3766.66
    // 3766.66 + .5 =3767.16    for rounding off value
    // then type cast to int so value is 3767
    // then divided by 100 so the value converted into 37.67
    float value = (int)(var * 100 + .5);
    return (float)value / 100;
}