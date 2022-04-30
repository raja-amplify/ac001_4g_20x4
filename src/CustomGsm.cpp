#include "CustomGsm.h"
#include<Preferences.h>
extern String ws_url_prefix_m;
extern String host_m;
extern int port_m;
extern String path_m;
extern Preferences preferences;
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void SetupGsm(){

    if(DEBUG_OUT) Serial.println(F("[CustomSIM7672] Starting 4G Setup"));
    TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
    delay(200);
    if(DEBUG_OUT) Serial.println(F("[CustomSIM7672] Initializing modem..."));
    modem.restart();

    String modemInfo = modem.getModemInfo();
    if(DEBUG_OUT) Serial.print(F("[CustomSIM7672] Modem Info: "));
    Serial.println(modemInfo);

    #if TINY_GSM_USE_GPRS && defined TINY_GSM_MODEM_XBEE
    // The XBee must run the gprsConnect function BEFORE waiting for network!
    modem.gprsConnect(apn, gprsUser, gprsPass);
    #endif

    if(DEBUG_OUT) SerialMon.println(F("[CustomSIM7672] Waiting for network..."));
    if (!modem.waitForNetwork()) {
        if(DEBUG_OUT) SerialMon.println(F("[CustomSIM7672] fail"));
        delay(200);
        return;
  }
  if(DEBUG_OUT) SerialMon.println(F("[CustomSIM7672] success"));

  if (modem.isNetworkConnected()) {
    if(DEBUG_OUT) SerialMon.println(F("[CustomSIM7672] Network connected"));
  }

#if TINY_GSM_USE_GPRS
  // GPRS connection parameters are usually set after network registration
    if(DEBUG_OUT) SerialMon.print(F("[CustomSIM7672] Connecting to "));
    SerialMon.print(apn);
    if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
      if(DEBUG_OUT) SerialMon.println(F("[CustomSIM7672] fail"));
      delay(200);
      return;
    }
    if(DEBUG_OUT) SerialMon.println(F("[CustomSIM7672] success"));

  if (modem.isGprsConnected()) {
    if(DEBUG_OUT) SerialMon.println(F("[CustomGsm]GPRS connected"));
  }
 
   

#endif
   int csq = modem.getSignalQuality();
    Serial.println("Signal quality: "+String(csq));
    delay(1);


}


void ConnectToServer(){
    host_m.trim();
    const char* host = (host_m).c_str();
    Serial.println(host);

   if(DEBUG_OUT) SerialMon.println(F("[CustomSIM7672] Connecting to "));
    SerialMon.println(String(host_m) + "\n" + String(port_m));
    if (!client.connect(host, port_m)) {
       if(DEBUG_OUT) SerialMon.println(F("[CustomSIM7672] Connection request failed"));
        delay(1000);
        return;
    }
   if(DEBUG_OUT) SerialMon.println(F("[CustomSIM7672] Connected"));


  // // Make a HTTP GET request:
  //   Serial.println(ws_url_prefix_m);
  //   String url = String(ws_url_prefix_m);
  //   url = url.substring(6);
  //   int a = url.indexOf('/');
  //   url = url.substring(a);
  //   String cpSerial = String("");
  //   preferences.begin("credentials",false);
  //   EVSE_A_getChargePointSerialNumber(cpSerial);
  //   Serial.println(cpSerial);
  //   url += cpSerial;

  //   preferences.end();

    Serial.println("[Custom SIM7672: ]"+ String(path_m));
    SerialMon.println("Performing HTTP GET request...");
    client.print(String("GET ") + path_m + " HTTP/1.1\r\n");
    client.print(String("Host: ") + host_m + "\r\n");
    client.print("Upgrade: websocket\r\n");//delay(300);
    client.print("Connection: Upgrade\r\n");//delay(300);
  //  client.print("Origin: http://13.233.136.157:8080\r\n");//delay(300);
    client.print("Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n");//delay(300);
    client.print("Sec-WebSocket-Protocol: ocpp2.0, ocpp1.6\r\n");//delay(300);
   //  client.print("Connection: keep-alive\r\n");
    client.print("Sec-WebSocket-Version: 13\r\n\r\n");//delay(300);


    delay(100);

    //printSerialData();
    gsm_Loop();

}

void printSerialData(){
    while(client.available()!=0)
    Serial.println(client.readString());

}

int counter_clientConnection = 0;

void gsm_Loop(){

    if(!client.connected()){
        Serial.println(F("[CustomSIM7672] Client is disconnected. Trying to connect"));
       // cloudConnectivityLed_Loop();
        if(getChargePointStatusService_A() != NULL){
            if(getChargePointStatusService_A()->getEmergencyRelayClose() == false){
             requestLed(BLINKYWHITE_ALL, START,1);
            }
        }
       
        delay(100); //do not flood the network (have to create millis)
        SetupGsm();
        ConnectToServer();
    }else{
        Serial.println(F("[Custom GSM] Client Connected"));
    }

    gsmOnEvent();

}

void processJson(String &IncomingDataString){
    int index1= -1;
    int index2= -1;
    int len;
    uint8_t *payload;
    size_t DataLen;
    String DataString;

    int csq = modem.getSignalQuality();
    Serial.println("Signal quality: "+String(csq));

    len =IncomingDataString.length();
    if(DEBUG_EXTRA) Serial.println("\nLen-->"+String(len));
    IncomingDataString.trim();
    len =IncomingDataString.length();
    if(DEBUG_EXTRA) Serial.println("\nAfter trim Len-->"+String(len));

    index1 = IncomingDataString.indexOf('[');
    index2 = IncomingDataString.lastIndexOf(']');
    delay(250);
    if(DEBUG_EXTRA) Serial.println(IncomingDataString);
  //  delay(1000);
   // index3 = IncomingDataString.indexOf(']');
    if(DEBUG_EXTRA) Serial.println("");    
    if(DEBUG_EXTRA) Serial.println("Index 1--->"+String(index1));
    if(DEBUG_EXTRA) Serial.println("Index 2--->"+String(index2));
   // Serial.println("Index 2--->"+String(index3));
    if(index1 != -1 && index2 != -1){

        DataString = IncomingDataString.substring(index1,index2+1);
        DataLen = DataString.length();

        if(DEBUG_EXTRA) Serial.println(DataString +"\n Length:"+ String(DataLen));
        Serial.println("[CustomSIM7672] Wsc_Txt: "+DataString);
         
        payload = (uint8_t *)DataString.c_str();
    
        if (!processWebSocketEvent(payload, DataLen)) { //forward message to OcppEngine
            if(DEBUG_OUT) Serial.print(F("[CustomSIM7672] Processing WebSocket input event failed!\n"));
        }  
    
        IncomingDataString = ""; //Clearing the local buffer
    }
   // if(DEBUG_EXTRA) Serial.println("It's not Json packet");
}

void printingUint(String IncomingDataString){
int length = IncomingDataString.length();

uint8_t *payload = (uint8_t *)IncomingDataString.c_str();

    Serial.println("");
    Serial.print("Payload Size-->"+String(sizeof(payload)));
    for(int i=0;i<length;i++){
     Serial.print(" ");   
     Serial.print(payload[i],DEC);
    }
    Serial.println("");
 }

void gsmOnEvent(){
    bool flagWebSktHdrTxt = 0;
    bool flagCounter=0;
    bool flagJsonPkt = 1;
    char c;
    char header;
    int CounterOpenBracket = 0;
    int CounterCloseBracket = 0;
    String IncomingDataString ="";
    while(client.connected() && client.available()){
        c = client.read();
        if(c == 129 && flagJsonPkt == 1){ //Reading Headers
            flagJsonPkt = 0;
            flagWebSktHdrTxt =1;
        }

        if(flagWebSktHdrTxt == 1){
            flagWebSktHdrTxt = 0;
            header = client.read(); //reading payload length
            if(header >= 126){
                client.read(); //flushing Extra headers
                client.read();
            }
        }else{ //End of Headers
            IncomingDataString += c;
            if(c=='[') {CounterOpenBracket++; flagCounter=1;}
            if(c==']') CounterCloseBracket++;

            if(CounterOpenBracket == CounterCloseBracket && flagCounter == 1){

                if(DEBUG_EXTRA) Serial.print(c);
                if(DEBUG_EXTRA) printingUint(IncomingDataString);
                processJson(IncomingDataString);
                CounterOpenBracket=0;
                flagCounter=0;
                CounterCloseBracket=0;
                flagJsonPkt = 1;
                flagWebSktHdrTxt = 0;
            }else{

                if(1) Serial.print(c);
            }   




        }       

    }

}
/*void gsmOnEvent(){

   // String DataString;
    String IncomingDataString;
    int CounterOpenBracket=0;
    int CounterCloseBracket=0; 
    char c;  
    int flag=0; 
   while(client.connected() && client.available()) {
    delay(2);

  //  if(DEBUG_OUT) Serial.println("[CustomGsm]Data receieved in GSMBuffer: ");
    c = client.read();

    IncomingDataString += c;

    if(c=='[') {CounterOpenBracket++; flag=1;}
    if(c==']') CounterCloseBracket++;

    if(CounterOpenBracket == CounterCloseBracket && flag == 1){

        Serial.print(c);
        printinUint(IncomingDataString);
        processJson(IncomingDataString);
        CounterOpenBracket=0;
        flag=0;
        CounterCloseBracket=0;
    }else{

        Serial.print(c);
    }   

 //Commented this section
    IncomingDataString = client.readString();
    Serial.println(IncomingDataString);
    // int startByte = IncomingDataString.indexOf('[');
    int startByte = 2;
    int endByte = IncomingDataString.indexOf(']');
   if(DEBUG_OUT) Serial.println("[CustomGsm]Indexes are: ");
    Serial.println(startByte);
    Serial.println(endByte); 
    if(startByte != -1 && endByte !=-1){ 
      DataString = IncomingDataString.substring(startByte,endByte+1);
      // IncomingDataString.substring(2);
      DataLen = DataString.length();   
      Serial.println(DataString);
      Serial.println(DataLen);
      
      payload = (uint8_t *)DataString.c_str();
    
    if (!processWebSocketEvent(payload, DataLen)) { //forward message to OcppEngine
       if(DEBUG_OUT) Serial.print(F("[CustomSIM7672] Processing WebSocket input event failed!\n"));
      }   
   }
  
  }
}*/

/***********************************************************************************************/
 bool sendTXTGsmStr(String & payload) {
   
    return sendTXTGsm((uint8_t *)payload.c_str(), payload.length());
}

bool sendTXTGsm(uint8_t * payload, size_t length, bool headerToPayload) {

    if(length == 0) {
        length = strlen((const char *)payload);
    }
    if(true) {
        return sendFrame(WSop_text, payload, length, true, headerToPayload);
    }
    return false;
}

uint8_t createHeader(uint8_t * headerPtr, WSopcode_t opcode, size_t length, bool mask, uint8_t maskKey[4], bool fin) {
    uint8_t headerSize;
    // calculate header Size
    if(length < 126) {
        headerSize = 2;
    } else if(length < 0xFFFF) {
        headerSize = 4;
    } else {
        headerSize = 10;
    }

    if(mask) {
        headerSize += 4;
    }

    // create header

    // byte 0
    *headerPtr = 0x00;
    if(fin) {
        *headerPtr |= bit(7);    ///< set Fin
    }
    *headerPtr |= opcode;    ///< set opcode
    headerPtr++;

    // byte 1
    *headerPtr = 0x00;
    if(mask) {
        *headerPtr |= bit(7);    ///< set mask
    }

    if(length < 126) {
        *headerPtr |= length;
        headerPtr++;
    } else if(length < 0xFFFF) {
        *headerPtr |= 126;
        headerPtr++;
        *headerPtr = ((length >> 8) & 0xFF);
        headerPtr++;
        *headerPtr = (length & 0xFF);
        headerPtr++;
    } else {
        // Normally we never get here (to less memory)
        *headerPtr |= 127;
        headerPtr++;
        *headerPtr = 0x00;
        headerPtr++;
        *headerPtr = 0x00;
        headerPtr++;
        *headerPtr = 0x00;
        headerPtr++;
        *headerPtr = 0x00;
        headerPtr++;
        *headerPtr = ((length >> 24) & 0xFF);
        headerPtr++;
        *headerPtr = ((length >> 16) & 0xFF);
        headerPtr++;
        *headerPtr = ((length >> 8) & 0xFF);
        headerPtr++;
        *headerPtr = (length & 0xFF);
        headerPtr++;
    }

    if(mask) {
        *headerPtr = maskKey[0];
        headerPtr++;
        *headerPtr = maskKey[1];
        headerPtr++;
        *headerPtr = maskKey[2];
        headerPtr++;
        *headerPtr = maskKey[3];
        headerPtr++;
    }
    return headerSize;
}


bool sendFrame(WSopcode_t opcode, uint8_t * payload, size_t length, bool fin, bool headerToPayload) {
  
if(!client.connected()){

   if(DEBUG_OUT) Serial.println(F("[CustomSIM7672] Client is not connected"));
    return false;
}

  /*
    if(client->tcp && !client->tcp->connected()) {
        DEBUG_WEBSOCKETS("[WS][%d][sendFrame] not Connected!?\n", client->num);
        return false;
    }

    if(client->status != WSC_CONNECTED) {
        DEBUG_WEBSOCKETS("[WS][%d][sendFrame] not in WSC_CONNECTED state!?\n", client->num);
        return false;
    }
*/
 //   DEBUG_WEBSOCKETS("[WS][%d][sendFrame] ------- send message frame -------\n", client->num);
 //   DEBUG_WEBSOCKETS("[WS][%d][sendFrame] fin: %u opCode: %u mask: %u length: %u headerToPayload: %u\n", client->num, fin, opcode, true, length, headerToPayload);

 //   if(opcode == WSop_text) {
 //       DEBUG_WEBSOCKETS("[WS][%d][sendFrame] text: %s\n", client->num, (payload + (headerToPayload ? 14 : 0)));
 //   }

    uint8_t maskKey[4]                         = { 0x00, 0x00, 0x00, 0x00 };
    uint8_t buffer[WEBSOCKETS_MAX_HEADER_SIZE] = { 0 };

    uint8_t headerSize;
    uint8_t * headerPtr;
    uint8_t * payloadPtr = payload;
    bool useInternBuffer = false;
    bool ret             = true;

    // calculate header Size
    if(length < 126) {
        headerSize = 2;
    } else if(length < 0xFFFF) {
        headerSize = 4;
    } else {
        headerSize = 10;
    }

    if(true) {
        headerSize += 4;
    }

#ifdef WEBSOCKETS_USE_BIG_MEM

    // only for ESP since AVR has less HEAP
    // try to send data in one TCP package (only if some free Heap is there)
    if(!headerToPayload && ((length > 0) && (length < 1400)) && (GET_FREE_HEAP > 6000)) {
      //  DEBUG_WEBSOCKETS("[WS][%d][sendFrame] pack to one TCP package...\n", client->num);
        uint8_t * dataPtr = (uint8_t *)malloc(length + WEBSOCKETS_MAX_HEADER_SIZE);
        if(dataPtr) {
            memcpy((dataPtr + WEBSOCKETS_MAX_HEADER_SIZE), payload, length);
            headerToPayload = true;
            useInternBuffer = true;
            payloadPtr      = dataPtr;
        }
    }
#endif

    // set Header Pointer
    if(headerToPayload) {
        // calculate offset in payload
        headerPtr = (payloadPtr + (WEBSOCKETS_MAX_HEADER_SIZE - headerSize));
    } else {
        headerPtr = &buffer[0];
    }

    if(true && useInternBuffer) {
        // if we use a Intern Buffer we can modify the data
        // by this fact its possible the do the masking
        for(uint8_t x = 0; x < sizeof(maskKey); x++) {
            maskKey[x] = random(0xFF);
        }
    }

    createHeader(headerPtr, opcode, length, true, maskKey, fin);

    if(true && useInternBuffer) {
        uint8_t * dataMaskPtr;

        if(headerToPayload) {
            dataMaskPtr = (payloadPtr + WEBSOCKETS_MAX_HEADER_SIZE);
        } else {
            dataMaskPtr = payloadPtr;
        }

        for(size_t x = 0; x < length; x++) {
            dataMaskPtr[x] = (dataMaskPtr[x] ^ maskKey[x % 4]);
        }
    }

#ifndef NODEBUG_WEBSOCKETS
    unsigned long start = micros();
#endif

    if(headerToPayload) {
        // header has be added to payload
        // payload is forced to reserved 14 Byte but we may not need all based on the length and mask settings
        // offset in payload is calculatetd 14 - headerSize
        if(client.write(&payloadPtr[(WEBSOCKETS_MAX_HEADER_SIZE - headerSize)], (length + headerSize)) != (length + headerSize)) {
            ret = false;
        }
    } else {
        // send header
        if(client.write(&buffer[0], headerSize) != headerSize) {
            ret = false;
        }

        if(payloadPtr && length > 0) {
            // send payload
            if(client.write(&payloadPtr[0], length) != length) {
                ret = false;
            }
        }
    }

  //  DEBUG_WEBSOCKETS("[WS][%d][sendFrame] sending Frame Done (%luus).\n", client->num, (micros() - start));
//mySerial.write("\r\n");
//mySerial.write(0x1a);
//mySerial.flush();
delay(100);
//printSerialData();
#ifdef WEBSOCKETS_USE_BIG_MEM
    if(useInternBuffer && payloadPtr) {
        free(payloadPtr);
    }
#endif
  if(DEBUG_OUT)  Serial.println("[CustomGsm-frame] Return : "+String(ret));
    return ret;
}
