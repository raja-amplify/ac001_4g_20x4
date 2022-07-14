#include "EVSE_C.h"
#include "Master.h"
#include "ControlPilot.h"
#include "LCD_I2C.h"
#include "EVSE_C_Offline.h"

#if DISPLAY_ENABLED
#include "display.h"
extern bool flag_tapped; 
#endif

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
extern unsigned char cid3[7];
#endif


extern uint8_t currentCounterThreshold_C;
extern float minCurr;

#if 0
OnBoot_C onBoot_C;
OnReadUserId_C onReadUserId_C;
OnSendHeartbeat_C onSendHeartbeat_C;
OnAuthentication_C onAuthentication_C;
OnStartTransaction_C onStartTransaction_C;
OnStopTransaction_C onStopTransaction_C;
OnUnauthorizeUser_C onUnauthorizeUser_C;
#endif


extern bool evIsPlugged_C; 
extern bool flag_evseIsBooted_C;
extern bool flag_evseReadIdTag_C;
extern bool flag_evseAuthenticate_C;
extern bool flag_evseStartTransaction_C;
extern bool flag_evRequestsCharge_C;
extern bool flag_evseStopTransaction_C;
extern bool flag_evseUnauthorise_C;
extern bool flag_rebootRequired_C;
extern bool flag_evseSoftReset_C; 
extern bool notFaulty_C;
extern bool flag_ed_A;
extern bool flag_rebootRequired_A;
extern bool flag_rebootRequired_B;

extern float chargingLimit_C;

String Ext_currentIdTag_C_Offl = "";

extern long int blinckCounter_C;
extern int counter1_C ;
extern ulong timerDisplayC;

//Reason for stop
extern uint8_t reasonForStop_C;
extern int8_t fault_code_C;

extern LCD_I2C lcd;

extern ulong t_C;
extern int connectorDis_counter_C;
extern short int counter_drawingCurrent_C;
extern float drawing_current_C ;

String currentIdTag_C_Offl ="";

extern WebSocketsClient webSocket;

extern bool disp_evse_C ;

extern EVSE_states_enum EVSE_state;
extern Preferences preferences;
extern MFRC522 mfrc522;
extern String currentIdTag;
extern bool webSocketConncted;
extern bool isInternetConnected;
extern bool wifi_connect;
extern bool gsm_connect;
extern bool offline_connect;
extern bool flag_offline;
extern bool flag_internet;
extern Preferences resumeTxn_C;
extern TinyGsmClient client;
extern bool ongoingTxn_C;
extern String idTagData_C;

extern bool timeout_active_C;
extern bool timer_initialize_C;
extern ulong timeout_start_C;
extern short int fault_counter_C;
extern bool flag_faultOccured_C;
extern ulong relay_timer_C;
extern ulong faultTimer_C;
const ulong TIMEOUT_EMERGENCY_RELAY_CLOSE_C = 120000;

extern uint8_t currentCounterThreshold_C;
extern int globalmeterstartC;
extern unsigned long st_timeC;
unsigned long timer_green_offline_C = 0;
extern time_t lastsampledTimeC_off;

float offline_charging_Enargy_C = 0;
extern unsigned long offline_t_C;

extern Preferences energymeter;

bool offline_charging_C;
bool flag_txndone_off_C = false;
ulong timer_Offl_C;
/****************New Offline Functions********************************/

void EVSE_C_stopOfflineTxn()
{
  disp_evse_C = false;
  requestForRelay(STOP, 3);
  getChargePointStatusService_C()->stopEvDrawsEnergy();
  getChargePointStatusService_C()->unauthorize();
  if( getChargePointStatusService_C()->getEmergencyRelayClose() == false)
  {
    requestLed(GREEN, START, 3);
  }
  
  // Display transaction finished
  energymeter.begin("MeterData", false);
  float meterStop = energymeter.getFloat("currEnergy_C", 0);
  energymeter.end();
  unsigned long stop_time = millis();
flag_txndone_off_C = true;
 #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop - globalmeterstartC) / 1000)));
  lcd.setCursor(0, 2);
  lcd.print("CONNECTOR C");
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



void EVSE_C_startOfflineTxn()
{
  offline_charging_C = true;
  disp_evse_C = true;
  requestForRelay(START, 3);
  requestLed(ORANGE, START, 3);
  delay(1200);
  requestLed(WHITE, START, 3);
  delay(1200);
  requestLed(GREEN, START, 3);
  delay(1000);
  requestLed(BLINKYGREEN, START, 3);
  Serial.println("[EVSE] EV is connected and Started charging");
  if (DEBUG_OUT)
    Serial.println("[EVSE] Started Drawing Energy");
  st_timeC = millis();
  offline_t_C = millis();
  lastsampledTimeC_off = now();
  energymeter.begin("MeterData", false);
  globalmeterstartC = energymeter.getFloat("currEnergy_C", 0);
  energymeter.end();
}

void EVSE_C_offline_Loop()
{
  if (offline_charging_C)
  {

    drawing_current_C = eic.GetLineCurrentC();
    if (drawing_current_C <= minCurr)
    {
      counter_drawingCurrent_C++;
      if (counter_drawingCurrent_C > currentCounterThreshold_C)
      {
        counter_drawingCurrent_C = 0;
        Serial.println("Stopping Session Becoz of no Current");
        offline_charging_C = false;
        Serial.println("Stopping Offline Charging by low current");
        EVSE_C_stopOfflineTxn();
        disp_evse_C = false;
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
      counter_drawingCurrent_C = 0;
      currentCounterThreshold_C = 60; // 2 ideally.
      Serial.println("counter_drawingCurrent Reset");
    }

    emergencyRelayClose_Loop_C();

    if (DEBUG_OUT)
      Serial.println("[EVSE] Drawing Energy");

    if (millis() - t_C > 5000)
    {

      float instantCurrrent_C = eic.GetLineCurrentC();
      float instantVoltage_C = eic.GetLineVoltageC();

      if (getChargePointStatusService_C()->getEmergencyRelayClose() == true)
      {

        offline_charging_C = false;
        Serial.println("Stopping Offline Charging by emergency");
        EVSE_C_stopOfflineTxn();
        disp_evse_C = false;
      }

      if (millis() - relay_timer_C > 15000 && offline_charging_C == true)
      {

        requestForRelay(START, 3);
        relay_timer_C = millis();
        if( getChargePointStatusService_C()->getEmergencyRelayClose() == false)
        {
          requestLed(BLINKYGREEN, START, 3);
        }
      }

      time_t sampledTimeC_off = now();
      time_t delta = sampledTimeC_off - lastsampledTimeC_off;
      energymeter.begin("MeterData", false);
      float lastEnergy_C = energymeter.getFloat("currEnergy_C", 0);
      float finalEnergy_C = lastEnergy_C + ((float)(instantVoltage_C * instantCurrrent_C * ((float)delta))) / 3600; // Whr

      // placing energy value back in EEPROM
      offline_charging_Enargy_C = finalEnergy_C;
      energymeter.putFloat("currEnergy_C", finalEnergy_C);
      Serial.println("[EnergyCSampler] currEnergy_C: " + String(finalEnergy_C));
      energymeter.end();

      lastsampledTimeC_off = sampledTimeC_off;
    }
  }
}

void EVSE_C_LED_loop()
{

  // if not faulted and not charging then take the led status to green once every 5 seconds

  if (getChargePointStatusService_C()->getEmergencyRelayClose() == false && offline_charging_C == false)
  {
    if (millis() - timer_green_offline_C > 8000)
    {
      timer_green_offline_C = millis();
      requestLed(GREEN, START, 3);
    }
  }
}
