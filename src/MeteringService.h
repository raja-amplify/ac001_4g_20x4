// matth-x/ESP8266-OCPP
// Copyright Matthias Akstaller 2019 - 2020
// MIT License

#ifndef METERINGSERVICE_H
#define METERINGSERVICE_H

#define METER_VALUE_SAMPLE_INTERVAL 30 //in seconds

#define METER_VALUES_SAMPLED_DATA_MAX_LENGTH 1 //after 1 measurements, send the values to the CS

#include <LinkedList.h>
#include <Preferences.h>
#include "libraries/arduinoWebSockets-master/src/WebSocketsClient.h"
#include "TimeHelper.h"
#include "EEPROMLayout.h"
#include "ATM90E36.h"
#include "Variants.h"

//typedef float (*PowerSampler)();
typedef float (*EnergyASampler)(float volt, float current, time_t delta);
typedef float (*EnergyBSampler)(float volt, float current, time_t delta);
typedef float (*EnergyCSampler)(float volt, float current, time_t delta);


typedef float (*PowerASampler)(float volt, float current);
typedef float (*PowerBSampler)(float volt, float current);
typedef float (*PowerCSampler)(float volt, float current);

typedef float (*VoltageASampler)();
typedef float (*VoltageBSampler)();
typedef float (*VoltageCSampler)();
typedef float (*CurrentASampler)();
typedef float (*CurrentBSampler)();
typedef float (*CurrentCSampler)();
typedef float (*TemperatureSampler)();
extern ATM90E36 eic;

class MeteringService {
private:
  WebSocketsClient *webSocket;
  time_t lastSampleTimeA = 0; //0 means not charging right now
  time_t lastSampleTimeB = 0; //0 means not charging right now
  time_t lastSampleTimeC = 0; //0 means not charging right now
  float lastPower;
  float lastEnergyA;
  float lastEnergyB;
  float lastEnergyC;
  float lastVoltageA;
  float lastVoltageB;
  float lastVoltageC;
  float lastCurrentA;
  float lastCurrentB;
  float lastCurrentC;
  float lastTemperature;
  LinkedList<time_t> sampleTimeA;
  LinkedList<time_t> sampleTimeB;
  LinkedList<time_t> sampleTimeC;
  LinkedList<float> powerA;
  LinkedList<float> powerB;
  LinkedList<float> powerC;
  LinkedList<float> energyA;  
  LinkedList<float> energyB;  
  LinkedList<float> energyC;  
  LinkedList<float> voltageA;
  LinkedList<float> voltageB;
  LinkedList<float> voltageC;
  LinkedList<float> currentA;
  LinkedList<float> currentB;
  LinkedList<float> currentC;
  LinkedList<float> temperature;

  //float (*powerSampler)() = NULL;
  float (*energyASampler)(float volt, float current, time_t delta) = NULL;
  float (*energyBSampler)(float volt, float current, time_t delta) = NULL;
  float (*energyCSampler)(float volt, float current, time_t delta) = NULL;

  float (*powerASampler)(float volt, float current) = NULL;
  float (*powerBSampler)(float volt, float current) = NULL;
  float (*powerCSampler)(float volt, float current) = NULL;

  float (*voltageASampler)() = NULL;
  float (*voltageBSampler)() = NULL;
  float (*voltageCSampler)() = NULL;
  float (*currentASampler)() = NULL;
  float (*currentBSampler)() = NULL;
  float (*currentCSampler)() = NULL;
  float (*temperatureSampler)() = NULL; 
  void addDataPoint(time_t currentTime, float currentPower, float currentEnergy, float currentVoltage, float currentCurrent, float currentTemperature, int connector);
  bool timer_init = false;
  
public:
  MeteringService(WebSocketsClient *webSocket);

  void loop();
  void init(MeteringService *meteringService);
  float currentEnergy_A();
  float currentEnergy_B();
  float currentEnergy_C();

  //void setPowerSampler(PowerSampler powerSampler);
  void setEnergyASampler(EnergyASampler energySampler);
  void setEnergyBSampler(EnergyBSampler energySampler);
  void setEnergyCSampler(EnergyCSampler energySampler);

  void setPowerASampler(PowerASampler powerSampler);
  void setPowerBSampler(PowerBSampler powerSampler);
  void setPowerCSampler(PowerCSampler powerSampler);

  void setVoltageASampler(VoltageASampler voltageSampler);
  void setVoltageBSampler(VoltageBSampler voltageSampler);
  void setVoltageCSampler(VoltageCSampler voltageSampler);
  void setCurrentASampler(CurrentASampler currentSampler);
  void setCurrentBSampler(CurrentBSampler currentSampler);
  void setCurrentCSampler(CurrentCSampler currentSampler);
  void setTemperatureSampler(TemperatureSampler temperatureSampler);
  
  void flushLinkedListValues(int connector);
    
};
float roundingFloat(float var); //explicit funtion

#endif
