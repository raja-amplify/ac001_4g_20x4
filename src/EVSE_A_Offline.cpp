// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

/**
Edited by Pulkit Agrawal.
*/

#include "EVSE_A.h"
#include "Master.h"
#include "EVSE_A_Offline.h"
#include "MeteringService.h"
#include "OcppEngine.h"


#include "ControlPilot.h"

#include "LCD_I2C.h"

#if DISPLAY_ENABLED
#include "display.h"
extern bool flag_tapped; 
#endif

extern int8_t fault_code_A;

#if DWIN_ENABLED
#include "dwin.h"

extern unsigned char v1[8];
extern unsigned char v2[8];
extern unsigned char v3[8];
extern unsigned char i1[8];
extern unsigned char i2[8];
extern unsigned char i3[8];
extern unsigned char e1[8];
extern unsigned char e2[8];
extern unsigned char e3[8];
extern unsigned char change_page[10];
extern unsigned char avail[22];
extern unsigned char charging[28];
extern unsigned char cid1[7];
extern unsigned char fault_emgy[28];
#endif

#if 0
//new variable names defined by @Pulkit. might break the build.
OnBoot_A onBoot_A;
OnReadUserId_A onReadUserId_A;
OnSendHeartbeat_A onSendHeartbeat_A;
OnAuthentication_A onAuthentication_A;
OnStartTransaction_A onStartTransaction_A;
OnStopTransaction_A onStopTransaction_A;
OnUnauthorizeUser_A onUnauthorizeUser_A;
#endif



extern uint8_t currentCounterThreshold_A;

extern bool disp_evse_A;

extern ulong timerHb;
extern unsigned int heartbeatInterval;

extern bool notFaulty_A;

//timeout for heartbeat signal.
extern ulong T_SENDHEARTBEAT;
extern bool timeout_active_A;
extern bool timer_initialize_A;
extern ulong timeout_start_A;
//Reason for stop
extern uint8_t reasonForStop_A;
//new flag names. replace them with old names.
extern bool evIsPlugged_A; 
extern bool flag_evseIsBooted_A;
extern bool flag_evseReadIdTag_A;
extern bool flag_evseAuthenticate_A;
extern bool flag_evseStartTransaction_A;
extern bool flag_evRequestsCharge_A;
extern bool flag_evseStopTransaction_A;
extern bool flag_evseUnauthorise_A;
extern bool flag_rebootRequired_A;
extern bool flag_evseSoftReset_A; //added by @Wamique

extern float minCurr;

extern ATM90E36 eic;
extern bool flag_rebootRequired_B;
extern bool flag_rebootRequired_C;

//not used. part of Smart Charging System.
extern float chargingLimit_A;
String Ext_currentIdTag_A_Offl = "";
extern WebSocketsClient webSocket;

extern ulong timerDisplay;
extern bool EMGCY_FaultOccured;

extern LCD_I2C lcd;

extern MFRC522 mfrc522;
//extern String currentIdTag;
extern long int blinckCounter_A;
extern int counter1_A;
extern ulong t;
ulong t_A;
extern int connectorDis_counter_A;

String currentIdTag_A_Offl = "";

extern EVSE_states_enum EVSE_state;
extern Preferences preferences;
extern short int fault_counter_A;
extern bool flag_faultOccured_A;
extern short int counter_drawingCurrent_A;
extern float drawing_current_A;

extern bool webSocketConncted;
extern bool isInternetConnected;
extern short counter_faultstate_A;

//metering Flag
extern bool flag_MeteringIsInitialised;
extern MeteringService *meteringService;
extern bool wifi_connect;
extern bool gsm_connect;
extern bool offline_connect;
extern bool flag_internet;
extern bool flag_offline;

extern Preferences resumeTxn_A;
extern TinyGsmClient client;

extern bool ongoingTxn_A;
extern String idTagData_A;

extern ulong relay_timer_A;
extern ulong faultTimer_A;

extern short EMGCY_counter_A;
extern bool EMGCY_FaultOccured_A;

unsigned long timer_green_offline = 0;
extern time_t lastsampledTimeA_off;

extern uint8_t currentCounterThreshold_A;
extern int globalmeterstartA;
extern unsigned long st_timeA;
const ulong TIMEOUT_EMERGENCY_RELAY_CLOSE_A = 120000;
bool offline_charging_A = false;
ulong timer_Offl_A;
bool flag_txndone_off_A = false;
extern bool flag_txndone_off_B;
extern bool flag_txndone_off_C;

float offline_charging_Enargy_A = 0;

extern unsigned long offline_t_A;


extern Preferences energymeter;

/****************New Offline Functions********************************/

void EVSE_A_stopOfflineTxn()
{
  disp_evse_A = false;
  requestForRelay(STOP, 1);
  getChargePointStatusService_A()->stopEvDrawsEnergy();
  getChargePointStatusService_A()->unauthorize();
  if( getChargePointStatusService_A()->getEmergencyRelayClose() == false)
  {
    requestLed(GREEN, START, 1);
  }
  
  // Display transaction finished
  energymeter.begin("MeterData", false);
  float meterStop = energymeter.getFloat("currEnergy_A", 0);
  energymeter.end();
  unsigned long stop_time = millis();

  flag_txndone_off_A = true;
  #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop - globalmeterstartA) / 1000)));
  lcd.setCursor(0, 2);
  lcd.print("CONNECTOR A");
  lcd.setCursor(0, 3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds / 3600;                                                 // Number of seconds in an hour
  int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
  int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
  delay(5000);
#endif


}

#if 0
void showTxn_Finish()
{
  if(flag_txndone_off_A)
  {
    flag_txndone_off_A = false;
   #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop - globalmeterstartA) / 1000)));
  lcd.setCursor(0, 3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeA) / 1000;
  int hr = seconds / 3600;                                                 // Number of seconds in an hour
  int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
  int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
  delay(5000);
#endif
  }
  if(flag_txndone_off_B)
  {
    flag_txndone_off_B = false;
    #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop-globalmeterstartB)/1000)));
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
  }
   if(flag_txndone_off_C)
  {
    flag_txndone_off_C = false;
   #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop - globalmeterstartC) / 1000)));
  lcd.setCursor(0, 3);
  lcd.print("DURATION  :");
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  unsigned long seconds = (stop_time - st_timeC) / 1000;
  int hr = seconds / 3600;                                                 // Number of seconds in an hour
  int mins = (seconds - hr * 3600) / 60;                                   // Remove the number of hours and calculate the minutes.
  int sec = seconds - hr * 3600 - mins * 60;                               // Remove the number of hours and minutes, leaving only seconds.
  String hrMinSec = (String(hr) + ":" + String(mins) + ":" + String(sec)); // Converts to HH:MM:SS string. This can be returned to the calling function.
  lcd.print(String(hrMinSec));
  delay(5000);
#endif
  }
}
#endif 

void EVSE_A_startOfflineTxn()
{
  offline_charging_A = true;
  disp_evse_A = true;
  requestForRelay(START, 1);
  requestLed(ORANGE, START, 1);
  delay(1200);
  requestLed(WHITE, START, 1);
  delay(1200);
  requestLed(GREEN, START, 1);
  delay(1000);
  requestLed(BLINKYGREEN, START, 1);
  Serial.println("[EVSE] EV is connected and Started charging");
  if (DEBUG_OUT)
    Serial.println("[EVSE] Started Drawing Energy");
  st_timeA = millis();
  offline_t_A = millis();
  lastsampledTimeA_off = now();
  energymeter.begin("MeterData", false);
  globalmeterstartA = energymeter.getFloat("currEnergy_A", 0);
  energymeter.end();
}

void EVSE_A_offline_Loop()
{
  if (offline_charging_A)
  {

    drawing_current_A = eic.GetLineCurrentA();
    if (drawing_current_A <= minCurr)
    {
      counter_drawingCurrent_A++;
      if (counter_drawingCurrent_A > currentCounterThreshold_A)
      {
        counter_drawingCurrent_A = 0;
        Serial.println("Stopping Session Becoz of no Current");
        offline_charging_A = false;
        Serial.println("Stopping Offline Charging by low current");
        EVSE_A_stopOfflineTxn();
        disp_evse_A = false;
#if LCD_ENABLED
        lcd.clear();
        lcd.setCursor(3, 0);
        lcd.print("No Power Drawn /");
        lcd.setCursor(3, 1);
        lcd.print("EV disconnected");
#endif
      }
    }
    else
    {
      counter_drawingCurrent_A = 0;
      currentCounterThreshold_A = 60; // ARAI expects 2
      Serial.println("counter_drawingCurrent Reset");
    }

    emergencyRelayClose_Loop_A();

    if (DEBUG_OUT)
      Serial.println("[EVSE] Drawing Energy");

    if (millis() - t_A > 5000)
    {
      float instantCurrrent_A = eic.GetLineCurrentA();
      float instantVoltage_A = eic.GetLineVoltageA();

      if (getChargePointStatusService_A()->getEmergencyRelayClose() == true)
      {

        offline_charging_A = false;
        Serial.println("Stopping Offline Charging by emergency");
        EVSE_A_stopOfflineTxn();
        disp_evse_A = false;
      }

      if (millis() - relay_timer_A > 15000 && offline_charging_A == true)
      {

        requestForRelay(START, 1);
        relay_timer_A = millis();
        if( getChargePointStatusService_A()->getEmergencyRelayClose() == false)
        {
          
          requestLed(BLINKYGREEN, START, 1);
        }
        
      }

      time_t sampledTimeA_off = now();
      time_t delta = sampledTimeA_off - lastsampledTimeA_off;
      energymeter.begin("MeterData", false);
      float lastEnergy_A = energymeter.getFloat("currEnergy_A", 0);
      float finalEnergy_A = lastEnergy_A + ((float)(instantVoltage_A * instantCurrrent_A * ((float)delta))) / 3600; // Whr

      // placing energy value back in EEPROM
      offline_charging_Enargy_A = finalEnergy_A;
      energymeter.putFloat("currEnergy_A", finalEnergy_A);
      Serial.println("[EnergyCSampler] currEnergy_A: " + String(finalEnergy_A));
      energymeter.end();

      lastsampledTimeA_off = sampledTimeA_off;
    }
  }
}

void EVSE_A_LED_loop()
{

  // if not faulted and not charging then take the led status to green once every 8 seconds

  if (getChargePointStatusService_A()->getEmergencyRelayClose() == false && offline_charging_A == false)
  {
    if (millis() - timer_green_offline > 8000)
    {
      timer_green_offline = millis();
      requestLed(GREEN, START, 1);
      #if LCD_ENABLED
					lcd.clear();
					lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
					lcd.print("STATUS: AVAILABLE");
					lcd.setCursor(0, 1);
					lcd.print("TAP RFID/SCAN QR");
					lcd.setCursor(0, 2);
					lcd.print("CONNECTION");
					lcd.setCursor(0, 3);
					lcd.print("CLOUD: OFFLINE");
					#endif
    }
  }
}
