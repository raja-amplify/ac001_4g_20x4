#include "EVSE_B.h"
#include "Master.h"
#include "ControlPilot.h"
#include "LCD_I2C.h"
#include "EVSE_B_Offline.h"

#if DISPLAY_ENABLED
#include "display.h"
extern bool flag_tapped; 
#endif

#if DWIN_ENABLED
#include "dwin.h"
extern SoftwareSerial dwin;
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
extern unsigned char cid2[7];
#endif

extern uint8_t currentCounterThreshold_B;
extern bool disp_evse_B;
extern float minCurr;
#if 0
OnBoot_B onBoot_B;
OnReadUserId_B onReadUserId_B;
OnSendHeartbeat_B onSendHeartbeat_B;
OnAuthentication_B onAuthentication_B;
OnStartTransaction_B onStartTransaction_B;
OnStopTransaction_B onStopTransaction_B;
OnUnauthorizeUser_B onUnauthorizeUser_B;
#endif



extern bool notFaulty_B;
extern bool evIsPlugged_B; 
extern bool flag_evseIsBooted_B;
extern bool flag_evseReadIdTag_B;
extern bool flag_evseAuthenticate_B;
extern bool flag_evseStartTransaction_B;
extern bool flag_evRequestsCharge_B;
extern bool flag_evseStopTransaction_B;
extern bool flag_evseUnauthorise_B;
extern bool flag_rebootRequired_B;
extern bool flag_evseSoftReset_B; 
extern bool flag_rebootRequired_C;
extern bool flag_rebootRequired_A;
extern bool flag_ed_A;

//Reason for stop
extern uint8_t reasonForStop_B;
extern int8_t fault_code_B;
extern float chargingLimit_B;

String Ext_currentIdTag_B_Offl = "";

extern long int blinckCounter_B;
extern int counter1_B;
extern ulong t_B;
extern int connectorDis_counter_B;
extern short int counter_drawingCurrent_B;
extern float drawing_current_B;

String currentIdTag_B_Offl = "";

extern WebSocketsClient webSocket;
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
extern Preferences resumeTxn_B;
extern TinyGsmClient client;

extern bool ongoingTxn_B;
extern String idTagData_B;

extern bool timeout_active_B;
extern bool timer_initialize_B;
extern ulong timeout_start_B ;
extern short int fault_counter_B ;
extern bool flag_faultOccured_B ;
extern ulong relay_timer_B;
extern ulong faultTimer_B;
extern ulong timerDisplayB;
const ulong TIMEOUT_EMERGENCY_RELAY_CLOSE_B = 120000;

unsigned long timer_green_offline_B = 0;
extern time_t lastsampledTimeB_off;


extern uint8_t currentCounterThreshold_B;
extern int globalmeterstartB;
extern unsigned long st_timeB;

bool flag_txndone_off_B = false;

float offline_charging_Enargy_B = 0;

extern Preferences energymeter;

extern LCD_I2C lcd;
extern unsigned long offline_t_B;


bool offline_charging_B;
ulong timer_Offl_B;
/****************New Offline Functions********************************/

void EVSE_B_stopOfflineTxn()
{
  disp_evse_B = false;
  requestForRelay(STOP,2);
  getChargePointStatusService_B()->stopEvDrawsEnergy();
  getChargePointStatusService_B()->unauthorize();
  if( getChargePointStatusService_B()->getEmergencyRelayClose() == false)
  {
    requestLed(GREEN,START,2);
  }
  
  //Display transaction finished
    energymeter.begin("MeterData",false); 
  float meterStop =  energymeter.getFloat("currEnergy_B",0);
  energymeter.end();
  unsigned long stop_time = millis();
flag_txndone_off_B = true;
 #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("TRANSACTION FINISHED");
  lcd.setCursor(0, 1);
  lcd.print("KWH       :");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(float((meterStop-globalmeterstartB)/1000)));
  lcd.setCursor(0, 2);
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

 
}



void EVSE_B_startOfflineTxn(){
  offline_charging_B = true;
  disp_evse_B = true;
  requestForRelay(START,2);
  requestLed(ORANGE,START,2);
  delay(1200);
  requestLed(WHITE,START,2);
  delay(1200);
  requestLed(GREEN,START,2);
  delay(1000);
  requestLed(BLINKYGREEN,START, 2);
  Serial.println("[EVSE] EV is connected and Started charging");
  if(DEBUG_OUT) Serial.println("[EVSE] Started Drawing Energy");
  st_timeB = millis();
  offline_t_B = millis();
  lastsampledTimeB_off=now();
  energymeter.begin("MeterData",false); 
  globalmeterstartB = energymeter.getFloat("currEnergy_B",0);
  energymeter.end();
}
void EVSE_B_offline_Loop()
{
  if(offline_charging_B)
  {

         drawing_current_B = eic.GetLineCurrentB();
         if(drawing_current_B <= minCurr){
          counter_drawingCurrent_B++;
          if(counter_drawingCurrent_B > currentCounterThreshold_B){
            counter_drawingCurrent_B = 0;
            Serial.println("Stopping Session Becoz of no Current");
            offline_charging_B = false;
            Serial.println("Stopping Offline Charging by low current");
            EVSE_B_stopOfflineTxn();
            disp_evse_B = false;
             #if LCD_ENABLED
  						lcd.clear();
  						lcd.setCursor(3, 0);
  						lcd.print("No Power Drawn /"); 
						  lcd.setCursor(3, 1);
						  lcd.print("EV disconnected"); 
						  #endif
          }

         }else{
          counter_drawingCurrent_B = 0;
          currentCounterThreshold_B = 60;
          Serial.println("counter_drawingCurrent Reset");

         }
          
          emergencyRelayClose_Loop_B();

         if(DEBUG_OUT) Serial.println("[EVSE] Drawing Energy");
         
         if(millis() - t_B > 5000){

           float instantCurrrent_B = eic.GetLineCurrentB();
           float instantVoltage_B  = eic.GetLineVoltageB();

          if(getChargePointStatusService_B()->getEmergencyRelayClose() == true){
              offline_charging_B = false;
              Serial.println("Stopping Offline Charging by emergency");
              EVSE_B_stopOfflineTxn();
              disp_evse_B = false;
            }

           if(millis() - relay_timer_B > 15000 && offline_charging_B == true){
            
            requestForRelay(START,2);
            relay_timer_B = millis();
            
            if( getChargePointStatusService_B()->getEmergencyRelayClose() == false)
            {
              requestLed(BLINKYGREEN,START,2);
            }

           }
           
          time_t sampledTimeB_off = now();
          time_t delta = sampledTimeB_off - lastsampledTimeB_off;
          energymeter.begin("MeterData",false);  
          float lastEnergy_B = energymeter.getFloat("currEnergy_B",0);
  	      float finalEnergy_B = lastEnergy_B + ((float)(instantVoltage_B * instantCurrrent_B * ((float)delta)))/3600; //Whr
          	
            //placing energy value back in EEPROM
          offline_charging_Enargy_B = finalEnergy_B;
  	      energymeter.putFloat("currEnergy_B",finalEnergy_B);
        	Serial.println("[EnergyCSampler] currEnergy_B: "+ String(finalEnergy_B));
          energymeter.end();

          lastsampledTimeB_off = sampledTimeB_off;
         
         }
         
  } 
  
}

void EVSE_B_LED_loop()
{
 
    //if not faulted and not charging then take the led status to green once every 5 seconds

if(getChargePointStatusService_B()->getEmergencyRelayClose() == false && offline_charging_B == false)
{
if(millis() -timer_green_offline_B > 8000)
{
  timer_green_offline_B = millis();
  requestLed(GREEN,START,2);
}
}
  

}


