#include "OTA.h"
#include "ChargePointStatusService.h"
#include "OcppEngine.h"
#include "Master.h"
#include <Preferences.h>
extern bool wifi_connect;
// extern bool ongoingTxn_A;
// extern bool ongoingTxn_B;
// extern bool ongoingTxn_C;
extern Preferences resumeTxn_A;
extern Preferences resumeTxn_B;
extern Preferences resumeTxn_C;
unsigned long int startTimer = 0;

void setupOTA() {
    Serial.println("[HTTP] begin...");
    
    int updateSize = 0;
    HTTPClient http;
  
    // configure server and url
    //String post_data = "{\"version\":\"AC001_India/AC001_Three_Connectors.ino.esp32\", \"deviceId\":\"AC001\"}";  //evre-iot-308216.appspot.com/ota.update/AC001_Africa
    //String post_data = "{\"version\":\"AC001_India_4G/AC001_Three_Connectors.ino.esp32\", \"deviceId\":\"AC001\"}";  //evre-iot-308216.appspot.com/ota.update/AC001_Africa
    String post_data = "{\"version\":\"AC001_India_4G/AC001_Three_Connectors20x4.ino.esp32\", \"deviceId\":\"AC001\"}";
    http.begin("https://us-central1-evre-iot-308216.cloudfunctions.net/otaUpdate");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Connection", "keep-alive");

    int httpCode = http.POST(post_data);
  
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.println( "Checking for new firmware updates..." );
    
        // If file found at server
        if(httpCode == HTTP_CODE_OK) {
            // get lenght of document (is -1 when Server sends no Content-Length header)
            int len = http.getSize();
            updateSize = len;
            Serial.printf("[OTA] Update found, File size(bytes) : %d\n", len);
    
            // get tcp stream
            WiFiClient* client = http.getStreamPtr();

            Serial.println();
            performUpdate(*client, (size_t)updateSize);   
            Serial.println("[HTTP] connection closed or file end.\n");
        }
        // If there is no file at server
        if(httpCode == HTTP_CODE_INTERNAL_SERVER_ERROR) {
            Serial.println("[HTTP] No Updates");
            Serial.println();
            //ESP.restart();
        }
    }
    http.end();
}


// perform the actual update from a given stream
void performUpdate(WiFiClient& updateSource, size_t updateSize) {
    if (Update.begin(updateSize)) {
        Serial.println("...Downloading File...");
        Serial.println();
        
        // Writing Update
        size_t written = Update.writeStream(updateSource);
        if (written == updateSize) {
            Serial.println("Written : " + String(written) + "bytes successfully");
        }
        else {
            Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
        }
        if (Update.end()) {
            Serial.println("OTA done!");
            if (Update.isFinished()) {
                Serial.println("Update successfully completed. Rebooting...");
                Serial.println();
                ESP.restart();
            }
            else {
                Serial.println("Update not finished? Something went wrong!");
            }
        }
        else {
            Serial.println("Error Occurred. Error #: " + String(Update.getError()));
        }
    }
    else{
        Serial.println("Not enough space to begin OTA");
    }
}

short resetCounter =0;
void ota_Loop(){

	if(millis() - startTimer > 43200000){

		if((getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Charging  && getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Preparing) &&
            (getChargePointStatusService_B()->inferenceStatus() != ChargePointStatus::Charging  && getChargePointStatusService_B()->inferenceStatus() != ChargePointStatus::Preparing)&&
            (getChargePointStatusService_C()->inferenceStatus() != ChargePointStatus::Charging  && getChargePointStatusService_C()->inferenceStatus() != ChargePointStatus::Preparing)&&
            (resumeTxn_A.getBool("ongoingTxn_A",false) == false)&&
            (resumeTxn_B.getBool("ongoingTxn_B",false) == false)&&
            (resumeTxn_C.getBool("ongoingTxn_C",false) == false)&&
            (getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Available)&&
            (getChargePointStatusService_B()->inferenceStatus() == ChargePointStatus::Available)&&
            (getChargePointStatusService_C()->inferenceStatus() == ChargePointStatus::Available)&&
            (wifi_connect == true)){
            // Serial.println(ongoingTxn_A);
            // Serial.println(ongoingTxn_B);
            // Serial.println(ongoingTxn_C);
			requestLed(BLUE,START,1); 
            requestLed(BLUE,START,2); 
            requestLed(BLUE,START,3); 
			setupOTA();
            if(resetCounter++ > 2){
                Serial.println(F("Rebooting Device in 5 sec"));
                delay(2000);
                
                ESP.restart();
            }
		//	startTimer = millis();
		} 
	}
}
