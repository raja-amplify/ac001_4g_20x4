#include "EVSE_B.h"
#include "Master.h"
#include "ControlPilot.h"
#include "LCD_I2C.h"

#if DWIN_ENABLED
#include "dwin.h"
extern SoftwareSerial dwin;
extern unsigned char v1[8];
extern unsigned char v2[8];
extern unsigned char v3[8];
extern unsigned char i1[8];
extern unsigned char i2[8];
extern unsigned char i3[8];
extern unsigned char e1[8];
extern unsigned char e2[8];
extern unsigned char e3[8];
extern unsigned char change_page[10];
extern unsigned char avail[22];
extern unsigned char charging[28];
extern unsigned char cid2[7];
#endif

uint8_t currentCounterThreshold_B = 30;

bool disp_evse_B = false;

OnBoot_B onBoot_B;
OnReadUserId_B onReadUserId_B;
OnSendHeartbeat_B onSendHeartbeat_B;
OnAuthentication_B onAuthentication_B;
OnStartTransaction_B onStartTransaction_B;
OnStopTransaction_B onStopTransaction_B;
OnUnauthorizeUser_B onUnauthorizeUser_B;

bool notFaulty_B = false;

bool evIsPlugged_B; 
bool flag_evseIsBooted_B;
bool flag_evseReadIdTag_B;
bool flag_evseAuthenticate_B;
bool flag_evseStartTransaction_B;
bool flag_evRequestsCharge_B;
bool flag_evseStopTransaction_B;
bool flag_evseUnauthorise_B;
bool flag_rebootRequired_B;
bool flag_evseSoftReset_B; 

extern bool flag_rebootRequired_C;
extern bool flag_rebootRequired_A;

extern bool flag_ed_A;

//Reason for stop
extern uint8_t reasonForStop;

extern int8_t fault_code_B;

float chargingLimit_B = 32.0f;
String Ext_currentIdTag_B = "";

long int blinckCounter_B =0;
int counter1_B =0;

ulong t_B;
int connectorDis_counter_B=0;
short int counter_drawingCurrent_B = 0;
float drawing_current_B = 0;


String currentIdTag_B = "";
extern WebSocketsClient webSocket;
extern EVSE_states_enum EVSE_state;
extern Preferences preferences;
extern MFRC522 mfrc522;
extern String currentIdTag;
extern bool webSocketConncted;
extern bool isInternetConnected;
extern bool wifi_connect;
extern bool gsm_connect;

Preferences resumeTxn_B;
extern TinyGsmClient client;

bool ongoingTxn_B;
String idTagData_B = "";

bool timeout_active_B = false;
bool timer_initialize_B = false;
ulong timeout_start_B = 0;
short int fault_counter_B = 0;
bool flag_faultOccured_B = false;
ulong relay_timer_B = 0;
ulong faultTimer_B =0;

ulong timerDisplayB;

extern LCD_I2C lcd;


/**********************************************************/
void EVSE_B_StopSession(){

	if(getChargePointStatusService_B()->getEvDrawsEnergy() == true){
		getChargePointStatusService_B()->stopEvDrawsEnergy();
	}
	
	//digitalWrite(32, LOW);
	disp_evse_B = false;
    requestForRelay(STOP,2); 
    delay(500);           	
    flag_evseReadIdTag_B = false;
	flag_evseAuthenticate_B = false;
	flag_evseStartTransaction_B = false;
	flag_evRequestsCharge_B = false;
	flag_evseStopTransaction_B = true;
	flag_evseUnauthorise_B = false;
    Serial.println("[EVSE] Stopping Session : " +String(EVSE_state));
}
/**************************************************************************/

/**************************************************************************/

void EVSE_B_initialize() {
	if(DEBUG_OUT) Serial.print(F("[EVSE_B] EVSE is powered on or reset. Starting Boot.\n"));
	//onBoot_B();
	flag_evseIsBooted_B = true; //Exit condition for booting. 	
	flag_evseReadIdTag_B = true; //Entry condition for reading ID Tag.
	faultTimer_B = millis();
}

/**************************SetUp********************************************/
void EVSE_B_setup(){

	resumeTxn_B.begin("resume_B",false); //opening preferences in R/W mode
  	idTagData_B = resumeTxn_B.getString("idTagData_B","");
  	ongoingTxn_B = resumeTxn_B.getBool("ongoingTxn_B",false);

  	Serial.println("Stored ID:"+String(idTagData_B));
  	Serial.println("Ongoing Txn: "+String(ongoingTxn_B));

	EVSE_B_setOnBoot([]() {
		//this is not in loop, that is why we need not update the flag immediately to avoid multiple copies of bootNotification.
		OcppOperation *bootNotification = makeOcppOperation(&webSocket,	new BootNotification());
		initiateOcppOperation(bootNotification);
		bootNotification->setOnReceiveConfListener([](JsonObject payload) {

      	// if( flag_MeteringIsInitialised == false){
      	// 	Serial.println("[SetOnBooT] Initializing metering services");
      	// //	meteringService->init(meteringService);
      	// }

      if (DEBUG_OUT) Serial.print(F("EVSE_setOnBoot Callback: Metering Services Initialization finished.\n"));

			flag_evseIsBooted_B = true; //Exit condition for booting. 	
			flag_evseReadIdTag_B = true; //Entry condition for reading ID Tag.
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
      if (DEBUG_OUT) Serial.print(F("EVSE_setOnBoot Callback: Closing Relays.\n"));

			if (DEBUG_OUT) Serial.print(F("EVSE_setOnBoot Callback: Boot successful. Calling Read User ID Block.\n"));
		});
	});

	EVSE_B_setOnReadUserId([] () {
		if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnReadUserId Callback: EVSE_B waiting for User ID read...\n"));
		static ulong timerForRfid = millis();
		currentIdTag_B = "";  
		idTagData_B = resumeTxn_B.getString("idTagData_B","");
  		ongoingTxn_B = resumeTxn_B.getBool("ongoingTxn_B",false);

  		if((wifi_connect == true)&& 														//Wifi Block
  			(getChargePointStatusService_B()->getEmergencyRelayClose() == false)&&
  			(webSocketConncted == true)&&
  			(WiFi.status() == WL_CONNECTED)&&
  			(isInternetConnected == true)){

  			if((idTagData_B != "") && (ongoingTxn_B == 1)){
  				currentIdTag_B = resumeTxn_B.getString("idTagData_B", "");
  				Serial.println(F("[EVSE_B_setOnReadUserId] Resuming Session"));
  				getChargePointStatusService_B()->authorize(currentIdTag_B); // so that Starttxn can easily fetch this data
  				requestLed(BLUE, START, 2);
  			}else{
				if(millis() - timerForRfid > 10000){ //timer for sending led request
	    		  	requestLed(GREEN,START,2);
	    		  	timerForRfid = millis();
    		  	}
			
				currentIdTag_B = EVSE_B_getCurrnetIdTag(&mfrc522);
				Serial.println(F("[Wifi]********RFID B**********"));
  			}

  		}else if((gsm_connect == true)&&													//GSM Block
  				(getChargePointStatusService_B()->getEmergencyRelayClose() == false)&&
  				(client.connected() == true)){

  			if((idTagData_B != "") && (ongoingTxn_B == 1)){
  				currentIdTag_B = resumeTxn_B.getString("idTagData_B", "");
  				getChargePointStatusService_B()->authorize(currentIdTag_B); // so that Starttxn can easily fetch this data
  				Serial.println(F("[EVSE_B_setOnReadUserId] Resuming Session"));
  				requestLed(BLUE, START, 2);
  			}else{
				if(millis() - timerForRfid > 10000){ //timer for sending led request
	    		  	requestLed(GREEN,START,2);
	    		  	timerForRfid = millis();
    		  	}
			
				currentIdTag_B = EVSE_B_getCurrnetIdTag(&mfrc522);
				Serial.println(F("[GSM]********RFID B**********"));
  			}


  		}


		/*	
		idTagData_m = resumeTxn.getString("idTagData","");
  		ongoingTxn_m = resumeTxn.getBool("ongoingTxn",false);

		if((ongoingTxn_m == 1) && (idTagData_m != "") && 
		      (getChargePointStatusService_B()->getEmergencyRelayClose() == false) &&
		      (WiFi.status() == WL_CONNECTED)&&
		      (webSocketConncted == true)&&
		      (isInternetConnected == true)){   //giving priority to stored data
			currentIdTag_B = resumeTxn.getString("idTagData","");
			Serial.println("[EVSE_setOnReadUserId] Resuming Session");
      		requestLed(BLUE,START,1);
         
		}else*/ 
		// if((getChargePointStatusService_B()->getEmergencyRelayClose() == false) &&
		//           (WiFi.status() == WL_CONNECTED) &&
		//           (webSocketConncted == true) && 
		//           (isInternetConnected == true)){
		// 	  #if LED_ENABLED
		// 	  if(millis() - timerForRfid > 5000){ //timer for sending led request
  //   		  requestLed(GREEN,START,2);
  //   		  timerForRfid = millis();
  //   		  }
  //   		  #endif
		// 	currentIdTag_B = EVSE_B_getCurrnetIdTag(&mfrc522);
		// 	Serial.println("********RFID B**********");
		// }

		if (currentIdTag_B.equals("") == true) {
			flag_evseReadIdTag_B = true; //Looping back read block as no ID tag present.
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
		} else {
			flag_evseReadIdTag_B = false;
			flag_evseAuthenticate_B = true; //Entry condition for authentication block.
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
			if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnReadUserId Callback: Successful User ID Read. Calling Authentication Block.\n"));
		}	
	});
	
	EVSE_B_setOnsendHeartbeat([] () {
    if (DEBUG_OUT) Serial.print(F("EVSE_setOnsendHeartbeat Callback: Sending heartbeat signal...\n"));
    OcppOperation *heartbeat = makeOcppOperation(&webSocket, new Heartbeat());
    initiateOcppOperation(heartbeat); 
    heartbeat->setOnReceiveConfListener([](JsonObject payload) {
        const char* currentTime = payload["currentTime"] | "Invalid";
        if (strcmp(currentTime, "Invalid")) {
          if (setTimeFromJsonDateString(currentTime)) {
            if (DEBUG_OUT) Serial.print(F("EVSE_setOnsendHeartbeat Callback: Request has been accepted!\n"));
          } else {
            Serial.print(F("EVSE_setOnsendHeartbeat Callback: Request accepted. But Error reading time string. Expect format like 2020-02-01T20:53:32.486Z\n"));
          }
        } else {
          Serial.print(F("EVSE_setOnsendHeartbeat Callback: Request denied. Missing field currentTime. Expect format like 2020-02-01T20:53:32.486Z\n"));
        }
    });   
  });
 
	EVSE_B_setOnAuthentication([] () {
		if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnAuthentication Callback: Authenticating...\n"));
		flag_evseAuthenticate_B = false;
		OcppOperation *authorize = makeOcppOperation(&webSocket, new Authorize(currentIdTag_B));
		initiateOcppOperation(authorize);
		/*chargePointStatusService->authorize(currentIdTag_B, connectorId_B);  */    //have to edit
		authorize->setOnReceiveConfListener([](JsonObject payload) {
			const char* status = payload["idTagInfo"]["status"] | "Invalid";
			if (!strcmp(status, "Accepted")) {
				flag_evseReadIdTag_B = false;
				flag_evseAuthenticate_B = false;
				flag_evseStartTransaction_B = true; //Entry condition for starting transaction.
				flag_evRequestsCharge_B = false;
				flag_evseStopTransaction_B = false;
				flag_evseUnauthorise_B = false;
				requestLed(BLUE,START,2);
				if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnAuthentication Callback: Authorize request has been accepted! Calling StartTransaction Block.\n"));
				#if CP_ACTIVE 
				flag_controlPAuthorise = true;
				#endif

			} else {
				flag_evseReadIdTag_B = false;
				flag_evseAuthenticate_B = false;
				flag_evseStartTransaction_B = false;
				flag_evRequestsCharge_B = false;
				flag_evseStopTransaction_B = false;
				flag_evseUnauthorise_B = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
				if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnAuthentication Callback: Authorize request has been denied! Read new User ID. \n"));
			}  
		});
	});
	
	EVSE_B_setOnStartTransaction([] () {
		flag_evseStartTransaction_B = false;
		String idTag = "";
		int connectorId = 0;

		if (getChargePointStatusService_B() != NULL) {
			if (!getChargePointStatusService_B()->getIdTag().isEmpty()) {
				idTag = String(getChargePointStatusService_B()->getIdTag());
				connectorId = getChargePointStatusService_B()->getConnectorId();
			}
		}
		OcppOperation *startTransaction = makeOcppOperation(&webSocket, new StartTransaction(idTag,connectorId));
		initiateOcppOperation(startTransaction);
		startTransaction->setOnReceiveConfListener([](JsonObject payload) {
    		const char* status = payload["idTagInfo"]["status"] | "Invalid";
      if (!strcmp(status, "Accepted") || !strcmp(status, "ConcurrentTx")) { 

      flag_evseReadIdTag_B = false;
      flag_evseAuthenticate_B = false;
      flag_evseStartTransaction_B = false;
      flag_evRequestsCharge_B = true;
      flag_evseStopTransaction_B = false;
      flag_evseUnauthorise_B = false;
      if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnStartTransaction Callback: StartTransaction was successful\n"));
	 #if LCD_ENABLED
      lcd.clear();
      lcd.setCursor(3, 2);
      lcd.print("SESSION B STARTED");
     #endif
      /************************************/
      int transactionId = payload["transactionId"] | -1;
      getChargePointStatusService_B()->startTransaction(transactionId);        //new block for Three connector
      getChargePointStatusService_B()->startEnergyOffer();
      resumeTxn_B.putString("idTagData_B",getChargePointStatusService_B()->getIdTag());
      resumeTxn_B.putBool("ongoingTxn_B",true);

      //*****Storing tag data in EEPROM****//
      /*
	  resumeTxn.putString("idTagData",currentIdTag);
      resumeTxn.putBool("ongoingTxn",true);*/
      //***********************************//

      } else {
        flag_evseReadIdTag_B = false;
        flag_evseAuthenticate_B = false;
        flag_evseStartTransaction_B = false;
        flag_evRequestsCharge_B = false;
        flag_evseStopTransaction_B = false;
        flag_evseUnauthorise_B = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.

        if (DEBUG_OUT) Serial.print(F("************************************************************************\n"));
        if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnStartTransaction Callback: StartTransaction was unsuccessful\n"));
        if (DEBUG_OUT) Serial.print(F("************************************************************************\n"));
#if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 2);
  lcd.print("SESSION B NOT STARTED");
  lcd.setCursor(0, 3);
  lcd.print("CONCURRENT");
	#endif
    	#if CP_ACTIVE
		requestforCP_OUT(STOP);  //stop pwm
		#endif
		/*resume namespace
		resumeTxn.putBool("ongoingTxn",false);
		resumeTxn.putString("idTagData","");
		*/

      }
		});
	});
			
	EVSE_B_setOnStopTransaction([] () {
		flag_evseStopTransaction_B = false;
		if (getChargePointStatusService_B() != NULL) {
          getChargePointStatusService_B()->stopEnergyOffer();
        }
        int txnId = getChargePointStatusService_B()->getTransactionId();
        int connector = getChargePointStatusService_B()->getConnectorId();
		OcppOperation *stopTransaction = makeOcppOperation(&webSocket, new StopTransaction(currentIdTag_B, txnId, connector));
		initiateOcppOperation(stopTransaction);
    	if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnStopTransaction  before Callback: Closing Relays.\n"));
		#if LCD_ENABLED
  			lcd.clear();
  			lcd.setCursor(3, 2);
  			lcd.print("STOP SESSION B");
			#endif

    	/**********************Until Offline functionality is implemented***********/
    	//Resume namespace(Preferences)
    	if(getChargePointStatusService_B()->getEmergencyRelayClose() == false){
    		requestLed(GREEN,START,1);   //temp fix
    	}
    	resumeTxn_B.putBool("ongoingTxn_B",false);
    	resumeTxn_B.putString("idTagData_B","");
    	if(wifi_connect == true){
	    	if(!webSocketConncted || WiFi.status() != WL_CONNECTED || isInternetConnected == false ){
		        getChargePointStatusService_B()->stopTransaction();
		    	getChargePointStatusService_B()->unauthorize();  //can be buggy 
		        flag_evseReadIdTag_B = true;
		        flag_evseAuthenticate_B = false;
		        flag_evseStartTransaction_B = false;
		        flag_evRequestsCharge_B = false;
		        flag_evseStopTransaction_B = false;
		        flag_evseUnauthorise_B = false;
    			Serial.println(F("Clearing Stored ID tag in StopTransaction()"));
    		}
    	}else if(gsm_connect == true){
    		if(client.connected() == false){
    			getChargePointStatusService_B()->stopTransaction();
		    	getChargePointStatusService_B()->unauthorize();  //can be buggy 
		        flag_evseReadIdTag_B = true;
		        flag_evseAuthenticate_B = false;
		        flag_evseStartTransaction_B = false;
		        flag_evRequestsCharge_B = false;
		        flag_evseStopTransaction_B = false;
		        flag_evseUnauthorise_B = false;
    			Serial.println(F("Clearing Stored ID tag in StopTransaction()"));
    		}
    	}

    	requestForRelay(STOP,2);
    
    	delay(500);
		stopTransaction->setOnReceiveConfListener([](JsonObject payload) {
			#if LCD_ENABLED
  			lcd.clear();
  			lcd.setCursor(3, 2);
  			lcd.print("STOP B SUCCESS");
			#endif
			flag_evseReadIdTag_B = false;
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = true;
			getChargePointStatusService_B()->stopTransaction();
      		if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnStopTransaction Callback: Closing Relays.\n"));
			if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnStopTransaction Callback: StopTransaction was successful\n"));
			if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnStopTransaction Callback: Reinitializing for new transaction. \n"));
		});
	});
	
	EVSE_B_setOnUnauthorizeUser([] () {
		if(flag_evseSoftReset_B == true){
			//This 'if' block is developed by @Wamique.
			flag_evseReadIdTag_B = false;
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
			flag_rebootRequired_B = true;
			getChargePointStatusService_B()->unauthorize();
			if (DEBUG_OUT) Serial.println(F("EVSE_B_setOnUnauthorizeUser Callback: Initiating Soft reset"));
		} else if(flag_evseSoftReset_B == false){
			flag_evseReadIdTag_B = true;
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = false;
			flag_evseUnauthorise_B = false;
			if (DEBUG_OUT) Serial.print(F("EVSE_B_setOnUnauthorizeUser Callback: Unauthorizing user and setting up for new user ID read.\n"));
			getChargePointStatusService_B()->unauthorize();
			
		}
	});
}

/*********************************************************************/

void EVSE_B_loop() {
	
	if (flag_evseIsBooted_B == false){
		if(DEBUG_OUT) Serial.println(F("[EVSE] Booting..."));
		delay(1000);
		//onBoot();
		t_B = millis();
		return;
	} else if (flag_evseIsBooted_B == true) {
		if (flag_evseReadIdTag_B == true) {
			if (onReadUserId_B != NULL) {
				onReadUserId_B();
			}
			return;
		} else if (flag_evseAuthenticate_B == true) {
			if (onAuthentication_B != NULL) {
				onAuthentication_B();
				
			}
			return;
		} else if (flag_evseStartTransaction_B == true) {
			if (onStartTransaction_B != NULL) {
				#if CP_ACTIVE
				if((EVSE_state == STATE_C || EVSE_state == STATE_D) && getChargePointStatusService()->getEmergencyRelayClose() == false){
					onStartTransaction_B();
				}else{
					Serial.println(F("Connect the Connector to EV / Or fault exist"));     //here have to add timeout of 30 sec
					connectorDis_counter_B++;
					//EVSE_stopTransactionByRfid();
					if(connectorDis_counter_B > 25){
						connectorDis_counter_B = 0;

						EVSE_StopSession_B();
					}
					
				}
				#endif

				#if !CP_ACTIVE
					onStartTransaction_B();   //can add check for fault
				#endif

			}
		} else if (flag_evRequestsCharge_B == true){

		#if CP_ACTIVE
			//flag_evRequestsCharge = false;
				if (getChargePointStatusService_B() != NULL && getChargePointStatusService_B()->getEvDrawsEnergy() == false) {
				
			/***********************Control Pilot @Wamique******************/
					if(EVSE_state == STATE_C || EVSE_state == STATE_D){
						if (getChargePointStatusService_B()->getEmergencyRelayClose() == false) {
							EVSE_StartCharging_B();
						} else if (getChargePointStatusService_B()->getEmergencyRelayClose() == true) {
							Serial.println(F("The voltage / current / Temp is out or range. FAULTY CONDITION DETECTED."));
						}
					}else if(EVSE_state == STATE_SUS){
						EVSE_Suspended_B();
						Serial.println(counter1);
						if(counter1_B++ > 25){    //Have to implement proper timeout
							counter1_B = 0;
							EVSE_B_StopSession();
						}

					}else if(EVSE_state == STATE_DIS || EVSE_state == STATE_E || EVSE_state == STATE_B || EVSE_state == STATE_A){
				
					//	EVSE_StopSession();     // for the very first time cable can be in disconnected state

						//if(txn == true){           // can implement counter > 10 just to remove noise
						EVSE_B_StopSession();
				//	}

					}else{

						Serial.println("[EVSE] STATE Error" +String(EVSE_state));
						delay(2000);

					//	requestLed(RED,START,1);
					}
				} 
				if(getChargePointStatusService_B()->getEvDrawsEnergy() == true){

				//	txn = true;
				
					if(EVSE_state == STATE_C || EVSE_state == STATE_D ){

						if(DEBUG_OUT) Serial.println(F("[EVSE_CP] Drawing Energy"));	

						if(millis() - t_B > 10000){
				 			if(getChargePointStatusService_B()->getEmergencyRelayClose() == false){
				 				requestLed(BLINKYGREEN,START,2);
				 				t_B = millis();
				 			}
				 		}
						/*
						if(blinckCounter++ % 2 == 0){
							requestLed(GREEN,START,1);
						}else{
							requestLed(GREEN,STOP,1);
						}*/
					}else if(EVSE_state == STATE_A || EVSE_state == STATE_E || EVSE_state == STATE_B){//Although CP Inp will never go to A,B state
						if(counter_faultstate_B++ > 5){
						 EVSE_B_StopSession();
						 counter_faultstate_B = 0;
						}
                
					}else if(EVSE_state == STATE_SUS){
						EVSE_B_Suspended();    //pause transaction :update suspended state is considered in charging state

					}else if(EVSE_state == STATE_DIS){

						Serial.println(F("[EVSE] Connect the Connector with EV and Try again"));
						EVSE_B_StopSession();						
				}                      
			}

			 /***Implemented Exit Feature with RFID @Wamique****/ 
		//	  EVSE_B_stopTransactionByRfid();
		#endif

			
			#if !CP_ACTIVE
			if (getChargePointStatusService_B() != NULL && getChargePointStatusService_B()->getEvDrawsEnergy() == false) {
				if (getChargePointStatusService_B()->getEmergencyRelayClose() == false) {
					getChargePointStatusService_B()->startEvDrawsEnergy();
					
					if (DEBUG_OUT) Serial.print(F("[EVSE_B] Opening Relays.\n"));
					requestForRelay(START,2);
					requestLed(ORANGE,START,2);
    				delay(1200);
    				requestLed(WHITE,START,2);
    				delay(1200);
    				requestLed(GREEN,START,2);
   				 	delay(1000);
					if(DEBUG_OUT) Serial.println(F("[EVSE_B] Started Drawing Energy"));
					
				} else if (getChargePointStatusService_B()->getEmergencyRelayClose() == true) {
						Serial.println(F("The voltage or current is out or range. FAULTY CONDITION DETECTED."));
					}
			} 
			if(getChargePointStatusService_B()->getEvDrawsEnergy() == true){
				//delay(250);
            
				 if(DEBUG_OUT) Serial.println(F("[EVSE_B] Drawing Energy"));
				 disp_evse_B = true;
				//displayMeterValuesB();
				 //blinking green Led
				 if(millis() - t_B > 5000){
				 	// if((WiFi.status() == WL_CONNECTED) && (webSocketConncted == true) && (isInternetConnected == true)&& getChargePointStatusService()->getEmergencyRelayClose() == false){
				 	// 	requestLed(BLINKYGREEN_EINS,START,1);
				 	// 	t = millis();
				 	// }

					if(getChargePointStatusService_B()->getEmergencyRelayClose() == false){
					 		requestLed(BLINKYGREEN,START,2);
					 		t_B = millis();

					 		if(millis() - relay_timer_B > 15000){
						 	
								requestForRelay(START,2);
								relay_timer_B = millis();

							}
					}

				}
				 //Current check
				 drawing_current_B = eic.GetLineCurrentB();
				 Serial.println("Current B: "+String(drawing_current_B));
				 if(drawing_current_B <= 0.15){
				 	counter_drawingCurrent_B++;
				 	//if(counter_drawingCurrent_B > 120){
					if(counter_drawingCurrent_B > currentCounterThreshold_B)
					{
				 		counter_drawingCurrent_B = 0;
						 //Check for the case where reasonForStop is not Local , Other
						if(reasonForStop!= 3 || reasonForStop!= 4)
						 reasonForStop = 1; // EV disconnected
						 #if LCD_ENABLED
  						lcd.clear();
  						lcd.setCursor(3, 0);
  						lcd.print("No Power Drawn /"); 
						lcd.setCursor(3, 1);
						lcd.print("EV disconnected"); 
						#endif
				 		Serial.println(F("Stopping session due to No current"));
				 		EVSE_B_StopSession();
				 	}

				 }else{
				 	counter_drawingCurrent_B = 0;
				    currentCounterThreshold_B = 2;
				 	Serial.println(F("counter_drawing Current Reset"));

				 }
				
			}
			   //Implemented Exit Feature with RFID @Wamique//
		//	 EVSE_B_stopTransactionByRfid();
			#endif
			//this is the only 'else if' block which is calling next else if block. the control is from this file itself. the control is not changed from any other file. but the variables are required to be present as extern in other file to decide calling of other functions. 
			return;
		} else if (flag_evseStopTransaction_B == true) {
			if (getChargePointStatusService_B() != NULL) {
				getChargePointStatusService_B()->stopEvDrawsEnergy();

			}
			if (onStopTransaction_B != NULL) {
				onStopTransaction_B();
				#if CP_ACTIVE
				requestforCP_OUT(STOP);  //stop pwm
				#endif
			}
			return;
		} else if (flag_evseUnauthorise_B == true) {
			if (onUnauthorizeUser_B != NULL) {
				onUnauthorizeUser_B();
			}
			return;
		} else if (flag_rebootRequired_B == true && flag_rebootRequired_A == true && flag_rebootRequired_C == true) {
			//soft reset execution.
			// flag_evseIsBooted_B = false;
			// flag_rebootRequired_B = false;
			// flag_evseSoftReset_B = false;
			if(getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Charging &&
				getChargePointStatusService_B()->inferenceStatus() != ChargePointStatus::Charging &&
				getChargePointStatusService_C()->inferenceStatus() != ChargePointStatus::Charging){
					if(DEBUG_OUT) Serial.print(F("[EVSE] rebooting in 5 seconds...\n"));
					delay(5000);
					ESP.restart();
			}
		} else {
			if(DEBUG_OUT) Serial.print(F("[EVSE_B] waiting for response...\n"));
			delay(100);
		}	
	}
}

short EMGCY_counter_B =0;
bool EMGCY_FaultOccured_B = false;

void emergencyRelayClose_Loop_B(){
	if(millis() - faultTimer_B > 1000){
		faultTimer_B = millis();
		bool EMGCY_status_B = requestEmgyStatus();
		//Serial.println("EMGCY_Status_B: "+String(EMGCY_status_B));
		if(EMGCY_status_B == true){
			if(EMGCY_counter_B++ > 0){
			//if(EMGCY_counter_B == 0){
				requestForRelay(STOP,2);
				requestLed(BLINKYRED,START,2);
				#if LCD_ENABLED
				lcd.clear();
				//lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
    			//lcd.print("                    ");//Clear the line
				lcd.setCursor(4, 0); // Or setting the cursor in the desired position.
				lcd.print("FAULTED: EMGY");
				#endif
				getChargePointStatusService_B()->setEmergencyRelayClose(true);
				EMGCY_FaultOccured_B = true;
				EMGCY_counter_B = 0;
			}
		}else{
			EMGCY_FaultOccured_B = false;
			EMGCY_counter_B = 0;
			getChargePointStatusService_B()->setEmergencyRelayClose(false);
		}

		if(EMGCY_FaultOccured_B == true && getChargePointStatusService_B()->getTransactionId() != -1){

			flag_evseReadIdTag_B = false;
			flag_evseAuthenticate_B = false;
			flag_evseStartTransaction_B = false;
			flag_evRequestsCharge_B = false;
			flag_evseStopTransaction_B = true;
		
		}else if(EMGCY_FaultOccured_B == false){


			float volt = eic.GetLineVoltageB();
			float current = eic.GetLineCurrentB();
			float temp= eic.GetTemperature();
			Serial.println("Voltage_B: "+String(volt)+", Current_B: "+String(current)+", Temperature: "+String(temp));
			if (getChargePointStatusService_B() != NULL) {
				if(getChargePointStatusService_B()->getOverVoltage() == true ||
					getChargePointStatusService_B()->getUnderVoltage() == true ||
					getChargePointStatusService_B()->getUnderTemperature() == true ||
					getChargePointStatusService_B()->getOverTemperature() == true ||
					(flag_ed_A) ||
					getChargePointStatusService_B()->getOverCurrent() == true){
						Serial.println("[EVSE_B] Fault Occurred.");
						notFaulty_B = false;						
						getChargePointStatusService_B()->setEmergencyRelayClose(true);
						if (!timer_initialize_B){
							timeout_start_B = millis();
							timer_initialize_B = true;
						}
					} else if(getChargePointStatusService_B()->getOverVoltage() == false &&
							getChargePointStatusService_B()->getUnderVoltage() == false &&
							getChargePointStatusService_B()->getUnderTemperature() == false &&
							getChargePointStatusService_B()->getOverTemperature() == false &&
							(!flag_ed_A) &&
							getChargePointStatusService_B()->getOverCurrent() == false){
						Serial.println("[EVSE_B]Not Faulty.");	
						notFaulty_B = true;					
						getChargePointStatusService_B()->setEmergencyRelayClose(false);
						//if (!timer_initialize){
							timeout_start_B = 0;
							timer_initialize_B = false;
						//}
					}
					
				if (getChargePointStatusService_B()->getEmergencyRelayClose() == true){
					timeout_active_B = true;
					requestForRelay(STOP,2);
					delay(50);
					#if LED_ENABLED
					requestLed(RED,START,2);
					#endif

					flag_faultOccured_B = true;
				} else if (getChargePointStatusService_B()->getEmergencyRelayClose() == false && flag_faultOccured_B == true){
					timeout_active_B = false;
					if ( (getChargePointStatusService_B()->getTransactionId() != -1)){ //can be buggy
						if(fault_counter_B++ > 1){
							fault_counter_B = 0;
							//requestForRelay(START,2);
							//delay(50);
							Serial.println("[EmergencyRelay_B] Starting Txn");
							flag_faultOccured_B = false;
						}
					}
				}

				if (timeout_active_B && getChargePointStatusService_B()->getTransactionId() != -1) {
					if (millis() - timeout_start_B >= TIMEOUT_EMERGENCY_RELAY_CLOSE){
						Serial.println("[EVSE_B] Emergency Stop.");
						flag_evRequestsCharge_B = false;
						flag_evseStopTransaction_B = true;
						timeout_active_B = false;
						timer_initialize_B = false;
					}
				}
			}
		}
	}
}

/*
* @param limit: expects current in amps from 0.0 to 32.0
*/
void EVSE_B_setChargingLimit(float limit) {
	if(DEBUG_OUT) Serial.print(F("[EVSE] New charging limit set. Got "));
	if(DEBUG_OUT) Serial.print(limit);
	if(DEBUG_OUT) Serial.print(F("\n"));
	chargingLimit_B = limit;
}

bool EVSE_B_EvRequestsCharge() {
	return flag_evRequestsCharge_B;
}

bool EVSE_B_EvIsPlugged() {
	return evIsPlugged_B;
}


void EVSE_B_setOnBoot(OnBoot_B onBt_B){
	onBoot_B = onBt_B;
}

void EVSE_B_setOnReadUserId(OnReadUserId_B onReadUsrId_B){
	onReadUserId_B = onReadUsrId_B;
}

void EVSE_B_setOnsendHeartbeat(OnSendHeartbeat_B onSendHeartbt_B){
	onSendHeartbeat_B = onSendHeartbt_B;
}

void EVSE_B_setOnAuthentication(OnAuthentication_B onAuthenticatn_B){
	onAuthentication_B = onAuthenticatn_B;
}

void EVSE_B_setOnStartTransaction(OnStartTransaction_B onStartTransactn_B){
	onStartTransaction_B = onStartTransactn_B;
}

void EVSE_B_setOnStopTransaction(OnStopTransaction_B onStopTransactn_B){
	onStopTransaction_B = onStopTransactn_B;
}

void EVSE_B_setOnUnauthorizeUser(OnUnauthorizeUser_B onUnauthorizeUsr_B){
	onUnauthorizeUser_B = onUnauthorizeUsr_B;
}

// void EVSE_getSsid(String &out) {
// 	out += "Pied Piper";
// }
// void EVSE_getPass(String &out) {
// 	out += "plmzaq123";
// }


// void EVSE_getChargePointSerialNumber(String &out) {


// 	out += preferences.getString("chargepoint","");

// 	/*
// 	#if STEVE
// 	out += "dummyCP002";
// 	#endif

// 	#if EVSECEREBRO
// 	out += "testpodpulkit";
// 	#endif
// 	*/
// }

// char *EVSE_getChargePointVendor() {
// 	return "Amplify Mobility";
// }

// char *EVSE_getChargePointModel() {
// 	return "Wx2";
// }

String EVSE_B_getCurrnetIdTag(MFRC522 * mfrc522) {
    String currentIdTag = "";
	//currentIdTag_B = EVSE_B_readRFID(mfrc522);
	
	if (getChargePointStatusService_B()->getIdTag().isEmpty() == false){
		if(DEBUG_OUT) Serial.println("[EVSE_B] Reading from Charge Point Station Service ID Tag stored.");
		currentIdTag = getChargePointStatusService_B()->getIdTag();
		if(DEBUG_OUT) Serial.print("[EVSE_B] ID Tag: ");
		if(DEBUG_OUT) Serial.println(currentIdTag);
		Serial.flush();
	}
	
	return currentIdTag;
}


String EVSE_B_readRFID(MFRC522 * mfrc522) {
	String currentIdTag_B;	
	currentIdTag_B = readRfidTag(true, mfrc522);
	return currentIdTag_B;
}

/********Added new funtion @Wamique***********************/

// void EVSE_B_stopTransactionByRfid(){

// 	Ext_currentIdTag_B = EVSE_B_readRFID(&mfrc522);
// 	if(currentIdTag_B.equals(Ext_currentIdTag_B) == true){
// 		flag_evRequestsCharge_B = false;
// 		flag_evseStopTransaction_B = true;
// 	}else{
// 			if(Ext_currentIdTag_B.equals("") == false)
// 			if(DEBUG_OUT) Serial.println("\n[EVSE_B] Incorrect ID tag\n");
// 		}
// }

#if CP_ACTIVE
/**************CP Implementation @mwh*************/
void EVSE_B_StartCharging(){

	if(getChargePointStatusService_B()->getEvDrawsEnergy() == false){
		getChargePointStatusService_B()->startEvDrawsEnergy();
	}
    if (DEBUG_OUT) Serial.print(F("[EVSE_B] Opening Relays.\n"));
 //   pinMode(32,OUTPUT);
  //  digitalWrite(32, HIGH); //RELAY_1
    //digitalWrite(RELAY_2, RELAY_HIGH);
    requestForRelay(START,2);
    requestLed(ORANGE,START,2);
    delay(1200);
    requestLed(WHITE,START,2);
    delay(1200);
    requestLed(GREEN,START,2);
    delay(1000);
    Serial.println("[EVS_B] EV is connected and Started charging");
	if(DEBUG_OUT) Serial.println("[EVSE_B] Started Drawing Energy");
	delay(500);
}


void EVSE_B_Suspended(){


	if(getChargePointStatusService_B()->getEvDrawsEnergy() == true){
		getChargePointStatusService_B()->stopEvDrawsEnergy();
	}
		requestLed(BLUE,START,2);     //replace 1 with connector ID
		requestForRelay(STOP,2);
	//	delay(1000);
		Serial.printf("[EVSE_B] EV Suspended");


}



/**************************************************/

#endif

void displayMeterValuesB(){
		if(millis() - timerDisplayB > 10000){
			timerDisplayB = millis();
			/*float instantCurrrent_A = eic.GetLineCurrentA();
			float instantVoltage_A  = eic.GetLineVoltageA();
			float instantPower_A = 0.0f;

			if(instantCurrrent_A < 0.15){
				instantPower_A = 0;
			}else{
				instantPower_A = (instantCurrrent_A * instantVoltage_A)/1000.0;
			}*/

			float instantCurrrent_B = eic.GetLineCurrentB();
			int instantVoltage_B  = eic.GetLineVoltageB();
			float instantPower_B = 0.0f;

			if(instantCurrrent_B < 0.15){
				instantPower_B = 0;
			}else{
				instantPower_B = (instantCurrrent_B * instantVoltage_B)/1000.0;
			}

			/*float instantCurrrent_C = eic.GetLineCurrentC();
			int instantVoltage_C = eic.GetLineVoltageC();
			float instantPower_C = 0.0f;

			if(instantCurrrent_C < 0.15){
				instantPower_C = 0;
			}else{
				instantPower_C = (instantCurrrent_C * instantVoltage_C)/1000.0;
			}*/
			delay(100); // just for safety
			#if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
  if (notFaulty_B)
  {
  lcd.print("*****CHARGING 2*****"); // You can make spaces using well... spaces
  }
  else
  {
	switch(fault_code_B)
	{
		case -1: break; //It means default. 
		case 0: lcd.print("Connector2-Over Voltage");
				break;
		case 1: lcd.print("Connector2-Under Voltage");
				break;
		case 2: lcd.print("Connector2-Over Current");
				break;
		case 3: lcd.print("Connector2-Under Current");
				break;
		case 4: lcd.print("Connector2-Over Temp");
				break;
		case 5: lcd.print("Connector2-Over Temp");
				break;
		case 6: lcd.print("Connector2-GFCI"); // Not implemented in AC001
				break;
		case 7: lcd.print("Connector2-Earth Disc");
				break;
		default: lcd.print("*****FAULTED 2*****"); // You can make spaces using well... spacesbreak;
	}
  }
  /*
  lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
  lcd.print("V:");
  lcd.setCursor(4, 1); // Or setting the cursor in the desired position.
  lcd.print(String(instantVoltage_A));
  lcd.setCursor(9, 1); // Or setting the cursor in the desired position.
  lcd.print(String(instantVoltage_B));
  lcd.setCursor(15, 1); // Or setting the cursor in the desired position.
  lcd.print(String(instantVoltage_C));
  lcd.setCursor(0, 2);
  lcd.print("I:");
  lcd.setCursor(4, 2); // Or setting the cursor in the desired position.
  lcd.print(String(instantCurrrent_A)); 
  lcd.setCursor(9, 2); // Or setting the cursor in the desired position.
  lcd.print(String(instantCurrrent_B)); 
  lcd.setCursor(15, 2); // Or setting the cursor in the desired position.
  lcd.print(String(instantCurrrent_C)); 
   lcd.setCursor(0, 3);
  lcd.print("P:"); 
  lcd.setCursor(4, 3); // Or setting the cursor in the desired position.
  lcd.print(String(instantPower_A));
  lcd.setCursor(9, 3); // Or setting the cursor in the desired position.
  lcd.print(String(instantPower_B));
  lcd.setCursor(15, 3); // Or setting the cursor in the desired position.
  lcd.print(String(instantPower_C));*/

  lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
  lcd.print("VOLTAGE(v):");
  lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
  lcd.print(String(instantVoltage_B));
  lcd.setCursor(0, 2);
  lcd.print("CURRENT(A):");
  lcd.setCursor(12, 2); // Or setting the cursor in the desired position.
  lcd.print(String(instantCurrrent_B)); 
   lcd.setCursor(0, 3);
  lcd.print("POWER(KW) :"); 
  lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
  lcd.print(String(instantPower_B));
#endif

#if DWIN_ENABLED
uint8_t err = 0;
  change_page[9] = 5;
  //v1[6] = instantVoltage_A >> 8;
  //v1[7] = instantVoltage_A & 0xff;
  v2[4] = 0X75;
  instantVoltage_B = instantVoltage_B * 10;
  v2[6] = instantVoltage_B >> 8;
  v2[7] = instantVoltage_B & 0xff;
  //v3[6] = instantVoltage_C >> 8;
  //v3[7] = instantVoltage_C & 0xff;
  //i1[7] = instantCurrrent_A*10;
  i2[4] = 0X77;
  i2[7] = instantCurrrent_B*10;
  //i3[7] = instantCurrrent_C*10;
  //e1[7] = instantPower_A*10;
  e2[4] = 0X79;
  e2[7] = instantPower_B*10;
  //e3[7] = instantPower_C*10;
  err = DWIN_SET(cid2,sizeof(cid2)/sizeof(cid2[0]));
  //delay(50);
  err = DWIN_SET(change_page,sizeof(change_page)/sizeof(change_page[0])); // page 0
  if (notFaulty_B)
  {
  charging[4] = 0X71;
  err = DWIN_SET(charging,sizeof(charging)/sizeof(charging[0]));
  }
  //delay(50);
  //err = DWIN_SET(charging,sizeof(charging)/sizeof(charging[0])); 
  //delay(50);
  //err = DWIN_SET(v1,sizeof(v1)/sizeof(v1[0])); 
  err = DWIN_SET(v2,sizeof(v2)/sizeof(v2[0])); 
  //err = DWIN_SET(v3,sizeof(v3)/sizeof(v3[0])); 
  //err = DWIN_SET(i1,sizeof(i1)/sizeof(i1[0])); 
  err = DWIN_SET(i2,sizeof(i2)/sizeof(i2[0])); 
  //err = DWIN_SET(i3,sizeof(i3)/sizeof(i3[0])); 
  //err = DWIN_SET(e1,sizeof(e1)/sizeof(e1[0])); 
  err = DWIN_SET(e2,sizeof(e2)/sizeof(e2[0])); 
  //err = DWIN_SET(e3,sizeof(e3)/sizeof(e3[0])); 
  delay(500);
#endif
	}
		
}