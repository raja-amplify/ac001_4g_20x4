#ifndef CNTRLP
#define CNTRLP
#include <Arduino.h>
#define PWM_PIN 	14   //GPIO15 Pilot Out
#define PWM_FREQUENCY 	1000 //1KHz
#define PWM_CHANNEL 	0
#define PWM_RESOLUTION 	8
/* States for EVSE state machine */
typedef enum {
	STARTUP,
	GFCI_CHECK_START,
	GFCI_CHECK_EXPECTED,
	GFCI_PASSED,
	FAULT,
	STATE_A, //5
	STATE_B, //6
	STATE_C,  //7
	STATE_D, //9
	STATE_E, //10
	STATE_SUS, //11
	STATE_DIS //12
	//STATE_F
}EVSE_states_enum;

typedef enum {
	GFCI_FAULT,
	GFCI_CHECK_FAULT,
	RELAY_FAULT,
	PILOT_FAULT,
	VOLTAGE_FAULT,
	GENERIC_FAULT
}fault_conditions;


typedef enum {
	V_12,
	V_9,
	V_SUS,
	V_DIS,
	V_6,
	V_3,
	V_1,
	V_UNKNOWN,
	V_DEFAULT
}PILOT_readings_t;



void SetPwmOn();
void SetPwmOff();
void ControlPSetup();

void ControlPBegin();
void ControlPRead();
int CalculateDutyCycle(float chargingLimit);
void ControlP_loop();
void SetStateA();

#endif