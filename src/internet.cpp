#include "internet.h"

void wifi_Loop();
void cloudConnectivityLed_Loop();

extern bool isInternetConnected;
extern TinyGsmClient client;
extern WebSocketsClient webSocket;

extern bool wifi_connect;
extern bool wifi_enable;

extern bool gsm_connect;
extern bool gsm_enable;

int counter_gsmconnect = 0;
int counter_wifiConnect = 0;


void internetLoop(){

	if(wifi_enable == true && gsm_enable == true){
		//
		wifi_gsm_connect();

	}else if(wifi_enable == true){
		//
		wifi_connect = true;
		gsm_connect  = false;
		wifi_Loop();
		webSocket.loop();
		// cloudConnectivityLed_Loop();

	}else if(gsm_enable == true){

		gsm_connect = true;
		wifi_connect = false;  //redundant
		gsm_Loop();
	}

}



void wifi_gsm_connect(){

	if(wifi_connect == true){
		gsm_connect = false;
		Serial.println(F("[wifi_gsm_connect] WiFi"));
		wifi_Loop();
		webSocket.loop();
		// cloudConnectivityLed_Loop();
		if(WiFi.status() != WL_CONNECTED || isInternetConnected == false){
			Serial.println("[wifi] counter_wifiConnect"+ String(counter_wifiConnect));
			counter_wifiConnect++;
			if(counter_wifiConnect > 50){
				Serial.println(F("Switching To gsm"));
				WiFi.disconnect();
				counter_wifiConnect = 0;
				wifi_connect = false;
				gsm_connect = true;
			}

		}else{
			counter_wifiConnect = 0;
			Serial.println(F("default Counter_wifiConnect"));
		}
	}else if(gsm_connect == true){
		wifi_connect  = false;
		Serial.println(F("[wifi_gsm_connect] GSM "));
		gsm_Loop();

		if(!client.connected()){
			Serial.println("[gsm] counter_gsmconnect:"+ String(counter_gsmconnect));
			if(counter_gsmconnect++ > 1){   //almost 5 min
				counter_gsmconnect = 0;
				Serial.println(F("Switching to WIFI"));
				wifi_connect = true;
				gsm_connect = false;
			}

		}



	}



}