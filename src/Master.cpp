#include "Master.h"
#include "Variants.h"

SoftwareSerial masterSerial(25,33); //25 Rx, 33 Tx
StaticJsonDocument<250> txM_doc;
StaticJsonDocument<250> rxM_doc;

extern int8_t button;

void customflush(){
  while(masterSerial.available() > 0)
    masterSerial.read();
}

#if 0
/*
  @brief : Read the touch display
*/
#if DWIN_ENABLED
int8_t dwin_input()
{

  button = DWIN_read();
  Serial.printf("Button pressed : %d", button);
  //delay(50);
  return button;

}
#endif
#endif 

int requestConnectorStatus(){
  txM_doc.clear();
  rxM_doc.clear();

  
  const char* type = "";
  int connectorId = 0;
  int startTime = 0;
  bool success = false;
  
  Serial.println("Master: ");
  txM_doc["type"] = "request";
  txM_doc["object"] = "connector";
  
  serializeJson(txM_doc,masterSerial);     //data send to slave   
  serializeJson(txM_doc, Serial);
  delay(100);
  
  startTime = millis();
  while(millis() - startTime < 20000/*WAIT_TIMEOUT*/){ // waiting for response from slave
  
    if(masterSerial.available()){
      
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
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
      rxM_doc.clear();
      delay(200);
      customflush();
      Serial.println("***Sending request again***");
      delay(100);
      serializeJson(txM_doc, masterSerial);     // data send to slave again
      serializeJson(txM_doc, Serial);
      //return false;
    }else{
      type = rxM_doc["type"] | "Invalid";
      connectorId = rxM_doc["connectorId"];
      if(strcmp(type , "response") == 0){
        Serial.println("Received connectorId from slave--->" + String(connectorId));
        return connectorId;
      }
    }
   }
   
 }
  Serial.println(F("No response from Slave"));
  return connectorId;  //returns 0 if connectorId (push button is not pressed)
  
}


bool requestForRelay(int action , int connectorId){
  
  if(connectorId == 0 || connectorId > 3){
    return false;
  }

  // DynamicJsonDocument txM_doc(200);
  // DynamicJsonDocument rxM_doc(200);

  txM_doc.clear();
  txM_doc.clear();

  const char* type = "";
  const char* object = "";
  
  //const char* statusRelay = "";
  int statusRelay=0;
  bool success = false;
  int startTime = 0;
  
  Serial.println("Relay:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "relay";
  
  txM_doc["action"] = action;
  txM_doc["connectorId"] = connectorId;

  serializeJson(txM_doc,masterSerial);     //data send to slave   
  serializeJson(txM_doc, Serial);
  delay(2000);

  startTime = millis();
  while(millis() - startTime < 6000){ // waiting for response from slave
    if(masterSerial.available()){ 
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
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
      rxM_doc.clear();
      delay(200);
      customflush();
      Serial.println("***Sending request again***");
      delay(100);
      serializeJson(txM_doc, masterSerial);     // data send to slave again
      //return false;
    }else{
      type = rxM_doc["type"] | "Invalid";
      statusRelay = rxM_doc["status"];
      connectorId = rxM_doc["connectorId"];
      if((strcmp(type , "response") == 0) && (action == statusRelay)){
        Serial.println("Received Status--->" +String(statusRelay)+" for ConnectorId: "+ String(connectorId));
        return true;
      }
    }
   }//end of .available 
  }//end of while loop
  Serial.println(F("No response from Slave"));
  return false;
}

bool requestLed(int colour, int action , int connectorId){
  
  if(connectorId == 0 || connectorId > 4){
    return false;
  }

  // DynamicJsonDocument txM_doc(200);
  // DynamicJsonDocument rxM_doc(200);
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  const char* object = "";
  
  //const char* statusRelay = "";
  int statusLed=0;
  bool success = false;
  int startTime = 0;
  
  Serial.println("Led:");
  txM_doc["type"] = "request";
  if(connectorId == 4){
    txM_doc["object"] = "rfid";
  }else{
    txM_doc["object"] = "led";
  }
  txM_doc["colour"] = colour;
  txM_doc["action"] = action;
  txM_doc["connectorId"] = connectorId;

  serializeJson(txM_doc,masterSerial);     //data send to slave   
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while(millis() - startTime < WAIT_TIMEOUT){ // waiting for response from slave
    if(masterSerial.available()){ 
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
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
      rxM_doc.clear();
      delay(200);
      customflush();
      Serial.println("***Sending request again***");
      delay(100);
      serializeJson(txM_doc, masterSerial);     // data send to slave again
      //return false;
    }else{
      type = rxM_doc["type"] | "Invalid";
      connectorId = rxM_doc["connectorId"];
      statusLed = rxM_doc["status"];
      if((strcmp(type , "response") == 0) && (action == statusLed)){
        Serial.println("Received Status--->" +String(statusLed)+" for ConnectorId: "+ String(connectorId));
        return true;
      }
    }
   }//end of .available 
  }//end of while loop
  Serial.println(F("TimeOut"));
  return false;
}

bool requestEmgyStatus(){

  // DynamicJsonDocument txM_doc(100);
  // DynamicJsonDocument rxM_doc(100);
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  const char* object = "";
  bool success = false;
  int startTime = 0;
  bool statusE = 0;
  Serial.println("EMGY:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "emgy";

  serializeJson(txM_doc,masterSerial);     //data send to slave   
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while(millis() - startTime < WAIT_TIMEOUT){ // waiting for response from slave
    if(masterSerial.available()){ 
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
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
      rxM_doc.clear();
      delay(200);
      customflush();
      Serial.println("***Sending request again***");
      delay(100);
      serializeJson(txM_doc, masterSerial);     // data send to slave again
      //return false;
    }else{
      type = rxM_doc["type"] | "Invalid";
      object = rxM_doc["object"] | "Invalid";
      statusE = rxM_doc["status"];
      if((strcmp(type , "response") == 0) && (strcmp(object,"emgy") == 0)){
        Serial.println("Received Status--->" +String(statusE));
        if(statusE == true){ 
          return true;
        }else if(statusE ==false){
          return false;
        }
      }
     }
   }//end of .available 
  }//end of while loop
  Serial.println(F("TimeOut"));
  return false;
}



//CP
bool requestforCP_OUT(int action){

  // DynamicJsonDocument txM_doc(200);
  // DynamicJsonDocument rxM_doc(200);
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  const char* object = "";
  
  bool success = false;
  int startTime = 0;
  bool status = 0;
  
  Serial.println("Control Pilot:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "cpout";
  
  txM_doc["action"] = action;
//  txM_doc["connectorId"] = connectorId;   future Implemnetation

  serializeJson(txM_doc,masterSerial);     //data send to slave   
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while(millis() - startTime < WAIT_TIMEOUT){ // waiting for response from slave
    if(masterSerial.available()){ 
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
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
      rxM_doc.clear();
      delay(200);
      customflush();
      Serial.println("***Sending request again***");
      delay(100);
      serializeJson(txM_doc, masterSerial);     // data send to slave again
      //return false;
    }else{
      type = rxM_doc["type"] | "Invalid";
      object = rxM_doc["object"] | "Invalid";
      status = rxM_doc["status"];
      if((strcmp(type , "response") == 0) && (strcmp(object,"cpout") == 0)){
        Serial.println("Received Status--->" +String(status));
        return true;
      }
    }
   }//end of .available 
  }//end of while loop
  Serial.println(F("No response from Slave"));
  return false;
}

int requestforCP_IN(){

  // DynamicJsonDocument txM_doc(200);
  // DynamicJsonDocument rxM_doc(200);
  txM_doc.clear();
  rxM_doc.clear();

  const char* type = "";
  const char* object = "";
  
  bool success = false;
  int startTime = 0;
  int value = 0;
  
  Serial.println("Control Pilot:");
  txM_doc["type"] = "request";
  txM_doc["object"] = "cpin";
  
//  txM_doc["connectorId"] = connectorId;   future Implemnetation

  serializeJson(txM_doc,masterSerial);     //data send to slave   
  serializeJson(txM_doc, Serial);
  delay(100);

  startTime = millis();
  while(millis() - startTime < WAIT_TIMEOUT){ // waiting for response from slave
    if(masterSerial.available()){ 
      ReadLoggingStream loggingStream(masterSerial, Serial);
      DeserializationError err = deserializeJson(rxM_doc, loggingStream);
      //  Serial.print(rxM_doc);
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
      rxM_doc.clear();
      delay(200);
      customflush();
      serializeJson(txM_doc, masterSerial);     // data send to slave again
      //return false;
    }else{
      type = rxM_doc["type"] | "Invalid";
      object = rxM_doc["object"] | "Invalid";
      value = rxM_doc["value"];
      if((strcmp(type , "response") == 0) && (strcmp(object,"cpin") == 0)){
        Serial.println("Received value--->" +String(value));
        return value;
      }
    }
   }//end of .available 
  }//end of while loop
  Serial.println(F("No response from Slave"));
  return false;
}



void Master_setup() {
  // put your setup code here, to run once:
  
 // Serial.begin(115200);
  masterSerial.begin(9600, SWSERIAL_8N1, 25,33, false,256);// Required to fix the deserialization error
  Serial.println(F("***EVRE***AC001****BEGIN***"));
  Serial.println(F("***VERSION HISTORY***"));
  Serial.println(F("4G support - G. Raja Sumant 21/04/2022")); 
  Serial.println(F("Reason for stop - G. Raja Sumant 31/05/2022")); 
  Serial.println(F("20x4,DWIN, arduino uno - G. Raja Sumant 31/05/2022")); 
  Serial.println(F("Firmware version in the boot notification - G. Raja Sumant 31/05/2022")); 
  Serial.print("[MASTER] ESP32 Board MAC Address:  ");
  Serial.println(WiFi.macAddress());

}

/*
void loop() {
  // put your main code here, to run repeatedly:
   //requestLed(RED,START,1);

  bool emgcy = requestEmgyStatus();
  Serial.println("Emergency Button Status--> " +String(emgcy));
  if(emgcy == 0){
    for(;;){
      
      Serial.println("EMGY is pressed");
       bool emgcy = requestEmgyStatus();
       if(emgcy ==1) break;
      }
  }
  int connector = requestConnectorStatus();
  Serial.println("Connector: " + String(connector)+ "\n");
  delay(1000);
  if(connector > 0){
   bool statusRelay = requestForRelay(START,connector);
   Serial.println("statusRelay--->" + String(statusRelay));
  
  delay(1000);

  bool ch1 = requestForRelay(STOP,connector);
  Serial.println("ch1 --->" +String(ch1));

  delay(1000);

  bool ch2 = requestLed(RED,START,connector);
  Serial.println("ch2--->" +String(ch2));

  delay(1000);

  requestLed(GREEN,START,connector);

  delay(1000);

    requestLed(BLUE,START,connector);

  delay(1000);
    requestLed(WHITE,START,connector);

  delay(1000);
    requestLed(RED,STOP,connector);

  //  delay(1000);
   // requestLed(RED,START,connector);
  }
}
*/