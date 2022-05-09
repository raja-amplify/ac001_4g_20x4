/*
   4G support added - 21/04/2022

   Added DWIN display support.
   Fixed the status of NOT_SET 09
  <POD without Control Pilot Support>
  The following code is developed by Pulkit Agrawal & Wamique.
  Added Master-Slave files
  Added EnergyMeter Fix
*/

//ESP32 Libraries
#include <Arduino.h>
#include "src/libraries/arduinoWebSockets-master/src/WebSocketsClient.h"
//#include <ArduinoJson.h>
#include "src/Peripherals.h"

#if WIFI_ENABLED
#include <WiFi.h>
//#define SSIDW 	"Amplify Mobility_PD"
//#define PSSWD 	"Amplify5"
#endif

//OCPP Message Generation Class
#include "src/OcppEngine.h"
//#include "src/SmartChargingService.h"
#include "src/ChargePointStatusService.h"
#include "src/MeteringService.h"
#include "src/GetConfiguration.h"
#include "src/TimeHelper.h"
#include "src/SimpleOcppOperationFactory.h"
#include "src/EVSE_A.h"
#include "src/EVSE_B.h"
#include "src/EVSE_C.h"

//Master Class
#include"src/Master.h"

//Power Cycle
#include"src/OTA.h"
#include"src/internet.h"

#if CP_ACTIVE
//Control Pilot files
#include "src/ControlPilot.h"
#endif

//Gsm Files

#include "src/CustomGsm.h"
extern TinyGsmClient client;

#include"src/urlparse.h"

//20x4 lcd display

#if LCD_ENABLED
#include "src/LCD_I2C.h"

LCD_I2C lcd(0x27, 20, 4); // Default address of most PCF8574 modules, change according
#endif

#if DWIN_ENABLED
#include "src/dwin.h"
int8_t button = 0;
extern unsigned char ct[22];//connected
extern unsigned char nct[22];//not connected
extern unsigned char et[22];//ethernet
extern unsigned char wi[22];//wifi
extern unsigned char tr[22];//tap rfid
extern unsigned char utr[22];//rfid unavailable
extern unsigned char g[22];//4g
extern unsigned char clu[22];//connected
extern unsigned char clun[22];//not connected
extern unsigned char avail[22]; //available
extern unsigned char not_avail[22]; // not available
extern unsigned char change_page[10];
extern unsigned char tap_rfid[30];
extern unsigned char clear_tap_rfid[30];
#endif

WebSocketsClient webSocket;

//SmartChargingService *smartChargingService;
ChargePointStatusService *chargePointStatusService_A;
ChargePointStatusService *chargePointStatusService_B;
ChargePointStatusService *chargePointStatusService_C;

//Mertering Service declarations
MeteringService *meteringService;
ATM90E36 eic(5);
#define SS_EIC 5          //GPIO 5 chip_select pin
SPIClass * hspi = NULL;


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
bool webSocketConncted = false;

//RFID declarations
#define MFRC_RST    22
#define MFRC_SS     15
MFRC522 mfrc522(MFRC_SS, MFRC_RST); // Create MFRC522 instance
SPIClass *hspiRfid = NULL;

String currentIdTag;

//Bluetooth
#include "src/bluetoothConfig.h"
#define TIMEOUT_BLE 60000
extern BluetoothSerial SerialBT;
bool isInternetConnected = true;

bool flagswitchoffBLE = false;
int startBLETime = 0;
String ws_url_prefix_m = "";
String host_m = "";
int port_m = 0;
//String protocol_m = "";
String protocol = "ocpp1.6";
String key_m = "";
String ssid_m = "";
String path_m;

extern Preferences preferences;
String url_m = "";
Preferences resumeTxn;
String idTagData_m = "";
bool ongoingTxn_m = false;

//Led timers
ulong timercloudconnect = 0;
void wifi_Loop();

#define NUM_OF_CONNECTORS 3

//internet
bool wifi_enable = false;
bool gsm_enable  = false;

bool wifi_connect = false;
bool gsm_connect = false;



void setup() {
  //Test LED
  //  pinMode(16,OUTPUT);
  Serial.begin(115200);
  Master_setup();
  //https://arduino-esp8266.readthedocs.io/en/latest/Troubleshooting/debugging.html
  Serial.setDebugOutput(true);
#if LCD_ENABLED
  lcd.begin(true, 26, 27); // If you are using more I2C devices using the Wire library use lcd.begin(false)
  // this stop the library(LCD_I2C) from calling Wire.begin()
  lcd.backlight();
#endif


  if (DEBUG_OUT) Serial.println();
  if (DEBUG_OUT) Serial.println();
  if (DEBUG_OUT) Serial.println();

#if LCD_ENABLED
  lcd.setCursor(4, 0); // Or setting the cursor in the desired position.
  lcd.print("**BOOTING**"); // You can make spaces using well... spaces
  lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
  lcd.print("####################");
#endif

#if DWIN_ENABLED
  uint8_t err = 0;
  dwin_setup();
  change_page[9] = 0;
  err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0])); // page 0
  delay(50);
  err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0])); // status not available
  delay(50);
  err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0])); // cloud: not connected
  delay(50);
#endif

  for (uint8_t t = 4; t > 0; t--) {
    if (DEBUG_OUT) Serial.print(F("[SETUP] BOOT WAIT...Reconnect fix & GSM fix: "));
    if (DEBUG_OUT) Serial.println(VERSION);
    Serial.flush();
    delay(500);
  }

  requestLed(BLINKYWHITE_ALL, START, 1);
  requestForRelay(STOP, 1);
  requestForRelay(STOP, 2);
  requestForRelay(STOP, 3);

  requestforCP_OUT(STOP);

#if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("PREPARING");
  lcd.setCursor(5, 3);
  lcd.print("PLEASE WAIT");
#endif

#if BLE_ENABLE
  startingBTConfig();
#endif
  /************************Preferences***********************************************/
  /* preferences.begin("credentials",false);

    ws_url_prefix_m = preferences.getString("ws_url_prefix",""); //characters
    if(ws_url_prefix_m.length() > 0){
   	Serial.println("Fetched WS URL success: " + String(ws_url_prefix_m));
    }else{
   	Serial.println("Unable to Fetch WS URL / Empty");
    }
    delay(100);

    host_m = preferences.getString("host","");
    if(host_m.length() > 0){
   	Serial.println("Fetched host data success: "+String(host_m));
    }else{
   	Serial.println("Unable to Fetch host data / Empty");
    }
    delay(100);

    port_m = preferences.getInt("port",0);
    if(port_m>0){
   	Serial.println("Fetched port data success: "+String(port_m));
    }else{
   	Serial.println("Unable to Fetch port Or port is 0000");
    }
    delay(100);

    protocol_m = preferences.getString("protocol","");
    if(protocol_m.length() > 0){
   	Serial.println("Fetched protocol data success: "+String(protocol_m));
    }else{
   	Serial.println("Unable to Fetch protocol");
    }*/
  urlparser();

  ssid_m = preferences.getString("ssid", "");
  if (ssid_m.length() > 0) {
    Serial.println("Fetched SSID: " + String(ssid_m));
  } else {
    Serial.println(F("Unable to Fetch SSID"));
  }

  key_m = preferences.getString("key", "");
  if (key_m.length() > 0) {
    Serial.println("Fetched Key: " + String(key_m));
  } else {
    Serial.println(F("Unable to Fetch key"));
  }

  wifi_enable = preferences.getBool("wifi", 0);
  Serial.println("Fetched Wifi data: " + String(wifi_enable));

  gsm_enable = preferences.getBool("gsm", 0);
  Serial.println("Fetched Gsm data: " + String(gsm_enable));

  //WiFi
  wifi_connect = wifi_enable;
  gsm_connect  = gsm_enable;
  bool internet = false;
  int counter_wifiNotConnected = 0;
  int counter_gsmNotConnected = 0;

  if (wifi_enable == true) {

    WiFi.begin(ssid_m.c_str(), key_m.c_str());
  }

  //WiFi
  while (internet == false) {
    Serial.println(F("Internet loop"));
#if DWIN_ENABLED
    err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
    delay(50);
    err = DWIN_SET(clun, sizeof(clun) / sizeof(clun[0]));
    delay(50);
#endif
    bluetooth_Loop();
    if (wifi_enable == true && wifi_connect == true) {
      Serial.println(F("Waiting for WiFi Connction..."));

      if (WiFi.status() == WL_CONNECTED) {
        internet = true;
        gsm_connect = false;
        Serial.println(F("Connected via WiFi"));
#if LCD_ENABLED
        lcd.clear();

        lcd.setCursor(3, 2);
        lcd.print("STATUS: WIFI");
#endif
#if DWIN_ENABLED
        //Cloud : WiFi
        err = DWIN_SET(wi, sizeof(wi) / sizeof(wi[0]));
        delay(50);
        err = DWIN_SET(wi, sizeof(wi) / sizeof(wi[0]));
        delay(50);
        err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
        delay(50);
        err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
        delay(50);
#endif
        delay(100);
        connectToWebsocket();
      } else if (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
        bluetooth_Loop();
        wifi_Loop();
        Serial.println("Wifi Not Connected: " + String(counter_wifiNotConnected));
#if DWIN_ENABLED
        err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
        delay(50);
        err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
        delay(50);
#endif
        if (counter_wifiNotConnected++ > 50) {
          counter_wifiNotConnected = 0;

          if (gsm_enable == true) {
            WiFi.disconnect();
            wifi_connect = false;
            gsm_connect = true;
          }
        }

      }
    } else if (gsm_enable == true && gsm_connect == true) {
      // SetupGsm();                                     //redundant @optimise
      // ConnectToServer();
      if (!client.connected()) {
        gsm_Loop();
        bluetooth_Loop();
        Serial.println("GSM not Connected: " + String(counter_gsmNotConnected));
#if DWIN_ENABLED
        err = DWIN_SET(nct, sizeof(nct) / sizeof(nct[0]));
        delay(50);
        err = DWIN_SET(not_avail, sizeof(not_avail) / sizeof(not_avail[0]));
        delay(50);
#endif
        if (counter_gsmNotConnected++ > 1) { //2 == 5min
          counter_gsmNotConnected = 0;

          if (wifi_enable == true) {
            wifi_connect = true;
            gsm_connect = false;
          }
        }
      } else if (client.connected()) {
        internet = true;
        wifi_connect = false;
        Serial.println(F("connected via GSM"));
#if LCD_ENABLED
        lcd.clear();

        lcd.setCursor(0, 2);
        lcd.print("CONNECTED VIA 4G");
#endif
#if DWIN_ENABLED
        err = DWIN_SET(avail, sizeof(avail) / sizeof(avail[0]));
        delay(50);
        err = DWIN_SET(g, sizeof(g) / sizeof(g[0]));
        delay(50);
#endif
      }

    }


  }

  //SPI Enable for Energy Meter Read
  hspi = new SPIClass(HSPI); // Init SPI bus
  hspi->begin();
  pinMode(SS_EIC, OUTPUT); //HSPI SS Pin

  // SPI Enable for RFID
  hspiRfid = new SPIClass(HSPI);
  hspiRfid->begin();
  mfrc522.PCD_Init(); // Init MFRC522


  // Serial.println("closing preferences");
  // preferences.end();
  ocppEngine_initialize(&webSocket, 4096); //default JSON document size = 2048

  chargePointStatusService_A = new ChargePointStatusService(&webSocket);
  chargePointStatusService_B = new ChargePointStatusService(&webSocket);
  chargePointStatusService_C = new ChargePointStatusService(&webSocket);

  getChargePointStatusService_A()->setConnectorId(1);
  getChargePointStatusService_B()->setConnectorId(2);
  getChargePointStatusService_C()->setConnectorId(3);
  //  getChargePointStatusService_A()->authorize();

  EVSE_A_setup();
  EVSE_B_setup();
  EVSE_C_setup();

  meteringService = new MeteringService(&webSocket);

  //set system time to default value; will be without effect as soon as the BootNotification conf arrives
  setTimeFromJsonDateString("2021-22-12T11:59:55.123Z"); //use if needed for debugging

  if (DEBUG_OUT) Serial.println(F("Web Socket Connction..."));
  while (!webSocketConncted && wifi_connect == true) {
    Serial.print(F("*"));
    delay(50); //bit**
    webSocket.loop();
    bluetooth_Loop();
  }

  EVSE_B_initialize();
  EVSE_A_initialize();
  EVSE_C_initialize();

  Serial.println(F("End of Setup"));
  startBLETime = millis();

   err = DWIN_SET(clear_tap_rfid, sizeof(clear_tap_rfid) / sizeof(clear_tap_rfid[0]));
   delay(50);
   err = DWIN_SET(tap_rfid, sizeof(tap_rfid) / sizeof(tap_rfid[0]));
   delay(50);
}




void loop() {

  Serial.println(F("****************************************************************************S***********************************************"));
#if BLE_ENABLE
  if (millis() - startBLETime < TIMEOUT_BLE) {
    bluetooth_Loop();
    flagswitchoffBLE = true;
  } else {
    if (flagswitchoffBLE == true) {
      flagswitchoffBLE = false;
      Serial.println(F("Disconnecting BT"));
      //SerialBT.println("Wifi Connected");
      SerialBT.println(F("Disconnecting BT"));
      delay(100);
      SerialBT.flush();
      SerialBT.disconnect();
      SerialBT.end();
      Serial.println(ESP.getFreeHeap());
    }
  }
#endif

  ocppEngine_loop();

  emergencyRelayClose_Loop_A();
  emergencyRelayClose_Loop_B();
  emergencyRelayClose_Loop_C();

  EVSE_ReadInput(&mfrc522);

  EVSE_A_loop();
  EVSE_B_loop();
  EVSE_C_loop();

  internetLoop();

  cloudConnectivityLed_Loop();
  ota_Loop();

  getChargePointStatusService_A()->loop();
  getChargePointStatusService_B()->loop();
  getChargePointStatusService_C()->loop();

  meteringService->loop();



#if CP_ACTIVE
  ControlP_loop();
#endif
  Serial.println("Wsbsocket: " + String(webSocketConncted));
  Serial.println(F("FREE HEAP"));
  Serial.println(ESP.getFreeHeap());
  Serial.println(F("\n*********************************************************E**************************************"));


}

/***************************************EVSE_READINPUT BLOCK*********************************************************/
String readIdTag = "";
void EVSE_ReadInput(MFRC522* mfrc522) {    // this funtion should be called only if there is Internet
  readIdTag = "";
  unsigned long tout = millis();
  uint8_t readConnectorVal = 0;
  readIdTag = readRfidTag(true, mfrc522);
  if (readIdTag.equals("") == false) {
    //EVSE_StopTxnRfid(readIdTag);
    //readConnectorVal = requestConnectorStatus();
#if DWIN_ENABLED
    change_page[9] = 3; // change to page 3 and wait for input
    uint8_t err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
    delay(10);
    err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
    delay(10);
    err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0]));
    delay(10);
    flush_dwin();
    while (millis() - tout < 15000)
    {

      readConnectorVal = dwin_input();
      if (readConnectorVal > 0) {
        bool result = assignEvseToConnector(readIdTag, readConnectorVal);
        if (result == true) {
          Serial.println(F("Attached/Detached EVSE to the requested connector"));
          break;
        } else {
          Serial.println(F("Unable To attach/detach EVSE to the requested connector"));
          //break;
        }
      } else {
        Serial.println(F("Invalid Connector Id Received"));
        //break;
        //delay(2000);
      }
    }
#endif
  }
  delay(100);
}

bool assignEvseToConnector(String readIdTag, int readConnectorVal) {
  bool status = false;
  unsigned long tout = millis();
  if (readConnectorVal == 1) {
    if (getChargePointStatusService_A()->getIdTag() == readIdTag && getChargePointStatusService_A()->getTransactionId() != -1) {
      //stop session
      Serial.println(F("[EVSE_A] Stopping Transaction with RFID TAP"));
    
      EVSE_A_StopSession();
      status = true;
    } else if (getChargePointStatusService_A()->inferenceStatus() == ChargePointStatus::Available) {
      getChargePointStatusService_A()->authorize(readIdTag, readConnectorVal);
      status = true;
    }

  } else if (readConnectorVal == 2) {
    if (getChargePointStatusService_B()->getIdTag() == readIdTag && getChargePointStatusService_B()->getTransactionId() != -1) {
      Serial.println(F("[EVSE_B] Stopping Transaction with RFID TAP"));
      EVSE_B_StopSession();
      status = true;
    } else if (getChargePointStatusService_B()->inferenceStatus() == ChargePointStatus::Available) {
      getChargePointStatusService_B()->authorize(readIdTag, readConnectorVal);    //authorizing twice needed to be improvise
      status = true;
    }
  } else if (readConnectorVal == 3) {
    if (getChargePointStatusService_C()->getIdTag() == readIdTag && getChargePointStatusService_C()->getTransactionId() != -1) {
      Serial.println(F("[EVSE_C] Stopping Transaction with RFID TAP"));
      EVSE_C_StopSession();
      status = true;
    } else if (getChargePointStatusService_C()->inferenceStatus() == ChargePointStatus::Available) {
      getChargePointStatusService_C()->authorize(readIdTag, readConnectorVal);
      status = true;
    }
  } else {
    Serial.println(F("Connector Unavailable"));
    status = false;
  }

  return status;

}

/*
   Called by Websocket library on incoming message on the internet link
*/
//#if WIFI_ENABLED || ETHERNET_ENABLED
//extern OnSendHeartbeat onSendHeartbeat;
int wscDis_counter = 0;
int wscConn_counter = 0;
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      webSocketConncted = false;
      Serial.println("Counter:" + String(wscDis_counter));
      if (DEBUG_OUT) Serial.print(F("[WSc] Disconnected!!!\n"));
      if (wscDis_counter++ > 2) {
        delay(200);
        Serial.println(F("Trying to reconnect to WSC endpoint"));
        wscDis_counter = 0;
        Serial.println("URL:" + String(path_m));
        webSocket.begin(host_m, port_m, path_m, protocol);
        while (!webSocketConncted) {                             //how to take care if while loop fails
          Serial.print(F("..**.."));
          delay(100); //bit**
          webSocket.loop();                                     //after certain time stop relays and set fault state
          if (wscConn_counter++ > 30) {
            wscConn_counter = 0;
            Serial.println(F("[Wsc] Unable To Connect"));
            break;
          }
        }
      }
      //have to add websocket.begin if websocket is unable to connect //Static variable
      break;
    case WStype_CONNECTED:
      webSocketConncted = true;
      if (DEBUG_OUT) Serial.printf("[WSc] Connected to url: %s\n", payload);
      break;
    case WStype_TEXT:
      if (DEBUG_OUT) if (DEBUG_OUT) Serial.printf("[WSc] get text: %s\n", payload);

      if (!processWebSocketEvent(payload, length)) { //forward message to OcppEngine
        if (DEBUG_OUT) Serial.print(F("[WSc] Processing WebSocket input event failed!\n"));
      }
      break;
    case WStype_BIN:
      if (DEBUG_OUT) Serial.print(F("[WSc] Incoming binary data stream not supported"));
      break;
    case WStype_PING:
      // pong will be send automatically
      if (DEBUG_OUT) Serial.print(F("[WSc] get ping\n"));
      break;
    case WStype_PONG:
      // answer to a ping we send
      if (DEBUG_OUT) Serial.print(F("[WSc] get pong\n"));
      break;
  }
}
//#endif

/*
  @brief : Read the touch display
*/
#if DWIN_ENABLED
int8_t dwin_input()
{

  button = DWIN_read();
  Serial.printf("Button pressed : %d", button);
  return button;

}
#endif
//#if WIFI_ENABLED
int wifi_counter = 0;
void wifi_Loop() {
  Serial.println(F("[WiFi_Loop]"));
  if (WiFi.status() != WL_CONNECTED) {

    if (wifi_counter++ > 2 && (WiFi.status() != WL_CONNECTED) ) {
      wifi_counter = 0;
      Serial.print(".");
      WiFi.disconnect();
      delay(500);
      Serial.println(F("[WIFI] Trying to reconnect again"));
      WiFi.begin(ssid_m.c_str(), key_m.c_str());
      delay(3000);
    }
  }
}
//#endif
short int counterPing = 0;
void cloudConnectivityLed_Loop() {


  if (wifi_connect == true) {
    if (counterPing++ >= 3) { // sending ping after every 30 sec [if internet is not there sending ping packet itself consumes 10sec]
      isInternetConnected = webSocket.sendPing();
      Serial.println("*Sending Ping To Server: " + String(isInternetConnected));
      counterPing = 0;
    }
    if ((WiFi.status() != WL_CONNECTED || webSocketConncted == false || isInternetConnected == false ) && getChargePointStatusService_A()->getEmergencyRelayClose() == false) { //priority is on fault
      if (millis() - timercloudconnect > 10000) { //updates in 5sec
#if LED_ENABLED
        requestLed(BLINKYWHITE_ALL, START, 1);
#endif
        timercloudconnect = millis();
      }
    }
  } else if (gsm_connect == true && client.connected() ==  false && getChargePointStatusService_A()->getEmergencyRelayClose() == false) {
    if (millis() - timercloudconnect > 10000) { //updates in 5sec
#if LED_ENABLED
      requestLed(BLINKYWHITE_ALL, START, 1);
#endif
      timercloudconnect = millis();

    }
  }

}

void connectToWebsocket() {

  //url_m = String(ws_url_prefix_m);
  // String cpSerial = String("");
  // EVSE_A_getChargePointSerialNumber(cpSerial);
  // url_m += cpSerial; //most OCPP-Server require URLs like this. Since we're testing with an echo server here, this is obsolete

  //#if WIFI_ENABLED || ETHERNET_ENABLED
  Serial.print(F("Connecting to: "));
  Serial.println(path_m);
  // webSocket.begin(host_m, port_m, url_m, protocol_m);
  webSocket.begin(host_m, port_m, path_m, protocol);
  // event handler

  webSocket.onEvent(webSocketEvent);

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
  //#endif
}
