// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "StopTransaction.h"
#include "OcppEngine.h"
#include "MeteringService.h"
#include "LCD_I2C.h"
extern LCD_I2C lcd;

extern int globalmeterstartA;
extern unsigned long st_timeA;
extern int globalmeterstartB;
extern unsigned long st_timeB;
extern int globalmeterstartC;
extern unsigned long st_timeC;

StopTransaction::StopTransaction(String idTag, int transactionId, int connectorId) {
this->idTag = idTag;
this->transactionId = transactionId;
this->connectorId = connectorId;


}

StopTransaction::StopTransaction() {
}

const char* StopTransaction::getOcppOperationType(){
    return "StopTransaction";
}

DynamicJsonDocument* StopTransaction::createReq() {

  // String idTag = String('\0');

  // if (getChargePointStatusService() != NULL) {
  //   idTag += getChargePointStatusService()->getIdTag();
  // }

  DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(4) + (idTag.length() + 1) + (JSONDATE_LENGTH + 1));
  JsonObject payload = doc->to<JsonObject>();
  
  if (!idTag.isEmpty()) { //if there is no idTag present, we shouldn't add a default one
    payload["idTag"] = idTag;
  }

  float meterStop = 0.0f;
  unsigned long stop_time=0;
  if (getMeteringService() != NULL) {
    if(connectorId == 1){
      meterStop = getMeteringService()->currentEnergy_A();
      stop_time = millis();
      //Add that stop lcd display over here
  #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH");
  lcd.setCursor(4, 1);
  lcd.print(String(float((meterStop-globalmeterstartA)/1000)));
  lcd.setCursor(0,2);
  lcd.print("WH");
  lcd.setCursor(4,2);
  lcd.print(meterStop-globalmeterstartA);
  lcd.setCursor(0,3);
  unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
	#endif
    }else if(connectorId == 2){
      meterStop = getMeteringService()->currentEnergy_B();
      stop_time = millis();
        //Add that stop lcd display over here
  #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH");
  lcd.setCursor(4, 1);
  lcd.print(String(float((meterStop-globalmeterstartB)/1000)));
  lcd.setCursor(0,2);
  lcd.print("WH");
  lcd.setCursor(4,2);
  lcd.print(meterStop-globalmeterstartB);
  lcd.setCursor(0,3);
  unsigned long seconds = (stop_time - st_timeB) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
	#endif
    }else if(connectorId == 3){
      meterStop = getMeteringService()->currentEnergy_C();
      stop_time = millis();
        //Add that stop lcd display over here
  #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH");
  lcd.setCursor(4, 1);
  lcd.print(String(float((meterStop-globalmeterstartC)/1000)));
  lcd.setCursor(0,2);
  lcd.print("WH");
  lcd.setCursor(4,2);
  lcd.print(meterStop-globalmeterstartC);
  lcd.setCursor(0,3);
  unsigned long seconds = (stop_time - st_timeC) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
	#endif
    }

  }

  payload["meterStop"] = meterStop; //TODO meterStart is required to be in Wh, but measuring unit is probably inconsistent in implementation
  char timestamp[JSONDATE_LENGTH + 1] = {'\0'};
  getJsonDateStringFromSystemTime(timestamp, JSONDATE_LENGTH);
  payload["timestamp"] = timestamp;

  // int transactionId = -1;
  // if (getChargePointStatusService() != NULL) {
  //   transactionId = getChargePointStatusService()->getTransactionId();
  // }
  payload["transactionId"] = transactionId;

  

  // if (getChargePointStatusService() != NULL) {
  //   getChargePointStatusService()->stopEnergyOffer();
  // }

  return doc;
}

void StopTransaction::processConf(JsonObject payload) {

  //no need to process anything here

  // ChargePointStatusService *cpStatusService = getChargePointStatusService();
  // if (cpStatusService != NULL){
  //   //cpStatusService->stopEnergyOffer(); //No. This should remain in createReq
  //   cpStatusService->stopTransaction();
  //   cpStatusService->unauthorize();
  // }

  SmartChargingService *scService = getSmartChargingService();
  if (scService != NULL){
    scService->endChargingNow();
  }

  if (DEBUG_OUT) Serial.print(F("[StopTransaction] Request has been accepted!\n"));

}


void StopTransaction::processReq(JsonObject payload) {
  /**
   * Ignore Contents of this Req-message, because this is for debug purposes only
   */
}

DynamicJsonDocument* StopTransaction::createConf(){
  DynamicJsonDocument* doc = new DynamicJsonDocument(2 * JSON_OBJECT_SIZE(1));
  JsonObject payload = doc->to<JsonObject>();

  JsonObject idTagInfo = payload.createNestedObject("idTagInfo");
  idTagInfo["status"] = "Accepted";

  return doc;
}
