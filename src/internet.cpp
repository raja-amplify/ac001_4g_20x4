#include "internet.h"

#define DEBUG_INTERNET true
#define INTERNET_CHECK_PERIOD 		600000 //1800000   //Every 30 min we will check for internet
#define INTERNET_CONNECTION_TRY		300000		//Keep try connecting to Internet

void wifi_Loop();
void cloudConnectivityLed_Loop();

extern bool isInternetConnected;
extern TinyGsmClient client;
extern WebSocketsClient webSocket;

extern bool wifi_connect;
extern bool wifi_enable;

extern bool gsm_connect;
extern bool gsm_enable;

extern bool flag_internet;
extern bool flag_offline;

int counter_gsmconnect = 0;
int counter_wifiConnect = 0;

ulong timer1;
ulong timer2;


bool flag_Timer2Initialized;

bool checkInternet(){
	if(DEBUG_INTERNET) Serial.println("Internet check.....");

	if(millis() - timer1 > INTERNET_CHECK_PERIOD){

		if(flag_Timer2Initialized == false){
			timer2 = millis();
			if(DEBUG_INTERNET) Serial.println("Initialized Timer 2");
			flag_Timer2Initialized = true;
		}

		internetLoop();
		flag_offline = false;


		if(millis() - timer2 > INTERNET_CONNECTION_TRY){
			timer1 = millis();
			flag_Timer2Initialized = false;

			Serial.println("Initializing Timer 1");
		}

		return true;

	} 

	return false;

}




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