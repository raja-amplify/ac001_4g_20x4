// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef VARIANTS_H
#define VARIANTS_H

/*****************/
/*
In order to Enable Ethernet/WiFi, 
* Edit this "virtual void begin(uint16_t port=0) =0;" for WIFI :C:\Users\Mr Cool\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.6\cores\esp32\Server.h
* Set proper flag in OCPP_ETH_WIFI file kept in arduinoWebsockets/src
*/

//3.1 Concurrent fix

#define VERSION             String("AC001_4_3")
#define GSM_ENABLED   		 0
#define WIFI_ENABLED 		 1
#define ETHERNET_ENABLED 	 0

#define BLE_ENABLE  		 1

#define CP_ACTIVE 			 0

#define DISPLAY_ENABLED		 0

#define MASTERPIN_ENABLED    0

#define LED_ENABLED 		 1

#define LCD_ENABLED		     0  

#define EARTH_DISCONNECT     0


#define DWIN_ENABLED		 0   

#define EVSECEREBRO 		 1
#define STEVE                0

/*****************/
#define DEBUG_OUT true
#define DEBUG_EXTRA false
#define DEBUG_OUT_M true

//#define OCPP_SERVER //comment out if this should be compiled as server <--- needs to be implemented again

#endif
