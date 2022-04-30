#ifndef MASTER_H
#define MASTER_H
#include<ArduinoJson.h>
#include<SoftwareSerial.h>
#include <StreamUtils.h>
#include<WiFi.h>
/* int requestConnectorStatus(); returns the connector Id.
 * {"type":"request","object":"connector"} ::: {"type":"response","object":"connector","connectorId": int}
 * 
 * bool requestForRelay(int, int)  returns bool and takes start/stop as 0/1 and connector Id
 * {"type":"request","object":"relay","action":"on/off","connectorId":int}  ::: {"type":"response","object":"relay","status":"on/off","connectorId":int}
 * 
 * bool requestLed(int, int , int); returns bool and takes string of red/green/blue and action(solid/ blinky) and connector Id
 * {"type":"request","object":"led","colour":"red/green/blue","action":"on/off","connectorId":int} ::: {"type":"response","object":"led","colour":"red/green/blue","status":"on/off","connectorId":int}
 * 
 * bool requestEmgyStatus();
 * {"type":"request","object":"emgy"}  ::: {"type":"response","object":"emgy","status":"on/off"}
 * 
 *int requestforCP_IN();     //connector ID is future Implementation @anesh
 * {"type":"request","object":"cpin"} ::: {"type":"response","object":"cpin","value":int}

 *bool requestCP_OUT(int);
 *{"type":"request","object":"cpout","action":"on/off"} ::: {"type":"response","object","cpout","status":"on/off"}
 */
bool requestEmgyStatus();
bool requestLed(int colour, int action , int connectorId);
bool requestForRelay(int action , int connectorId);
int requestConnectorStatus();
void Master_setup();
int requestforCP_IN();
bool requestforCP_OUT(int);


#define WAIT_TIMEOUT 		5000
#define START 				1    //ON
#define STOP  				0    //OFF

#define RED    				00
#define GREEN  				01
#define BLUE   				02
#define WHITE  				03
#define ORANGE				9
#define VOILET				10

//Blinky
//Blinking
#define BLINKYGREEN 	05
#define BLINKYBLUE  	06
#define BLINKYRED   	07
#define BLINKYWHITE     11
#define BLINKYWHITE_ALL 12
#define BLINKYBLUE_ALL  13

//#define SOLID  0
//#define BLINKY 1

#endif