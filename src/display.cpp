#if DISPLAY_ENABLED
#include"display.h"
#include "ControlPilot.h"
#include "Master.h"
#include "ATM90E36.h"
#include "EVSE_A.h"


extern ATM90E36 eic;

#define WAIT_TIMEOUT_DISP 2500

SoftwareSerial display(27,26);
extern EVSE_states_enum EVSE_state;

StaticJsonDocument<100> data_tx;
StaticJsonDocument<100> data_rx;

void setupDisplay_Disp(){
  display.begin(9600);
  Serial.println("Starting Display");
}

void defaultScreen_Disp(){
  data_tx.clear();
  data_rx.clear();

  data_tx["start"] = "1";
  serializeJson(data_tx,Serial);
  serializeJson(data_tx,display);
  
 }

void statusOfCharger_Disp(String state){
  data_tx.clear();
  data_rx.clear();

  data_tx["status"] = state;   //(char *)
  serializeJson(data_tx,Serial);
  serializeJson(data_tx,display);
}

void authenticatingRFID_Disp(){
  data_tx.clear();
  data_rx.clear();

  data_tx["rfidtap"] = "1";
  serializeJson(data_tx,Serial);
  serializeJson(data_tx,display);
}

void authoriseStatus_Disp(bool flag){
  data_tx.clear();
  data_rx.clear();

  if(flag == true){
    data_tx["rfid"] = "1";
  }else{
    data_tx["rfid"] = "0";
  
  }
  serializeJson(data_tx,Serial);
  serializeJson(data_tx,display); 
}

//String pin ="";
bool checkForBtn(){
  data_tx.clear();
  data_rx.clear();

  bool success = false;
  bool btn = false;
  unsigned int startTime = millis();
    while(millis() - startTime < 2000){ // waiting for response from slave
      if(display.available()){  
       // Serial.println("[chkpin] Received data");
        ReadLoggingStream loggingStream(display, Serial);
        DeserializationError err = deserializeJson(data_rx, loggingStream);
        //  Serial.print(rx_doc);
        switch (err.code()) {
          case DeserializationError::Ok:
            success = true;
            break;
          case DeserializationError::InvalidInput:
            Serial.print(F(" Invalid input! Not a JSON\n"));
            break;
          case DeserializationError::NoMemory:
            Serial.print(F("Error: Not enough memory\n"));
            break;
          default:
            Serial.print(F("Deserialization failed\n"));
            break;
        }

      if (!success) {
          //propably the payload just wasn't a JSON message
          data_rx.clear();
           //serializeJson(data_tx, display);     // data send to slave again
          //return false;
      }else{
      
        btn = data_rx["btn"];

       // pin =String(pin_c);
        Serial.println("BTN Received: "+ String(btn));
        return btn;

    }
      } 

    }
    Serial.println("No btn Input from display");
  return btn;

}



int checkForPin(){
  data_tx.clear();
  data_rx.clear();

  bool success = false;
  int pin_c =0;
  unsigned int startTime = millis();
    while(millis() - startTime < 2000){ // waiting for response from slave
      if(display.available()){  
        Serial.println("[chkpin] Received data");
        ReadLoggingStream loggingStream(display, Serial);
        DeserializationError err = deserializeJson(data_rx, loggingStream);
        //  Serial.print(rx_doc);
        switch (err.code()) {
          case DeserializationError::Ok:
            success = true;
            break;
          case DeserializationError::InvalidInput:
            Serial.print(F(" Invalid input! Not a JSON\n"));
            break;
          case DeserializationError::NoMemory:
            Serial.print(F("Error: Not enough memory\n"));
            break;
          default:
            Serial.print(F("Deserialization failed\n"));
            break;
        }

      if (!success) {
          //propably the payload just wasn't a JSON message
          data_rx.clear();
           //serializeJson(data_tx, display);     // data send to slave again
          //return false;
      }else{
      
        pin_c = data_rx["pin"];
        Serial.println("PIN Received"+pin_c);
        return pin_c;
    }
      } 

    }
    Serial.println("No pin Input from display");
  return false;

}


void displayEnergyValues_Disp(String voltage, String current, String energy){
  data_tx.clear();
  data_rx.clear();

  data_tx["display"] = "values";
  data_tx["v"] = voltage;
  data_tx["i"] = current;
  data_tx["e"] = energy;

  serializeJson(data_tx,Serial);
  serializeJson(data_tx,display);

}

//void notify_Disp(String){}

void thanks_Disp(String value){
  data_tx.clear();
  data_rx.clear();

  data_tx["thank"] = value;

  serializeJson(data_tx,Serial);
  serializeJson(data_tx,display);


}

void cloudConnect_Disp(bool flag){
  data_tx.clear();
  data_rx.clear();

  if(flag == true){
    data_tx["server"] = "1";
  }else{
    data_tx["server"] = "0";
  }
  serializeJson(data_tx,Serial);
  serializeJson(data_tx,display);
}

bool checkForResponse_Disp(){
  bool status_disp = false;
  bool success = false;
  int startTime = millis();
    while(millis() - startTime < WAIT_TIMEOUT_DISP){ // waiting for response from slave
      if(display.available()){ 
        ReadLoggingStream loggingStream(display, Serial);
        DeserializationError err = deserializeJson(data_rx, loggingStream);
        //  Serial.print(rx_doc);
        switch (err.code()) {
          case DeserializationError::Ok:
            success = true;
            break;
          case DeserializationError::InvalidInput:
            Serial.print(F(" Invalid input! Not a JSON\n"));
            break;
          case DeserializationError::NoMemory:
            Serial.print(F("Error: Not enough memory\n"));
            break;
          default:
            Serial.print(F("Deserialization failed\n"));
             serializeJson(data_rx,Serial); //Display is sending NONULL for status
            break;
        }

      if (!success) {
          //propably the payload just wasn't a JSON message
          
          data_rx.clear();
          serializeJson(data_tx, display);     // data send to slave again
          //return false;
      }else{
        status_disp = data_rx["response"];
        Serial.println("OK");
        return status_disp;

    }
      } 

    }
    Serial.println("No response from display");
  return false;
}

/****************************************************/
//#define STARTTXNHEARTBEAT 60000
extern bool flag_stopOnlineCommunication;
extern bool flag_masterPin_Disp;
extern EVSE_states_enum EVSE_state;
//extern OnSendHeartbeat onSendHeartbeat;


void startmastertxn(){
  ulong starttxn = millis();
  int counter_master = 0;
  int counter_error = 0;
  float current = 0;
  int counter_disconnect = 0;
  float power = 0;
  bool flag_chargingM = false;

  authoriseStatus_Disp(true);
  checkForResponse_Disp();
  delay(500);
  while(flag_stopOnlineCommunication == true){

    ControlP_loop();
  
    if(EVSE_state == STATE_C || EVSE_state == STATE_D){

      if(flag_chargingM == false){
        requestForRelay(START,1);
        flag_chargingM = true;
      }
      statusOfCharger_Disp("CHARGING ");
      checkForResponse_Disp();
      delay(500);
      current = (float)eic.GetLineCurrentA();
      if(current > 0.1){
        power = (float)(eic.GetLineCurrentA() * eic.GetLineVoltageA())/1000.0;
      }else{
        power = 0;
      }
      displayEnergyValues_Disp(String(eic.GetLineVoltageA()), String(eic.GetLineCurrentA()), String(power));
      checkForResponse_Disp();
      
      Serial.println("[EVSE_M_P] EV is connected and charging");
      if(DEBUG_OUT) Serial.println("[EVSE_M_P] Drawing Energy");
    }else if(EVSE_state == STATE_SUS){
      flag_chargingM = false;
      statusOfCharger_Disp("SUSPENDED EV ");
      checkForResponse_Disp();
      delay(500);
    //  requestForRelay(STOP,1);
      Serial.println("[EVSE_M_P] SUSPENDED");
      counter_master++;
      if(counter_master > 20){
        requestforCP_OUT(STOP);  //stop session  @pwm
        flag_stopOnlineCommunication = false;
      }
    }else if(EVSE_state == STATE_DIS || EVSE_state == STATE_E){ //thank you
      flag_chargingM = false;
      statusOfCharger_Disp("DISCONNECTED ");
      checkForResponse_Disp();
      delay(500);
      if(counter_disconnect++ > 1){
        requestforCP_OUT(STOP);  //stop session @pwm
        requestForRelay(STOP,1);
        flag_stopOnlineCommunication = false;
      }
      Serial.println("[EVSE_M_P] Disconnected");
    }else if(EVSE_state == STATE_A || EVSE_state == STATE_B){ //this state will never achieve
      flag_chargingM = false;
      counter_error++;
      if(counter_error > 5){
        requestForRelay(STOP,1);
        requestforCP_OUT(STOP); 
        flag_stopOnlineCommunication = false;
        Serial.println("[EVSE_M_P]Error State A|B");
      }
    }

    delay(500);
    // if (millis() - starttxn >= STARTTXNHEARTBEAT){
    //   starttxn = millis();
    //   onSendHeartbeat();  //so that device is connected to internet
    // }


  }
  Serial.println("Thank You!!! \nTransaction Completed");
  Serial.println("[masterpin] Ending master_pin txn");
  thanks_Disp("");
  delay(2000);
  defaultScreen_Disp();
}


bool checkForResponse(){
	bool status = false;
	bool success =false;
	int startTime = millis();
	  while(millis() - startTime < WAIT_TIMEOUT_DISP){ // waiting for response from slave
	    if(display.available()){ 
	      ReadLoggingStream loggingStream(display, Serial);
	      DeserializationError err = deserializeJson(data_rx, loggingStream);
	      //  Serial.print(rx_doc);
	      switch (err.code()) {
	        case DeserializationError::Ok:
	          success = true;
	          break;
	        case DeserializationError::InvalidInput:
	          Serial.print(F(" Invalid input! Not a JSON\n"));
	          break;
	        case DeserializationError::NoMemory:
	          Serial.print(F("Error: Not enough memory\n"));
	          break;
	        default:
	          Serial.print(F("Deserialization failed\n"));
	          break;
	      }

     	if (!success) {
      		//propably the payload just wasn't a JSON message
      		data_rx.clear();
      		serializeJson(data_tx, display);     // data send to slave again
      		//return false;
    	}else{
    		status = data_rx["response"];
    		return status;

		}
  	}	

 	}
	return false;
}

#endif