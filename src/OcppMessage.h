// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

/**
 * This framework considers OCPP operations to be a combination of two things: first, ensure that the message reaches its
 * destination properly (the "Remote procedure call" header, e.g. message Id). Second, transmit the application data
 * as specified in the OCPP 1.6 document.
 * 
 * The remote procedure call (RPC) part is implemented by the class OcppOperation. The application data part is implemented by
 * the respective OcppMessage subclasses, e.g. BootNotification, StartTransaction, ect.
 * 
 * The resulting structure is that the RPC header (=instance of OcppOperation) holds a reference to the payload
 * message creator (=instance of BootNotification, StartTransaction, ...). Both objects working together give the complete
 * OCPP operation.
 */

 #ifndef OCPPMESSAGE_H
 #define OCPPMESSAGE_H

 #include <ArduinoJson.h>
 #include "libraries/arduinoWebSockets-master/src/WebSocketsClient.h"


class OcppMessage {
private:

public:

  OcppMessage();

  virtual ~OcppMessage();
  
  virtual const char* getOcppOperationType();

  /**
   * Create the payload for the respective OCPP message
   * 
   * For instance operation Authorize: creates Authorize.req(idTag)
   * 
   * This function is usually called multiple times by the Arduino loop(). On first call, the request is initially sent. In the
   * succeeding calls, the implementers decide to either recreate the request, or do nothing as the operation is still pending.
   */
  virtual DynamicJsonDocument* createReq();


  virtual void processConf(JsonObject payload);

  /**
   * Processes the request in the JSON document. 
   */
  virtual void processReq(JsonObject payload);

  /**
   * After successfully processing a request sent by the communication counterpart, this function creates the payload for a confirmation
   * message.
   */
  virtual DynamicJsonDocument* createConf();
    
};

 #endif
