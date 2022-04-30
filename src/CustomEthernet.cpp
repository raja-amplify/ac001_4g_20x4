#if 0
#include "CustomEthernet.h"
#include "Master.h"
#include "display.h"
#define SS_ETH 	2
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);

void ethernetSetup(){

	Serial.println(F("Enabling Ethernet"));
    Ethernet.init(SS_ETH);
    if (Ethernet.begin(mac) == 0) {
     	//Bue LED
    	Serial.println(F("Failed to configure Ethernet using DHCP"));
      	if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      		Serial.println(F("Ethernet shield was not found.  Sorry, can't run without hardware. :("));
   		 } else if(Ethernet.linkStatus() == LinkOFF) {
      		Serial.println(F("Ethernet cable is not connected."));
   		 }
     
      	// try to congifure using IP address instead of DHCP:
      	Ethernet.begin(mac, ip);
    }
      // print your local IP address:
  Serial.print(F("My IP address: "));
 	Serial.println(Ethernet.localIP());

}

bool flag_ethoffline = false;

void ethernetLoop(){

  if(Ethernet.linkStatus() != LinkON){
    Serial.println(F("[CustomEthernet] Trying to connect to Internet"));
    cloudConnect_Disp(false);
    checkForResponse_Disp();
    delay(500);
    flag_ethoffline = true;
  }else if((Ethernet.linkStatus() == LinkON) && (flag_ethoffline == true)){
   flag_ethoffline = false;
   
    Serial.println(F("Starting Ethernet Setup again"));
    ethernetSetup();
   // cloudConnect_Disp(true);
   // checkForResponse_Disp();
   // delay(500);
  }
	
	/*
  while(Ethernet.linkStatus() != LinkON){
		Serial.println("No Internet");
	//	requestLed(BLUE,START,1); // 1 means 1st connector
    cloudConnect_Disp(false);
    checkForResponse_Disp();
    delay(1000);
		Serial.println("LinkStatus == "+ String(Ethernet.linkStatus()));
		delay(500);
		
		if(Ethernet.linkStatus() == LinkON)
		ethernetSetup();
    Serial.println("Starting Ethernet Setup again");
    cloudConnect_Disp(true);
    checkForResponse_Disp();
    delay(1000);
	}
  */
}
#endif