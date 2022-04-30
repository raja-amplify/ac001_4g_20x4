#include "ControlPilot.h"
#include "Master.h"
/*
const int pwm_pin 		 	= PWM_PIN;
const int pwm_freq 		 	= PWM_FREQUENCY;
const int pwm_channel 	 	= PWM_CHANNEL;
const int pwm_resolution   	= PWM_RESOLUTION;

const int potPin = 34;    //Pilot In  GPIO35

*/
bool flag_controlPAuthorise = false; 
//int dutyCycle = 64; //Hard coded for 15Amps
float defaultCurrentLimit = 32.0f;   // Take care for every new Charge point //To slave Hardcode @anesh
extern float chargingLimit;
//bool flag_pwm = false;
EVSE_states_enum EVSE_state;

PILOT_readings_t PILOT_reading;

/*
void SetPwmOn(){

	if((EVSE_state == STATE_B || EVSE_state == STATE_C || EVSE_state == STATE_D)){
		//start pwm
		int dutyC = CalculateDutyCycle(chargingLimit);
		Serial.println("[ControlPilot]Set maximum Current limit->" + String(chargingLimit));
		ledcWrite(pwm_channel,dutyC);     //To Slave
		Serial.println("[ControlPilot] Pwm ON");	
	}else{

		Serial.print("[ControlPilot] Incorrect state for Turning ON Pwm: " + String(EVSE_state));
	}
}


void SetPwmOff(){  // testing phase as its not working the way it has to.

	//	ledcWrite(pwm_channel,0);
		ledcWrite(pwm_channel,255);                //To Slave
		Serial.println("[ControlPilot] Pwm OFF");
	
}
*/
/*
void SetStateA(){
	ledcWrite(pwm_channel,255);              //To slave
//	digitalWrite(pwm_pin,HIGH);
	Serial.println("[ControlPilot] STATE_A is Set");

}

void ControlPSetup(){             //To slave
	//pinmode(pwm_pin,OUTPUT);
	ledcSetup(pwm_channel, pwm_freq, pwm_resolution); //configured functionalities

	ledcAttachPin(pwm_pin , pwm_channel);

	analogReadResolution(12);
}
*/
/*
void ControlPBegin(){
	ledcWrite(pwm_channel,255);     //to slave
	EVSE_state = STATE_A; 
	Serial.println("[ControlPilot] EVSE_state is STATE_A");

}
*/

void ControlPRead(){


	int ADC_Result = 0;
	/*
	for(int i=0 ; i<50 ;i++){
 
		ADC_Result += analogRead(potPin);//Took 5 samples to remove any noise  //To slave
	}

	ADC_Result = ADC_Result/50;   

	Serial.println("[ControlPilot] ADC Value : ");
	Serial.println(ADC_Result);
	*/

	ADC_Result = requestforCP_IN();
	Serial.println(F("[ControlPilot] ADC Value : "));
	Serial.println(ADC_Result);
	/* ADC result is between 3.2 and 2.8 volts */
	if ((ADC_Result < 4096) && /*(ADC_Result > 935)*/(ADC_Result > 3500))
	{
		PILOT_reading = V_12;
	}

	/* ADC result is between 2.8 and 2.33 volts */
	else if ((ADC_Result < 3500) && (ADC_Result > 2800))
	{
		PILOT_reading = V_9;
	}

	/* ADC result is between 1.49 and 1.32 volts */    //implemented based on observation
	else if ((ADC_Result < 1750) && (ADC_Result > 1500))
	{
		PILOT_reading = V_SUS;
	}

	/* ADC result is between 1.73 and 1.49 volts */
	else if ((ADC_Result < 2100) && (ADC_Result > 1750))
	{
		PILOT_reading = V_DIS;
	}

	/* ADC result is between 1.32 and 1.08 volts */
	else if ((ADC_Result < 1500) && (ADC_Result > 1150))
	{
		PILOT_reading = V_6;
	}

	/* ADC result is between 1.08 and 0.60 volts */
	else if ((ADC_Result < 1150 ) && (ADC_Result > 850))//testing)
	{
		PILOT_reading = V_3;
	}

	else if ((ADC_Result < 850) ) //0.4V
	{
		PILOT_reading = V_UNKNOWN;
	}

	else
	{	
		PILOT_reading = V_DEFAULT;
		Serial.println(F("ADC values are not in range"));
	}
	
}
/*

int CalculateDutyCycle(float chargingLimit){

	int dutycycle_l = 0;

	if((chargingLimit <= 51) &&(chargingLimit > 5)){

		dutycycle_l = ((chargingLimit / 0.6) * 2.55);
		Serial.println("[ControlPilot] Duty Cycle is = " + String(dutycycle_l));

	}else if((chargingLimit < 80) &&(chargingLimit > 51)){

		dutycycle_l = (((chargingLimit / 2.5) + 64 ) * 2.55 );
		Serial.println("[ControlPilot] Duty Cycle is = " + String(dutycycle_l));

	}else{

		Serial.println("[ControlPilot] chargingLimit is not in range");
	}

	return dutycycle_l;
}

*/
void ControlP_loop(){

/* Future Implementation @mwh*/
/*	if(defaultCurrentLimit != chargingLimit)
	{
		
		if(EVSE_state == STATE_C || EVSE_state == STATE_D){
			int newDutyCyle = CalculateDutyCycle(chargingLimit);
			Serial.println("Charging Limit is changed to " + String(chargingLimit) + "\n Starting new PWM Signal");
			ledcWrite(pwm_channel,newDutyCyle); //setting new pwm 

			defaultCurrentLimit = chargingLimit;
		}else{
			defaultCurrentLimit = chargingLimit;
			Serial.println("[ControlPilot] Charging limit is changed");
		}
	}*/
/******************************************/
	ControlPRead();
	delay(100);

	if(PILOT_reading == V_12){

		EVSE_state = STATE_A;
		Serial.println(F("[ControlPilot] State A"));

	}else if(PILOT_reading == V_9){

		EVSE_state = STATE_B;
		Serial.println(F("[ControlPilot] State B"));

	}else if(PILOT_reading == V_SUS){

		EVSE_state = STATE_SUS;
		Serial.println(F("[ControlPilot] State SUSPENDED"));

	}else if(PILOT_reading == V_DIS){

		EVSE_state = STATE_DIS;
		Serial.println(F("[ControlPilot] State DISCONNECTED"));

	}else if(PILOT_reading == V_6){

		EVSE_state = STATE_C;
		Serial.println(F("[ControlPilot] State C"));

	}else if(PILOT_reading == V_3){

		EVSE_state = STATE_D;
		Serial.println(F("[ControlPilot] State D"));

	}else if(PILOT_reading == V_UNKNOWN){   // can Implement here defaults state

		EVSE_state = STATE_E; //error
		Serial.println(F("[ControlPilot] State E"));

	}else{

		Serial.println(F("[ControlPilot] Unknown State :ADC value not in range"));
	}

/*	while(EVSE_state == STATE_E){
		delay(1000);
		ControlPRead();
	}
*/
	if(EVSE_state == STATE_B){
		if (flag_controlPAuthorise == true){
		//SetPwmOn();
	//	Serial.println(flag_controlPAuthorise);
		requestforCP_OUT(START);
		delay(500);
		flag_controlPAuthorise = false;
		}
	}



}