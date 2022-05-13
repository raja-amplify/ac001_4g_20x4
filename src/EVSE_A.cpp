// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

/**
Edited by Pulkit Agrawal.
*/

#include "EVSE_A.h"
#include "Master.h"
#include "ControlPilot.h"

#include "LCD_I2C.h"

#if DWIN_ENABLED
#include "dwin.h"

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
extern unsigned char cid1[7];
#endif
//new variable names defined by @Pulkit. might break the build.
OnBoot_A onBoot_A;
OnReadUserId_A onReadUserId_A;
OnSendHeartbeat_A onSendHeartbeat_A;
OnAuthentication_A onAuthentication_A;
OnStartTransaction_A onStartTransaction_A;
OnStopTransaction_A onStopTransaction_A;
OnUnauthorizeUser_A onUnauthorizeUser_A;

uint8_t currentCounterThreshold_A = 60;

//timeout for heartbeat signal.
ulong T_SENDHEARTBEAT = 60000;
bool timeout_active_A =false;
bool timer_initialize_A = false;
ulong timeout_start_A =0;
//Reason for stop
extern uint8_t reasonForStop;
//new flag names. replace them with old names.
bool evIsPlugged_A; 
bool flag_evseIsBooted_A;
bool flag_evseReadIdTag_A;
bool flag_evseAuthenticate_A;
bool flag_evseStartTransaction_A;
bool flag_evRequestsCharge_A;
bool flag_evseStopTransaction_A;
bool flag_evseUnauthorise_A;
bool flag_rebootRequired_A;
bool flag_evseSoftReset_A; //added by @Wamique

extern ATM90E36 eic;
extern bool flag_rebootRequired_B;
extern bool flag_rebootRequired_C;

//not used. part of Smart Charging System.
float chargingLimit_A = 32.0f;
String Ext_currentIdTag_A = "";
extern WebSocketsClient webSocket;

ulong timerDisplay;
bool EMGCY_FaultOccured = false;

extern LCD_I2C lcd;

extern MFRC522 mfrc522;
//extern String currentIdTag;
long int blinckCounter_A =0;
int counter1_A =0;

ulong t;
int connectorDis_counter_A=0;

String currentIdTag_A = "";
extern EVSE_states_enum EVSE_state;
extern Preferences preferences;

short int fault_counter_A = 0;
bool flag_faultOccured_A = false;

short int counter_drawingCurrent_A = 0;
float drawing_current_A = 0;

extern bool webSocketConncted;
extern bool isInternetConnected;
short counter_faultstate_A = 0;

//metering Flag
extern bool flag_MeteringIsInitialised;
extern MeteringService *meteringService;
extern bool wifi_connect;
extern bool gsm_connect;

Preferences resumeTxn_A;
extern TinyGsmClient client;

bool ongoingTxn_A;
String idTagData_A = "";

ulong relay_timer_A;
ulong faultTimer_A = 0;

//initialize function. called when EVSE is booting. 
//NOTE: It should be also called when there is a reset or reboot required. create flag to control that. @Pulkit
/**********************************************************/
void EVSE_A_StopSession(){

	if(getChargePointStatusService_A()->getEvDrawsEnergy() == true){
		getChargePointStatusService_A()->stopEvDrawsEnergy();
	}
	
	//digitalWrite(32, LOW);
  requestForRelay(STOP,1); 
  delay(500);           	
  flag_evseReadIdTag_A = false;
	flag_evseAuthenticate_A = false;
	flag_evseStartTransaction_A = false;
	flag_evRequestsCharge_A = false;
	flag_evseStopTransaction_A = true;
	flag_evseUnauthorise_A = false;
    Serial.println("[EVSE] Stopping Session : " +String(EVSE_state));
}
/**************************************************************************/


/**************************SetUp********************************************/
void EVSE_A_setup(){

	resumeTxn_A.begin("resume_A",false); //opening preferences in R/W mode
  	idTagData_A = resumeTxn_A.getString("idTagData_A","");
  	ongoingTxn_A = resumeTxn_A.getBool("ongoingTxn_A",false);

  	Serial.println("Stored ID:"+String(idTagData_A));
  	Serial.println("Ongoing Txn: "+String(ongoingTxn_A));

	EVSE_A_setOnBoot([]() {
		//this is not in loop, that is why we need not update the flag immediately to avoid multiple copies of bootNotification.
		OcppOperation *bootNotification = makeOcppOperation(&webSocket,	new BootNotification());
		initiateOcppOperation(bootNotification);
		bootNotification->setOnReceiveConfListener([](JsonObject payload) {

      	 if( flag_MeteringIsInitialised == false){
      	 	Serial.println(F("[SetOnBooT] Initializing metering services"));
      		meteringService->init(meteringService);
			   /*#if LCD_ENABLED
  				lcd.clear();
  				lcd.setCursor(0, 2);
  				lcd.print("TAP RFID/SCAN QR");
			   #endif*/
      	 }

      if (DEBUG_OUT) Serial.print(F("EVSE_setOnBoot Callback: Metering Services Initialization finished.\n"));

			flag_evseIsBooted_A = true; //Exit condition for booting. 	
			flag_evseReadIdTag_A = true; //Entry condition for reading ID Tag.
			flag_evseAuthenticate_A = false;
			flag_evseStartTransaction_A = false;
			flag_evRequestsCharge_A = false;
			flag_evseStopTransaction_A = false;
			flag_evseUnauthorise_A = false;
      if (DEBUG_OUT) Serial.print(F("EVSE_setOnBoot Callback: Closing Relays.\n"));

			if (DEBUG_OUT) Serial.print(F("EVSE_setOnBoot Callback: Boot successful. Calling Read User ID Block.\n"));
		});
	});

	EVSE_A_setOnReadUserId([] () {
		if (DEBUG_OUT) Serial.print(F("EVSE_A_setOnReadUserId Callback: EVSE_A waiting for User ID read...\n"));
		static ulong timerForRfid = millis();
		currentIdTag_A = "";  	
		idTagData_A = resumeTxn_A.getString("idTagData_A","");
  		ongoingTxn_A = resumeTxn_A.getBool("ongoingTxn_A",false);

  		if((wifi_connect == true)&& 														//Wifi Block
  			(getChargePointStatusService_A()->getEmergencyRelayClose() == false)&&
  			(webSocketConncted == true)&&
  			(WiFi.status() == WL_CONNECTED)&&
  			(isInternetConnected == true)){

  			if((idTagData_A != "") && (ongoingTxn_A == 1)){
  				currentIdTag_A = resumeTxn_A.getString("idTagData_A", "");
  				Serial.println(F("[EVSE_A_setOnReadUserId] Resuming Session"));
				  #if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(3, 2);
  lcd.print("SESSION A RESUME");
#endif
  				getChargePointStatusService_A()->authorize(currentIdTag_A); // so that Starttxn can easily fetch this data
  				requestLed(BLUE, START, 1);
  			}else{
				if(millis() - timerForRfid > 8000){ //timer for sending led request
	    		  	requestLed(GREEN,START,1);
	    		  	timerForRfid = millis();
    		  	}
			
				currentIdTag_A = EVSE_A_getCurrnetIdTag(&mfrc522);
				Serial.println(F("[Wifi]********RFID A**********"));
  			}

  		}else if((gsm_connect == true)&&													//GSM Block
  				(getChargePointStatusService_A()->getEmergencyRelayClose() == false)&&
  				(client.connected() == true)){

  			if((idTagData_A != "") && (ongoingTxn_A == 1)){
  				currentIdTag_A = resumeTxn_A.getString("idTagData_A", "");
  				getChargePointStatusService_A()->authorize(currentIdTag_A); // so that Starttxn can easily fetch this data
  				Serial.println(F("[EVSE_A_setOnReadUserId] Resuming Session"));
				#if LCD_ENABLED
  				lcd.clear();
  				lcd.setCursor(3, 2);
  				lcd.print("SESSION A RESUME");
				#endif
  				requestLed(BLUE, START, 1);
  			}else{
				if(millis() - timerForRfid > 8000){ //timer for sending led request
	    		  	requestLed(GREEN,START,1);
	    		  	timerForRfid = millis();
    		  	}
			
				currentIdTag_A = EVSE_A_getCurrnetIdTag(&mfrc522);
				Serial.println(F("[GSM]********RFID A**********"));
  			}


  		}

  		/*
		if((ongoingTxn_m == 1) && (idTagData_m != "") && 
		      (getChargePointStatusService_A()->getEmergencyRelayClose() == false) &&
		      (WiFi.status() == WL_CONNECTED)&&
		      (webSocketConncted == true)&&
		      (isInternetConnected == true)){   //giving priority to stored data
			currentIdTag_A = resumeTxn.getString("idTagData","");
			Serial.println("[EVSE_setOnReadUserId] Resuming Session");
      		requestLed(BLUE,START,1);
         
		}else 
		if((getChargePointStatusService_A()->getEmergencyRelayClose() == false) &&
		          (WiFi.status() == WL_CONNECTED) &&
		          (webSocketConncted == true) && 
		          (isInternetConnected == true)){
			  #if LED_ENABLED
			  if(millis() - timerForRfid > 10000){ //timer for sending led request
    		  requestLed(GREEN,START,1);
    		  timerForRfid = millis();
    		  }
    		  #endif
			currentIdTag_A = EVSE_A_getCurrnetIdTag(&mfrc522);
			Serial.println("********RFID A**********");
		}*/

		if (currentIdTag_A.equals("") == true) {
			flag_evseReadIdTag_A = true; //Looping back read block as no ID tag present.
			flag_evseAuthenticate_A = false;
			flag_evseStartTransaction_A = false;
			flag_evRequestsCharge_A = false;
			flag_evseStopTransaction_A = false;
			flag_evseUnauthorise_A = false;
		} else {
			flag_evseReadIdTag_A = false;
			flag_evseAuthenticate_A = true; //Entry condition for authentication block.
			flag_evseStartTransaction_A = false;
			flag_evRequestsCharge_A = false;
			flag_evseStopTransaction_A = false;
			flag_evseUnauthorise_A = false;
			if (DEBUG_OUT) Serial.print(F("EVSE_setOnReadUserId Callback: Successful User ID Read. Calling Authentication Block.\n"));
		}	
	});
	
	EVSE_A_setOnsendHeartbeat([] () {
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
 
	EVSE_A_setOnAuthentication([] () {
		if (DEBUG_OUT) Serial.print(F("EVSE_setOnAuthentication Callback: Authenticating...\n"));
		flag_evseAuthenticate_A = false;
		OcppOperation *authorize = makeOcppOperation(&webSocket, new Authorize(currentIdTag_A));
		initiateOcppOperation(authorize);
		/*chargePointStatusService->authorize(currentIdTag_A, connectorId_A);  */    //have to edit
		authorize->setOnReceiveConfListener([](JsonObject payload) {
			const char* status = payload["idTagInfo"]["status"] | "Invalid";
			if (!strcmp(status, "Accepted")) {
				flag_evseReadIdTag_A = false;
				flag_evseAuthenticate_A = false;
				flag_evseStartTransaction_A = true; //Entry condition for starting transaction.
				flag_evRequestsCharge_A = false;
				flag_evseStopTransaction_A = false;
				flag_evseUnauthorise_A = false;
				
				if (DEBUG_OUT) Serial.print(F("EVSE_setOnAuthentication Callback: Authorize request has been accepted! Calling StartTransaction Block.\n"));
				requestLed(BLUE,START,1);
				#if CP_ACTIVE 
				flag_controlPAuthorise = true;
				#endif

			} else {
				flag_evseReadIdTag_A = false;
				flag_evseAuthenticate_A = false;
				flag_evseStartTransaction_A = false;
				flag_evRequestsCharge_A = false;
				flag_evseStopTransaction_A = false;
				flag_evseUnauthorise_A = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
				if (DEBUG_OUT) Serial.print(F("EVSE_setOnAuthentication Callback: Authorize request has been denied! Read new User ID. \n"));
			}  
		});
	});
	
	EVSE_A_setOnStartTransaction([] () {
		flag_evseStartTransaction_A = false;
		String idTag = "";
		int connectorId = 0;

		if (getChargePointStatusService_A() != NULL) {
			if (!getChargePointStatusService_A()->getIdTag().isEmpty()) {
				idTag = String(getChargePointStatusService_A()->getIdTag());
				connectorId = getChargePointStatusService_A()->getConnectorId();
			}

		}
		OcppOperation *startTransaction = makeOcppOperation(&webSocket, new StartTransaction(idTag,connectorId));
		initiateOcppOperation(startTransaction);
		startTransaction->setOnReceiveConfListener([](JsonObject payload) {
    		const char* status = payload["idTagInfo"]["status"] | "Invalid";
      if (!strcmp(status, "Accepted") || !strcmp(status, "ConcurrentTx")) { 

      flag_evseReadIdTag_A = false;
      flag_evseAuthenticate_A = false;
      flag_evseStartTransaction_A = false;
      flag_evRequestsCharge_A = true;
      flag_evseStopTransaction_A = false;
      flag_evseUnauthorise_A = false;
      if (DEBUG_OUT) Serial.print(F("EVSE_setOnStartTransaction Callback: StartTransaction was successful\n"));
	  #if LCD_ENABLED
      lcd.clear();
      lcd.setCursor(3, 2);
      lcd.print("SESSION A STARTED");
     #endif
      /************************************/
      int transactionId = payload["transactionId"] | -1;
      getChargePointStatusService_A()->startTransaction(transactionId);        //new block for Three connector
      getChargePointStatusService_A()->startEnergyOffer();
      resumeTxn_A.putString("idTagData_A",getChargePointStatusService_A()->getIdTag());
      resumeTxn_A.putBool("ongoingTxn_A",true);

      //*****Storing tag data in EEPROM****//
      /*
	  resumeTxn.putString("idTagData",currentIdTag);
      resumeTxn.putBool("ongoingTxn",true);*/
      //***********************************//

      } else {
        flag_evseReadIdTag_A = false;
        flag_evseAuthenticate_A = false;
        flag_evseStartTransaction_A = false;
        flag_evRequestsCharge_A = false;
        flag_evseStopTransaction_A = false;
        flag_evseUnauthorise_A = true; //wrong ID tag received, so clearing the global current ID tag variable and setting up to read again.
        if (DEBUG_OUT) Serial.print(F("EVSE_setOnStartTransaction Callback: StartTransaction was unsuccessful\n"));
		#if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 2);
  lcd.print("SESSION A NOT STARTED");
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
			
	EVSE_A_setOnStopTransaction([] () {
		flag_evseStopTransaction_A = false;
		if (getChargePointStatusService_A() != NULL) {
          getChargePointStatusService_A()->stopEnergyOffer();
        }
        int txnId = getChargePointStatusService_A()->getTransactionId();
        int connector = getChargePointStatusService_A()->getConnectorId();
		OcppOperation *stopTransaction = makeOcppOperation(&webSocket, new StopTransaction(currentIdTag_A, txnId, connector));
		initiateOcppOperation(stopTransaction);
    	if (DEBUG_OUT) Serial.print(F("EVSE_A_setOnStopTransaction  before Callback: Closing Relays.\n"));
		 #if LCD_ENABLED
  		lcd.clear();
  		lcd.setCursor(3, 2);
  		lcd.print("STOP SESSION A");
		#endif

    	/**********************Until Offline functionality is implemented***********/
    	//Resume namespace(Preferences)
    	if(getChargePointStatusService_A()->getEmergencyRelayClose() == false){
    		requestLed(GREEN,START,1);   //temp fix
    	}
    	resumeTxn_A.putBool("ongoingTxn_A",false);
    	resumeTxn_A.putString("idTagData_A","");
    	if(wifi_connect == true){
	    	if(!webSocketConncted || WiFi.status() != WL_CONNECTED || isInternetConnected == false){
		        getChargePointStatusService_A()->stopTransaction();
		    	getChargePointStatusService_A()->unauthorize();  //can be buggy 
		        flag_evseReadIdTag_A = true;
		        flag_evseAuthenticate_A = false;
		        flag_evseStartTransaction_A = false;
		        flag_evRequestsCharge_A = false;
		        flag_evseStopTransaction_A = false;
		        flag_evseUnauthorise_A = false;
	    		Serial.println("Clearing Stored ID tag in StopTransaction()");
	    	}
    	}else if(gsm_connect == true){
    		 if(client.connected() == false){
    		 	getChargePointStatusService_A()->stopTransaction();
		    	getChargePointStatusService_A()->unauthorize();  //can be buggy 
		        flag_evseReadIdTag_A = true;
		        flag_evseAuthenticate_A = false;
		        flag_evseStartTransaction_A = false;
		        flag_evRequestsCharge_A = false;
		        flag_evseStopTransaction_A = false;
		        flag_evseUnauthorise_A = false;
	    		Serial.println("Clearing Stored ID tag in StopTransaction()");


    		 }
    	}

    	requestForRelay(STOP,1);
    
    	delay(500);
		stopTransaction->setOnReceiveConfListener([](JsonObject payload) {
			#if LCD_ENABLED
  			lcd.clear();
  			lcd.setCursor(3, 2);
  			lcd.print("STOP A SUCCESS");
			#endif
			flag_evseReadIdTag_A = false;
			flag_evseAuthenticate_A = false;
			flag_evseStartTransaction_A = false;
			flag_evRequestsCharge_A = false;
			flag_evseStopTransaction_A = false;
			flag_evseUnauthorise_A = true;
			getChargePointStatusService_A()->stopTransaction();
      		if (DEBUG_OUT) Serial.print(F("EVSE_setOnStopTransaction Callback: Closing Relays.\n"));
			if (DEBUG_OUT) Serial.print(F("EVSE_setOnStopTransaction Callback: StopTransaction was successful\n"));
			if (DEBUG_OUT) Serial.print(F("EVSE_setOnStopTransaction Callback: Reinitializing for new transaction. \n"));
			
		});
	});
	
	EVSE_A_setOnUnauthorizeUser([] () {
		if(flag_evseSoftReset_A == true){
			//This 'if' block is developed by @Wamique.
			flag_evseReadIdTag_A = false;
			flag_evseAuthenticate_A = false;
			flag_evseStartTransaction_A = false;
			flag_evRequestsCharge_A = false;
			flag_evseStopTransaction_A = false;
			flag_evseUnauthorise_A = false;
			flag_rebootRequired_A = true;
			getChargePointStatusService_A()->unauthorize();
			if (DEBUG_OUT) Serial.println(F("EVSE_setOnUnauthorizeUser Callback: Initiating Soft reset"));
		} else if(flag_evseSoftReset_A == false){
			flag_evseReadIdTag_A = true;
			flag_evseAuthenticate_A = false;
			flag_evseStartTransaction_A = false;
			flag_evRequestsCharge_A = false;
			flag_evseStopTransaction_A = false;
			flag_evseUnauthorise_A = false;
			if (DEBUG_OUT) Serial.print(F("EVSE_setOnUnauthorizeUser Callback: Unauthorizing user and setting up for new user ID read.\n"));
			getChargePointStatusService_A()->unauthorize();
			
		}
	});
}

/*********************************************************************/
void EVSE_A_initialize() {
	if(DEBUG_OUT) Serial.print(F("[EVSE] EVSE is powered on or reset. Starting Boot.\n"));
	onBoot_A();
	faultTimer_A = millis();
}

//This is the main loop function which is controlling the whole charfing process. All the flags created are used to control the flow of the program. 



void EVSE_A_loop() {
	
	if (flag_evseIsBooted_A == false){
		if(DEBUG_OUT) Serial.println(F("[EVSE] Booting..."));
		delay(1000);
		//onBoot();
		t = millis();
		return;
	} else if (flag_evseIsBooted_A == true) {
		if (flag_evseReadIdTag_A == true) {
			if (onReadUserId_A != NULL) {
				onReadUserId_A();
				if(millis() - t > 5000){
					t= millis();
				//	onSendHeartbeat_A();
				}
			}
			return;
		} else if (flag_evseAuthenticate_A == true) {
			if (onAuthentication_A != NULL) {
				onAuthentication_A();
				
			}
			return;
		} else if (flag_evseStartTransaction_A == true) {
			if (onStartTransaction_A != NULL) {
				#if CP_ACTIVE
				if((EVSE_state == STATE_C || EVSE_state == STATE_D) && getChargePointStatusService_A()->getEmergencyRelayClose() == false){
					onStartTransaction_A();
				}else{
					Serial.println(F("Connect the Connector to EV / Or fault exist"));     //here have to add timeout of 30 sec
					connectorDis_counter_A++;
					//EVSE_stopTransactionByRfid();
					if(connectorDis_counter_A > 25){
						connectorDis_counter_A = 0;

						EVSE_A_StopSession();
					}
					
				}
				#endif

				#if !CP_ACTIVE
					onStartTransaction_A();
				#endif

			}
		} else if (flag_evRequestsCharge_A == true){

		#if CP_ACTIVE
			//flag_evRequestsCharge = false;
				if (getChargePointStatusService_A() != NULL && getChargePointStatusService_A()->getEvDrawsEnergy() == false) {
				
			/***********************Control Pilot @Wamique******************/
					if(EVSE_state == STATE_C || EVSE_state == STATE_D){
						if (getChargePointStatusService_A()->getEmergencyRelayClose() == false) {
							EVSE_A_StartCharging();
						} else if (getChargePointStatusService_A()->getEmergencyRelayClose() == true) {
							Serial.println(F("The voltage / current / Temp is out or range. FAULTY CONDITION DETECTED."));
						}
					}else if(EVSE_state == STATE_SUS){  
						EVSE_A_Suspended();
						Serial.println(counter1);
						if(counter1++ > 25){    //Have to implement proper timeout
							counter1 = 0;
							EVSE_A_StopSession();
						}

					}else if(EVSE_state == STATE_DIS || EVSE_state == STATE_E || EVSE_state == STATE_B || EVSE_state == STATE_A){
				
					//	EVSE_StopSession();     // for the very first time cable can be in disconnected state

						//if(txn == true){           // can implement counter > 10 just to remove noise
						EVSE_A_StopSession();
				//	}

					}else{

						Serial.println("[EVSE] STATE Error" +String(EVSE_state));
						delay(2000);

					//	requestLed(RED,START,1);
					}
				} 
				if(getChargePointStatusService_A()->getEvDrawsEnergy() == true){

				//	txn = true;
				
					if(EVSE_state == STATE_C || EVSE_state == STATE_D ){

						if(DEBUG_OUT) Serial.println(F("[EVSE_CP] Drawing Energy"));	

						if(millis() - t > 10000){
				 			if(getChargePointStatusService_A()->getEmergencyRelayClose() == false){
				 				requestLed(BLINKYGREEN,START,1);
				 				t = millis();
				 			}
				 		}
						/*
						if(blinckCounter++ % 2 == 0){
							requestLed(GREEN,START,1);
						}else{
							requestLed(GREEN,STOP,1);
						}*/
					}else if(EVSE_state == STATE_A || EVSE_state == STATE_E || EVSE_state == STATE_B){//Although CP Inp will never go to A,B state
						if(counter_faultstate++ > 5){
						 EVSE_StopSession_A();
						 counter_faultstate = 0;
						}
                
					}else if(EVSE_state == STATE_SUS){
						EVSE_Suspended_A();    //pause transaction :update suspended state is considered in charging state

					}else if(EVSE_state == STATE_DIS){

						Serial.println(F("[EVSE] Connect the Connector with EV and Try again"));
						EVSE_StopSession_A();						
				}                      
			}

			 /***Implemented Exit Feature with RFID @Wamique****/ 
		//	  EVSE_stopTransactionByRfid_A();
		#endif

			
			#if !CP_ACTIVE
			if (getChargePointStatusService_A() != NULL && getChargePointStatusService_A()->getEvDrawsEnergy() == false) {
				if (getChargePointStatusService_A()->getEmergencyRelayClose() == false) {
					getChargePointStatusService_A()->startEvDrawsEnergy();
					
					if (DEBUG_OUT) Serial.print(F("[EVSE_A] Opening Relays.\n"));
					requestForRelay(START,1);
					requestLed(ORANGE,START,1);
    				delay(1200);
    				requestLed(WHITE,START,1);
    				delay(1200);
    				requestLed(GREEN,START,1);
   				 	delay(1000);
					if(DEBUG_OUT) Serial.println(F("[EVSE_A] Started Drawing Energy"));
					
				} else if (getChargePointStatusService_A()->getEmergencyRelayClose() == true) {
						Serial.println(F("The voltage or current is out or range. FAULTY CONDITION DETECTED."));
					}
			} 
			if(getChargePointStatusService_A()->getEvDrawsEnergy() == true){
				//delay(250);
				displayMeterValues();
				 if(DEBUG_OUT) Serial.println(F("[EVSE_A] Drawing Energy"));

				 //blinking green Led
				 if(millis() - t > 5000){
				 	// if((WiFi.status() == WL_CONNECTED) && (webSocketConncted == true) && (isInternetConnected == true)&& getChargePointStatusService()->getEmergencyRelayClose() == false){
				 	// 	requestLed(BLINKYGREEN_EINS,START,1);
				 	// 	t = millis();
				 	// }
				 //	onSendHeartbeat_A();
				if(getChargePointStatusService_A()->getEmergencyRelayClose() == false){
				 		requestLed(BLINKYGREEN,START,1);
				 		t = millis();

				 		if(millis() - relay_timer_A > 15000){
					 	
						 	requestForRelay(START,1);
						 	relay_timer_A = millis();

						}
				 	}

				}
				 //Current check
				 drawing_current_A = eic.GetLineCurrentA();
				 Serial.println("Current A: "+ String(drawing_current_A));
				 if(drawing_current_A <= 0.15){
				 	counter_drawingCurrent_A++;
				 	//if(counter_drawingCurrent_A > 120){
					if(counter_drawingCurrent_A > currentCounterThreshold_A){
				 		counter_drawingCurrent_A = 0;
						 reasonForStop = 1; // EV disconnected
						 #if LCD_ENABLED
  						lcd.clear();
  						lcd.setCursor(3, 0);
  						lcd.print("no power drawn"); 
						#endif
				 		Serial.println(F("Stopping session due to No current"));

				 		EVSE_A_StopSession();
				 	}

				 }else{
				 	counter_drawingCurrent_A = 0;
					currentCounterThreshold_A = 2;
				 	Serial.println(F("counter_drawing Current Reset"));

				 }
				
			}
			   //Implemented Exit Feature with RFID @Wamique//
			// EVSE_A_stopTransactionByRfid();
			#endif
			//this is the only 'else if' block which is calling next else if block. the control is from this file itself. the control is not changed from any other file. but the variables are required to be present as extern in other file to decide calling of other functions. 
			return;
		} else if (flag_evseStopTransaction_A == true) {
			if (getChargePointStatusService_A() != NULL) {
				getChargePointStatusService_A()->stopEvDrawsEnergy();

			}
			if (onStopTransaction_A != NULL) {
				onStopTransaction_A();
				#if CP_ACTIVE
				requestforCP_OUT(STOP);  //stop pwm
				#endif
			}
			return;
		} else if (flag_evseUnauthorise_A == true) {
			if (onUnauthorizeUser_A != NULL) {
				onUnauthorizeUser_A();
			}
			return;
		} else if (flag_rebootRequired_A == true && flag_rebootRequired_B == true && flag_rebootRequired_C == true) {
			//soft reset execution.
			// flag_evseIsBooted_A = false;
			// flag_rebootRequired_A = false;
			// flag_evseSoftReset_A = false;
			if(getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Charging &&
				getChargePointStatusService_B()->inferenceStatus() != ChargePointStatus::Charging &&
				getChargePointStatusService_C()->inferenceStatus() != ChargePointStatus::Charging
				){
					if(DEBUG_OUT) Serial.print(F("[EVSE_A] rebooting in 5 seconds...\n"));
					delay(5000);
					ESP.restart();
				}
		} else {
			if(DEBUG_OUT) Serial.print(F("[EVSE_A] waiting for response...\n"));
			delay(100);
		}	
	}
}

short EMGCY_counter_A =0;
bool EMGCY_FaultOccured_A = false;


void emergencyRelayClose_Loop_A(){
	if(millis() - faultTimer_A >2000){
		bool EMGCY_status_A = requestEmgyStatus();
		Serial.println("EMGCY_Status_A: "+String(EMGCY_status_A));
		if(EMGCY_status_A == true){
			if(EMGCY_counter_A++ > 0){
				requestForRelay(STOP,1);
				requestLed(BLINKYRED,START,1);
				getChargePointStatusService_A()->setEmergencyRelayClose(true);
				EMGCY_FaultOccured_A = true;
				EMGCY_counter_A = 0;
			}
		}else{
			EMGCY_FaultOccured_A = false;
			EMGCY_counter_A = 0;
			getChargePointStatusService_A()->setEmergencyRelayClose(false);
		}

		if(EMGCY_FaultOccured_A == true && getChargePointStatusService_A()->getTransactionId() != -1){

			flag_evseReadIdTag_A = false;
			flag_evseAuthenticate_A = false;
			flag_evseStartTransaction_A = false;
			flag_evRequestsCharge_A = false;
			flag_evseStopTransaction_A = true;
		
		}else if(EMGCY_FaultOccured_A == false){

				float volt = eic.GetLineVoltageA();
				float current = eic.GetLineCurrentA();
				float temp= eic.GetTemperature();
				Serial.println("Voltage_A: "+String(volt)+", Current_A: "+String(current)+", Temperature: "+String(temp));
				if (getChargePointStatusService_A() != NULL) {
					if(getChargePointStatusService_A()->getOverVoltage() == true ||
						getChargePointStatusService_A()->getUnderVoltage() == true ||
						getChargePointStatusService_A()->getUnderTemperature() == true ||
						getChargePointStatusService_A()->getOverTemperature() == true ||
						getChargePointStatusService_A()->getOverCurrent() == true){
							Serial.println(F("[EVSE_A] Fault Occurred."));
							getChargePointStatusService_A()->setEmergencyRelayClose(true);
							EMGCY_FaultOccured = true;						
							getChargePointStatusService_A()->setEmergencyRelayClose(true);
							if (!timer_initialize_A){
								timeout_start_A = millis();
								timer_initialize_A = true;
							}
						} else if(getChargePointStatusService_A()->getOverVoltage() == false &&
								getChargePointStatusService_A()->getUnderVoltage() == false &&
								getChargePointStatusService_A()->getUnderTemperature() == false &&
								getChargePointStatusService_A()->getOverTemperature() == false &&
								getChargePointStatusService_A()->getOverCurrent() == false){
							Serial.println(F("[EVSE_A] Not Faulty."));						
							getChargePointStatusService_A()->setEmergencyRelayClose(false);
							//if (!timer_initialize){
								timeout_start_A = 0;
								timer_initialize_A = false;
							//}
						}
						
					if (getChargePointStatusService_A()->getEmergencyRelayClose() == true){
						timeout_active_A = true;
						requestForRelay(STOP,1);
						delay(50);
						#if LED_ENABLED
						requestLed(RED,START,1);
						#endif

						flag_faultOccured_A = true;
					} else if (getChargePointStatusService_A()->getEmergencyRelayClose() == false && flag_faultOccured_A == true){
						timeout_active_A = false;
						if ( (getChargePointStatusService_A()->getTransactionId() != -1)){ //can be buggy
							if(fault_counter_A++ > 1){
								fault_counter_A = 0;
								requestForRelay(START,1);
								delay(50);
								Serial.println(F("[EmergencyRelay_A] Starting Txn"));
								flag_faultOccured_A = false;
							}
						}
					}

					if (timeout_active_A && getChargePointStatusService_A()->getTransactionId() != -1) {
						if (millis() - timeout_start_A >= TIMEOUT_EMERGENCY_RELAY_CLOSE){
							Serial.println(F("[EVSE_A] Emergency Stop."));
							flag_evRequestsCharge_A = false;
							flag_evseStopTransaction_A = true;
							timeout_active_A = false;
							timer_initialize_A = false;
						}
					}
				}
		    }
   }
}

/*
* @param limit: expects current in amps from 0.0 to 32.0
*/
void EVSE_A_setChargingLimit(float limit) {
	if(DEBUG_OUT) Serial.print(F("[EVSE] New charging limit set. Got "));
	if(DEBUG_OUT) Serial.print(limit);
	if(DEBUG_OUT) Serial.print(F("\n"));
	chargingLimit_A = limit;
}

bool EVSE_A_EvRequestsCharge() {
	return flag_evRequestsCharge_A;
}

bool EVSE_A_EvIsPlugged() {
	return evIsPlugged_A;
}


void EVSE_A_setOnBoot(OnBoot_A onBt_A){
	onBoot_A = onBt_A;
}

void EVSE_A_setOnReadUserId(OnReadUserId_A onReadUsrId_A){
	onReadUserId_A = onReadUsrId_A;
}

void EVSE_A_setOnsendHeartbeat(OnSendHeartbeat_A onSendHeartbt_A){
	onSendHeartbeat_A = onSendHeartbt_A;
}

void EVSE_A_setOnAuthentication(OnAuthentication_A onAuthenticatn_A){
	onAuthentication_A = onAuthenticatn_A;
}

void EVSE_A_setOnStartTransaction(OnStartTransaction_A onStartTransactn_A){
	onStartTransaction_A = onStartTransactn_A;
}

void EVSE_A_setOnStopTransaction(OnStopTransaction_A onStopTransactn_A){
	onStopTransaction_A = onStopTransactn_A;
}

void EVSE_A_setOnUnauthorizeUser(OnUnauthorizeUser_A onUnauthorizeUsr_A){
	onUnauthorizeUser_A = onUnauthorizeUsr_A;
}

// void EVSE_A_getSsid(String &out) {
// 	out += "Pied Piper";
// }
// void EVSE_getPass(String &out) {
// 	out += "plmzaq123";
// }


void EVSE_A_getChargePointSerialNumber(String &out) {


	out += preferences.getString("chargepoint","");

	/*
	#if STEVE
	out += "dummyCP002";
	#endif

	#if EVSECEREBRO
	out += "testpodpulkit";
	#endif
	*/
}



char *EVSE_A_getChargePointVendor() {
	return "Amplify Mobility";
}

char *EVSE_A_getChargePointModel() {
	return "AC001_3_1";
}

String EVSE_A_getCurrnetIdTag(MFRC522 * mfrc522) {
    String currentIdTag= "";
//	currentIdTag = EVSE_A_readRFID(mfrc522);    // masking rfid direct read from EVSE_A
	
	if (getChargePointStatusService_A()->getIdTag().isEmpty() == false){
		if(DEBUG_OUT) Serial.println(F("[EVSE] Reading from Charge Point Station Service ID Tag stored."));
		currentIdTag = getChargePointStatusService_A()->getIdTag();
		if(DEBUG_OUT) Serial.print(F("[EVSE] ID Tag: "));
		if(DEBUG_OUT) Serial.println(currentIdTag);
		Serial.flush();
	}
	
	return currentIdTag;
}


String EVSE_A_readRFID(MFRC522 * mfrc522) {
	String currentIdTag_A;	
	currentIdTag_A = readRfidTag(true, mfrc522);
	return currentIdTag_A;
}



/********Added new funtion @Wamique***********************/

// void EVSE_A_stopTransactionByRfid(){

// 	Ext_currentIdTag_A = EVSE_A_readRFID(&mfrc522);
// 	if(currentIdTag_A.equals(Ext_currentIdTag_A) == true){
// 		flag_evRequestsCharge_A = false;
// 		flag_evseStopTransaction_A = true;
// 	}else{
// 			if(Ext_currentIdTag_A.equals("")==false)
// 			if(DEBUG_OUT) Serial.println("\n[EVSE_A] Incorrect ID tag\n");
// 		}
// }



#if CP_ACTIVE
/**************CP Implementation @mwh*************/
void EVSE_A_StartCharging(){

	if(getChargePointStatusService_A()->getEvDrawsEnergy() == false){
		getChargePointStatusService_A()->startEvDrawsEnergy();
	}
    if (DEBUG_OUT) Serial.print(F("[EVSE_A] Opening Relays.\n"));
 //   pinMode(32,OUTPUT);
  //  digitalWrite(32, HIGH); //RELAY_1
    //digitalWrite(RELAY_2, RELAY_HIGH);
    requestForRelay(START,1);
    requestLed(ORANGE,START,1);
    delay(1200);
    requestLed(WHITE,START,1);
    delay(1200);
    requestLed(GREEN,START,1);
    delay(1000);
    Serial.println(F("[EVSE_A] EV is connected and Started charging"));
	if(DEBUG_OUT) Serial.println(F("[EVSE_A] Started Drawing Energy"));
	delay(500);
}


void EVSE_A_Suspended(){


	if(getChargePointStatusService_A()->getEvDrawsEnergy() == true){
		getChargePointStatusService_A()->stopEvDrawsEnergy();
	}
		requestLed(BLUE,START,1);
		requestForRelay(STOP,1);
	//	delay(1000);
		Serial.printf(F("[EVSE_A] EV Suspended"));


}



/**************************************************/

#endif

void displayMeterValues(){
		if(millis() - timerDisplay > 1000){
			timerDisplay = millis();
			float instantCurrrent_A = eic.GetLineCurrentA();
			int instantVoltage_A  = eic.GetLineVoltageA();
			float instantPower_A = 0.0f;

			if(instantCurrrent_A < 0.15){
				instantPower_A = 0;
			}else{
				instantPower_A = (instantCurrrent_A * instantVoltage_A)/1000.0;
			}

			float instantCurrrent_B = eic.GetLineCurrentB();
			int instantVoltage_B  = eic.GetLineVoltageB();
			float instantPower_B = 0.0f;

			if(instantCurrrent_B < 0.15){
				instantPower_B = 0;
			}else{
				instantPower_B = (instantCurrrent_B * instantVoltage_B)/1000.0;
			}

			float instantCurrrent_C = eic.GetLineCurrentC();
			int instantVoltage_C = eic.GetLineVoltageC();
			float instantPower_C = 0.0f;

			if(instantCurrrent_C < 0.15){
				instantPower_C = 0;
			}else{
				instantPower_C = (instantCurrrent_C * instantVoltage_C)/1000.0;
			}
			delay(100); // just for safety
			#if LCD_ENABLED
  lcd.clear();
  lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
  lcd.print("******CHARGING******"); // You can make spaces using well... spaces
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
  lcd.print(String(instantPower_C));
#endif

#if DWIN_ENABLED
uint8_t err = 0;
  change_page[9] = 4;
  v1[4] = 0X6A;
  instantVoltage_A = instantVoltage_A * 10;
  v1[6] = instantVoltage_A >> 8;
  v1[7] = instantVoltage_A & 0xff;
  /*v2[6] = instantVoltage_B >> 8;
  v2[7] = instantVoltage_B & 0xff;
  v3[6] = instantVoltage_C >> 8;
  v3[7] = instantVoltage_C & 0xff;*/
  i1[4] = 0X6C;
  i1[7] = instantCurrrent_A*10;
  /*i2[7] = instantCurrrent_B*10;
  i3[7] = instantCurrrent_C*10;*/
  e1[4] = 0X6E;
  e1[7] = instantPower_A*10;
  //e2[7] = instantPower_B*10;
  //e3[7] = instantPower_C*10;
  err = DWIN_SET(cid1,sizeof(cid1)/sizeof(cid1[0]));
  delay(50);
  err = DWIN_SET(change_page,sizeof(change_page)/sizeof(change_page[0])); // page 0
  delay(50);
  //err = DWIN_SET(charging,sizeof(charging)/sizeof(charging[0])); 
  //delay(50);
  err = DWIN_SET(v1,sizeof(v1)/sizeof(v1[0])); 
  //err = DWIN_SET(v2,sizeof(v2)/sizeof(v2[0])); 
  //err = DWIN_SET(v3,sizeof(v3)/sizeof(v3[0])); 
  err = DWIN_SET(i1,sizeof(i1)/sizeof(i1[0])); 
  //err = DWIN_SET(i2,sizeof(i2)/sizeof(i2[0])); 
  //err = DWIN_SET(i3,sizeof(i3)/sizeof(i3[0])); 
  err = DWIN_SET(e1,sizeof(e1)/sizeof(e1[0])); 
  //err = DWIN_SET(e2,sizeof(e2)/sizeof(e2[0])); 
  //err = DWIN_SET(e3,sizeof(e3)/sizeof(e3[0])); 
  delay(500);
  
#endif
	}
		
}