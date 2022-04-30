#include "PowerCycle.h"
#include "display.h"
#include<Preferences.h>
ulong cycleTimer = 0;
short waitCounter = 0;
bool ongoingTxn_p;
String idTagData_p="";
extern Preferences resumeTxn;

void PowerCycleInit(){

	cycleTimer = millis();
}
/*
void PowerCycle_Loop(){

	if(millis() - cycleTimer > 43200000){
		Serial.println("PowerCycle is Scheduled: "+String(waitCounter));
		// idTagData_p= resumeTxn.getString("idTagData","");
  // 		ongoingTxn_p = resumeTxn.getBool("ongoingTxn",false);
		// Serial.println("Stored ID:"+String(idTagData_p));
  // 		Serial.println("Ongoing Txn: "+String(ongoingTxn_p));
		if(getChargePointStatusService_A()->inferenceStatus() != ChargePointStatus::Charging &&
			getChargePointStatusService_B()->inferenceStatus() != ChargePointStatus::Charging &&
			 getChargePointStatusService_C()->inferenceStatus() != ChargePointStatus::Charging){
			if(waitCounter++ > 2){ //to allow some buffer time and let things stablize
				waitCounter = 0;   //not required still following convention
				Serial.println("Rebooting Device in 5 sec");
				thanks_Disp("Please wait....");
				delay(2000);
				ESP.restart();
			}
		}
	}
}*/