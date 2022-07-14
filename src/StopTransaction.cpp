// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#include "Variants.h"

#include "StopTransaction.h"
#include "OcppEngine.h"
#include "MeteringService.h"
#if DISPLAY_ENABLED
#include "display.h"
extern bool flag_unfreeze;
#endif
#if LCD_ENABLED
#include "LCD_I2C.h"
extern LCD_I2C lcd;
#endif
#if DWIN_ENABLED
#include "dwin.h"

extern unsigned char kwh[8];
extern unsigned char change_page[10];
extern unsigned char HR[8];
extern unsigned char MINS[8];
extern unsigned char SEC[8];
extern unsigned char cid1[7];
extern unsigned char cid2[7];
extern unsigned char cid3[7];
#endif

extern bool disp_evse_A;
extern bool disp_evse_B;
extern bool disp_evse_C;

extern bool flag_start_txn_A;
extern bool flag_start_txn_B;
extern bool flag_start_txn_C;

extern int globalmeterstartA;
extern unsigned long st_timeA;
extern int globalmeterstartB;
extern unsigned long st_timeB;
extern int globalmeterstartC;
extern unsigned long st_timeC;

/*
* @brief: Feature added by Raja
* This feature will avoid hardcoding of messages. 
*/
//typedef enum resonofstop { EmergencyStop, EVDisconnected , HardReset, Local , Other , PowerLoss, Reboot,Remote, Softreset,UnlockCommand,DeAuthorized};
extern uint8_t reasonForStop_A;
extern uint8_t reasonForStop_B;
extern uint8_t reasonForStop_C;
static const char *resonofstop_str[] = { "EmergencyStop", "EVDisconnected" , "HardReset", "Local" , "Other" , "PowerLoss", "Reboot","Remote", "SoftReset","UnlockCommand","DeAuthorized"};

extern uint8_t currentCounterThreshold_A;
extern uint8_t currentCounterThreshold_B;
extern uint8_t currentCounterThreshold_C;

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
  //JSON_OBJECT_SIZE = 5 to include reason for stop as well.

  DynamicJsonDocument *doc = new DynamicJsonDocument(JSON_OBJECT_SIZE(5) + (idTag.length() + 1) + (JSONDATE_LENGTH + 1));
  JsonObject payload = doc->to<JsonObject>();
  
  if (!idTag.isEmpty()) { //if there is no idTag present, we shouldn't add a default one
    payload["idTag"] = idTag;
  }

  float meterStop = 0.0f;
  unsigned long stop_time=0;
  if (getMeteringService() != NULL) {
    if(connectorId == 1){
      if(flag_start_txn_A)
      {
        flag_start_txn_A = false;
      meterStop = getMeteringService()->currentEnergy_A();
      stop_time = millis();
      currentCounterThreshold_A = 60;
      //Add that stop lcd display over here
      #if DISPLAY_ENABLED
      unsigned long seconds = (stop_time - st_timeA) / 1000;
      int hr = seconds/3600;                                                        //Number of seconds in an hour
      int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
      int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
      String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
      thanks_Disp_AC("1",String(float((meterStop-globalmeterstartA)/1000)),hrMinSec);
      #endif
  #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  // lcd.print("KWH");
  // lcd.setCursor(4, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop-globalmeterstartA)/1000)));
  // lcd.setCursor(0,2);
  // lcd.print("WH");
  // lcd.setCursor(4,2);
  // lcd.print(meterStop-globalmeterstartA);
  lcd.setCursor(0,2);
  lcd.print("CONNECTOR A");
  lcd.setCursor(0,3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
   delay(5000);
	#endif
  #if DWIN_ENABLED
  uint8_t err = 0;
    unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
   err = DWIN_SET(cid1, sizeof(cid1) / sizeof(cid1[0]));
      // Take to page 2. 
      change_page[9] = 1; 
      err = DWIN_SET(change_page,sizeof(change_page)/sizeof(change_page[0]));
      //kwh[7] = float((meterStop-globalmeterstartA)/1000);
      //kwh[7] = int((meterStop-globalmeterstartA)/1000);
      int jaf = meterStop-globalmeterstartA;
      kwh[6] = jaf >> 8;
      kwh[7] = jaf & 0xff;
      //Since Kwh is with 3 decimal points multiply by 1000
      //kwh[7]  = jaf/1000;
      err = DWIN_SET(kwh,sizeof(kwh)/sizeof(kwh[0]));
      HR[7] = hr;
      MINS[7] = mins;
      SEC[7] = sec;
      err = DWIN_SET(HR,sizeof(HR)/sizeof(HR[0]));
      err = DWIN_SET(MINS,sizeof(MINS)/sizeof(MINS[0]));
      err = DWIN_SET(SEC,sizeof(SEC)/sizeof(SEC[0]));
      delay(3000);
      // Take to page 2. 
      change_page[9] = 0; 
      err = DWIN_SET(change_page,sizeof(change_page)/sizeof(change_page[0]));
      delay(50);
  #endif
      }
    }
    else if(connectorId == 2){
       if(flag_start_txn_B)
      {
        flag_start_txn_B = false;
      meterStop = getMeteringService()->currentEnergy_B();
      stop_time = millis();
      currentCounterThreshold_B = 60;
        //Add that stop lcd display over here
        #if DISPLAY_ENABLED
      unsigned long seconds = (stop_time - st_timeB) / 1000;
      int hr = seconds/3600;                                                        //Number of seconds in an hour
      int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
      int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
      String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
      thanks_Disp_AC("2",String(float((meterStop-globalmeterstartB)/1000)),hrMinSec);
      #endif
  #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  // lcd.print("KWH");
  // lcd.setCursor(4, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop-globalmeterstartB)/1000)));
  // lcd.setCursor(0,2);
  // lcd.print("WH");
  // lcd.setCursor(4,2);
  // lcd.print(meterStop-globalmeterstartB);
  lcd.setCursor(0,2);
  lcd.print("CONNECTOR B");
  lcd.setCursor(0,3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeB) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
   delay(5000);
	#endif
  
  #if DWIN_ENABLED
uint8_t err = 0;
    unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
   
          err = DWIN_SET(cid2, sizeof(cid2) / sizeof(cid2[0]));
                 
          
      // Take to page 2. 
      change_page[9] = 1; 
      err = DWIN_SET(change_page,sizeof(change_page)/sizeof(change_page[0]));
      //kwh[7] = float((meterStop-globalmeterstartA)/1000);
      kwh[7] = int((meterStop-globalmeterstartA)/1000);
      err = DWIN_SET(kwh,sizeof(kwh)/sizeof(kwh[0]));
      HR[7] = hr;
      MINS[7] = mins;
      SEC[7] = sec;
      err = DWIN_SET(HR,sizeof(HR)/sizeof(HR[0]));
      err = DWIN_SET(MINS,sizeof(MINS)/sizeof(MINS[0]));
      err = DWIN_SET(SEC,sizeof(SEC)/sizeof(SEC[0]));
      delay(3000);
      // Take to page 2. 
      change_page[9] = 0; 
      err = DWIN_SET(change_page,sizeof(change_page)/sizeof(change_page[0]));
      delay(50);

  #endif
      }
    }else if(connectorId == 3){
       if(flag_start_txn_C)
      {
        flag_start_txn_C = false;
      meterStop = getMeteringService()->currentEnergy_C();
      stop_time = millis();
      //reset the counter.
      currentCounterThreshold_C = 60;

      #if DISPLAY_ENABLED
      unsigned long seconds = (stop_time - st_timeC) / 1000;
      int hr = seconds/3600;                                                        //Number of seconds in an hour
      int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
      int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
      String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
      thanks_Disp_AC("3",String(float((meterStop-globalmeterstartC)/1000)),hrMinSec);
      #endif
        //Add that stop lcd display over here
  #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  // lcd.print("KWH");
  // lcd.setCursor(4, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop-globalmeterstartC)/1000)));
  // lcd.setCursor(0,2);
  // lcd.print("WH");
  // lcd.setCursor(4,2);
  // lcd.print(meterStop-globalmeterstartC);
  lcd.setCursor(0,2);
  lcd.print("CONNECTOR C");
  lcd.setCursor(0,3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeC) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec));  //Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
  delay(5000);
	#endif

  #if DWIN_ENABLED
uint8_t err = 0;
    unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds/3600;                                                        //Number of seconds in an hour
  int mins = (seconds-hr*3600)/60;                                              //Remove the number of hours and calculate the minutes.
  int sec = seconds-hr*3600-mins*60;                                            //Remove the number of hours and minutes, leaving only seconds.
  
      // Take to page 2. 
       err = DWIN_SET(cid3, sizeof(cid3) / sizeof(cid3[0]));
      change_page[9] = 1; 
      err = DWIN_SET(change_page,sizeof(change_page)/sizeof(change_page[0]));
      //kwh[7] = float((meterStop-globalmeterstartA)/1000);
      kwh[7] = int((meterStop-globalmeterstartA)/1000);
      err = DWIN_SET(kwh,sizeof(kwh)/sizeof(kwh[0]));
      HR[7] = hr;
      MINS[7] = mins;
      SEC[7] = sec;
      err = DWIN_SET(HR,sizeof(HR)/sizeof(HR[0]));
      err = DWIN_SET(MINS,sizeof(MINS)/sizeof(MINS[0]));
      err = DWIN_SET(SEC,sizeof(SEC)/sizeof(SEC[0]));
      delay(3000);
      // Take to page 2. 
      change_page[9] = 0; 
      err = DWIN_SET(change_page,sizeof(change_page)/sizeof(change_page[0]));
      delay(50);

  #endif

    }
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
  #if DISPLAY_ENABLED
  flag_unfreeze = true;
  #endif
  
  switch(connectorId)
  {
    case 1: 
  Serial.printf("[StopTransaction] reason for stop : %d", reasonForStop_A);
  payload["reason"] = resonofstop_str[reasonForStop_A];
  disp_evse_A = false;
  //Clearing the reason.
 reasonForStop_A = 3;
  break;
  case 2: 
  Serial.printf("[StopTransaction] reason for stop : %d", reasonForStop_B);
  payload["reason"] = resonofstop_str[reasonForStop_B];
  disp_evse_B = false;
  //Clearing the reason.
 reasonForStop_B = 3;
  break;
  case 3: 
  Serial.printf("[StopTransaction] reason for stop : %d", reasonForStop_C);
  payload["reason"] = resonofstop_str[reasonForStop_C];
  disp_evse_C = false;
  //Clearing the reason.
 reasonForStop_C = 3;
  break;
  }

  

  

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
