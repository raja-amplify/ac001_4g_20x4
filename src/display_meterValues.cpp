#include "display_meterValues.h"
#include "Variants.h"
#include "ATM90E36.h"

extern ATM90E36 eic;
extern bool EMGCY_FaultOccured_A;
extern bool EMGCY_FaultOccured_B;
extern bool EMGCY_FaultOccured_C;


#if DWIN_ENABLED
#include "dwin.h"
unsigned long onTime = 0;
uint8_t state_timer = 0;
uint8_t disp_evse = 0;
extern bool disp_evse_A;
extern bool disp_evse_B;
extern bool disp_evse_C;
extern bool notFaulty_A;
extern bool notFaulty_B;
extern bool notFaulty_C;
extern int8_t fault_code_A;
extern int8_t fault_code_B;
extern int8_t fault_code_C;

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
extern unsigned char CONN_UNAVAIL[28];
extern bool flag_faultOccured_A;
extern bool flag_faultOccured_B;
extern bool flag_faultOccured_C;
extern unsigned char v1[8];
extern unsigned char v2[8];
extern unsigned char v3[8];
extern unsigned char i1[8];
extern unsigned char i2[8];
extern unsigned char i3[8];
extern unsigned char e1[8];
extern unsigned char e2[8];
extern unsigned char e3[8];
extern unsigned char charging[28];
extern unsigned char cid1[7];
extern unsigned char cid2[7];
extern unsigned char cid3[7];
extern unsigned char unavail[30];

void stateTimer()
{
  switch (state_timer)
  {
    case 0:
      onTime = millis();
      state_timer = 1;
      disp_evse = 1;
      break;
    case 1:
      if ((millis() - onTime) > 3000)
      {
        state_timer = 2;
      }
      break;
    case 2:
      onTime = millis();
      state_timer = 3;
      disp_evse = 2;
      break;
    case 3:
      if ((millis() - onTime) > 3000)
      {
        state_timer = 4;
      }
      break;
    case 4:
      onTime = millis();
      state_timer = 5;
      disp_evse = 3;
      break;
    case 5:
      if ((millis() - onTime) > 3000)
      {
        state_timer = 6;
      }
      break;
    case 6:
      state_timer = 0;
  }
}

uint8_t avail_counter = 0;

void disp_dwin_meter()
{
  uint8_t err = 0;
  float instantCurrrent_A = eic.GetLineCurrentA();
  int instantVoltage_A  = eic.GetLineVoltageA();
  float instantPower_A = 0.0f;

  if (instantCurrrent_A < 0.15) {
    instantPower_A = 0;
  } else {
    instantPower_A = (instantCurrrent_A * instantVoltage_A) / 1000.0;
  }

  float instantCurrrent_B = eic.GetLineCurrentB();
  int instantVoltage_B  = eic.GetLineVoltageB();
  float instantPower_B = 0.0f;

  if (instantCurrrent_B < 0.15) {
    instantPower_B = 0;
  } else {
    instantPower_B = (instantCurrrent_B * instantVoltage_B) / 1000.0;
  }

  float instantCurrrent_C = eic.GetLineCurrentC();
  int instantVoltage_C = eic.GetLineVoltageC();
  float instantPower_C = 0.0f;

  if (instantCurrrent_C < 0.15) {
    instantPower_C = 0;
  } else {
    instantPower_C = (instantCurrrent_C * instantVoltage_C) / 1000.0;
  }
  switch (disp_evse)
  {
    case 1: if (disp_evse_A)
      {
        if (notFaulty_A && !EMGCY_FaultOccured_A)
        {
          change_page[9] = 4;

          v1[4] = 0X6A;
          instantVoltage_A = instantVoltage_A * 10;
          v1[6] = instantVoltage_A >> 8;
          v1[7] = instantVoltage_A & 0xff;
          i1[4] = 0X6C;
          i1[7] = instantCurrrent_A * 10;
          e1[4] = 0X6E;
          e1[7] = instantPower_A * 10;
          err = DWIN_SET(cid1, sizeof(cid1) / sizeof(cid1[0]));
          err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0])); // page 0
          charging[4] = 0X66;
          charging[5] = 0X00;
          err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));
          err = DWIN_SET(v1, sizeof(v1) / sizeof(v1[0]));
          err = DWIN_SET(i1, sizeof(i1) / sizeof(i1[0]));
          err = DWIN_SET(e1, sizeof(e1) / sizeof(e1[0]));
        }
      }
      break;
    case 2: if (disp_evse_B)
      {
        if (notFaulty_B && !EMGCY_FaultOccured_B)
        {

          change_page[9] = 5;

          v2[4] = 0X75;
          instantVoltage_B = instantVoltage_B * 10;
          v2[6] = instantVoltage_B >> 8;
          v2[7] = instantVoltage_B & 0xff;

          i2[4] = 0X77;
          i2[7] = instantCurrrent_B * 10;

          e2[4] = 0X79;
          e2[7] = instantPower_B * 10;

          err = DWIN_SET(cid2, sizeof(cid2) / sizeof(cid2[0]));

          err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0])); // page 0

          charging[4] = 0X71;
          charging[5] = 0X00;
          err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));


          err = DWIN_SET(v2, sizeof(v2) / sizeof(v2[0]));

          err = DWIN_SET(i2, sizeof(i2) / sizeof(i2[0]));

          err = DWIN_SET(e2, sizeof(e2) / sizeof(e2[0]));

        }
      }

      break;
    case 3: if (disp_evse_C)
      {
        if (notFaulty_C && !EMGCY_FaultOccured_C)
        {
          change_page[9] = 6;

          v3[4] = 0X7F;
          instantVoltage_C = instantVoltage_C * 10;
          v3[6] = instantVoltage_C >> 8;
          v3[7] = instantVoltage_C & 0xff;

          i3[4] = 0X82;
          i3[7] = instantCurrrent_C * 10;

          e3[4] = 0X84;
          e3[7] = instantPower_C * 10;
          err = DWIN_SET(cid3, sizeof(cid3) / sizeof(cid3[0]));

          err = DWIN_SET(change_page, sizeof(change_page) / sizeof(change_page[0])); // page 0

          charging[4] = 0X7B;
          charging[5] = 0X00;
          err = DWIN_SET(charging, sizeof(charging) / sizeof(charging[0]));

          err = DWIN_SET(v3, sizeof(v3) / sizeof(v3[0]));

          err = DWIN_SET(i3, sizeof(i3) / sizeof(i3[0]));

          err = DWIN_SET(e3, sizeof(e3) / sizeof(e3[0]));
        }
      }


      break;
    default: Serial.println(F("**Display default**"));
      break;
  }

}

#endif

#if DISPLAY_ENABLED
unsigned long onTime = 0;
uint8_t state_timer = 0;
uint8_t disp_evse = 0;
extern bool disp_evse_A;
extern bool disp_evse_B;
extern bool disp_evse_C;
extern bool notFaulty_A;
extern bool notFaulty_B;
extern bool notFaulty_C;
extern int8_t fault_code_A;
extern int8_t fault_code_B;
extern int8_t fault_code_C;
void stateTimer()
{
  switch (state_timer)
  {
    case 0:
      onTime = millis();
      state_timer = 1;
      disp_evse = 1;
      break;
    case 1:
      if ((millis() - onTime) > 3000)
      {
        state_timer = 2;
      }
      break;
    case 2:
      onTime = millis();
      state_timer = 3;
      disp_evse = 2;
      break;
    case 3:
      if ((millis() - onTime) > 3000)
      {
        state_timer = 4;
      }
      break;
    case 4:
      onTime = millis();
      state_timer = 5;
      disp_evse = 3;
      break;
    case 5:
      if ((millis() - onTime) > 3000)
      {
        state_timer = 6;
      }
      break;
    case 6:
      state_timer = 0;
  }
}

uint8_t avail_counter = 0;

void disp_lcd_meter()
{
  float instantCurrrent_A = eic.GetLineCurrentA();
  float instantVoltage_A  = eic.GetLineVoltageA();
  float instantPower_A = 0.0f;

  if (instantCurrrent_A < 0.15) {
    instantPower_A = 0;
  } else {
    instantPower_A = (instantCurrrent_A * instantVoltage_A) / 1000.0;
  }

  float instantCurrrent_B = eic.GetLineCurrentB();
  int instantVoltage_B  = eic.GetLineVoltageB();
  float instantPower_B = 0.0f;

  if (instantCurrrent_B < 0.15) {
    instantPower_B = 0;
  } else {
    instantPower_B = (instantCurrrent_B * instantVoltage_B) / 1000.0;
  }

  float instantCurrrent_C = eic.GetLineCurrentC();
  int instantVoltage_C = eic.GetLineVoltageC();
  float instantPower_C = 0.0f;

  if (instantCurrrent_C < 0.15) {
    instantPower_C = 0;
  } else {
    instantPower_C = (instantCurrrent_C * instantVoltage_C) / 1000.0;
  }
  switch (disp_evse)
  {
    case 1: if (disp_evse_A)
      {
        if (notFaulty_A && !EMGCY_FaultOccured_A)
        {
          //connector, voltage, current, power
          displayEnergyValues_Disp_AC("1", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
        }
        else
        {
          avail_counter ++;
          switch (fault_code_A)
          {
            case -1: break; //It means default.
            case 0: displayEnergyValues_Disp_AC("1-Over Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 1:  displayEnergyValues_Disp_AC("1-Under Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 2: displayEnergyValues_Disp_AC("1-Over Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 3: displayEnergyValues_Disp_AC("1-Under Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 4: displayEnergyValues_Disp_AC("1-Over Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));

              break;
            case 5: displayEnergyValues_Disp_AC("1-Under Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 6: displayEnergyValues_Disp_AC("1-GFCI", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 7:  displayEnergyValues_Disp_AC("1-Earth Disc", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            default: SerialMon.println(F("Default in display"));
          }
        }
        checkForResponse_Disp();
      }
      else // we shall use this case to refresh home page.
      {
        if (!notFaulty_A || EMGCY_FaultOccured_A)
        {
          avail_counter++;
        }

      }

      break;
    case 2: if (disp_evse_B)
      {
        if (notFaulty_B && !EMGCY_FaultOccured_B)
        {
          displayEnergyValues_Disp_AC("2", String(instantVoltage_B), String(instantCurrrent_B), String(instantPower_B));
        }
        else
        {
          avail_counter++;
          switch (fault_code_B)
          {
            case -1: break; //It means default.
            case 0: displayEnergyValues_Disp_AC("2-Over Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 1:  displayEnergyValues_Disp_AC("2-Under Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 2: displayEnergyValues_Disp_AC("2-Over Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 3: displayEnergyValues_Disp_AC("2-Under Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 4: displayEnergyValues_Disp_AC("2-Over Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));

              break;
            case 5: displayEnergyValues_Disp_AC("2-Under Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 6: displayEnergyValues_Disp_AC("2-GFCI", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 7:  displayEnergyValues_Disp_AC("2-Earth Disc", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            default: SerialMon.println(F("Default in display"));
          }
        }
        checkForResponse_Disp();
      }
      else
      {
        if (!notFaulty_B || EMGCY_FaultOccured_B)
        {
          avail_counter++;
        }

      }

      break;
    case 3: if (disp_evse_C)
      {
        if (notFaulty_C && !EMGCY_FaultOccured_C)
        {
          displayEnergyValues_Disp_AC("3", String(instantVoltage_C), String(instantCurrrent_C), String(instantPower_C));
        }
        else
        {
          avail_counter++;
          switch (fault_code_C)
          {
            case -1: break; //It means default.
            case 0: displayEnergyValues_Disp_AC("3-Over Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 1:  displayEnergyValues_Disp_AC("3-Under Voltage", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 2: displayEnergyValues_Disp_AC("3-Over Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 3: displayEnergyValues_Disp_AC("3-Under Current", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 4: displayEnergyValues_Disp_AC("3-Over Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));

              break;
            case 5: displayEnergyValues_Disp_AC("3-Under Temp", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 6: displayEnergyValues_Disp_AC("3-GFCI", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            case 7:  displayEnergyValues_Disp_AC("3-Earth Disc", String(instantVoltage_A), String(instantCurrrent_A), String(instantPower_A));
              break;
            default: SerialMon.println(F("Default in display"));
          }
        }

        checkForResponse_Disp();

      }
      else
      {
        if (!notFaulty_C || EMGCY_FaultOccured_C)
        {
          avail_counter++;
        }


      }


      break;
    default: Serial.println(F("**Display default**"));
      break;
  }
  /*
    @brief: If all 3 are faulted, then set the response to rfid unavailable.
  */
  if (avail_counter == 3)
  {
    setHeader("RFID UNAVAILABLE");
    checkForResponse_Disp();
    avail_counter = 0;
  }

  if (flag_tapped)
  {
    setHeader("TAP RFID TO START/STOP");
    checkForResponse_Disp();
    flag_tapped = false;
  }

  if (flag_unfreeze)
  {
    if (disp_evse_A == false)
    {
      if (disp_evse_B == false)
      {
        if (disp_evse_C == false)
        {
          flag_freeze = false;
          flag_unfreeze = false;
        }
      }
    }
  }

  //if(disp_evse_A || disp_evse_B || disp_evse_C)
  if (flag_freeze)
  {
    Serial.println(F("**skip**"));
  }
  else
  {
    if (isInternetConnected)
    {
      if (notFaulty_A && !EMGCY_FaultOccured_A && !disp_evse_A)
      {
        connAvail(1, "AVAILABLE");
        checkForResponse_Disp();
        setHeader("TAP RFID TO START/STOP");
        checkForResponse_Disp();
      }
      if (notFaulty_B && !EMGCY_FaultOccured_B && !disp_evse_B)
      {
        connAvail(2, "AVAILABLE");
        checkForResponse_Disp();
        setHeader("TAP RFID TO START/STOP");
        checkForResponse_Disp();
      }
      if (notFaulty_C && !EMGCY_FaultOccured_C && !disp_evse_C)
      {
        connAvail(3, "AVAILABLE");
        checkForResponse_Disp();
        setHeader("TAP RFID TO START/STOP");
        checkForResponse_Disp();
      }


    }
  }
}

#endif

#if LCD_ENABLED
unsigned long onTime = 0;
uint8_t state_timer = 0;
uint8_t disp_evse = 0;
extern bool disp_evse_A;
extern bool disp_evse_B;
extern bool disp_evse_C;
extern bool notFaulty_A;
extern bool notFaulty_B;
extern bool notFaulty_C;
extern int8_t fault_code_A;
extern int8_t fault_code_B;
extern int8_t fault_code_C;
void stateTimer()
{
  switch (state_timer)
  {
    case 0:
      onTime = millis();
      state_timer = 1;
      disp_evse = 1;
      break;
    case 1:
      if ((millis() - onTime) > 3000)
      {
        state_timer = 2;
      }
      break;
    case 2:
      onTime = millis();
      state_timer = 3;
      disp_evse = 2;
      break;
    case 3:
      if ((millis() - onTime) > 3000)
      {
        state_timer = 4;
      }
      break;
    case 4:
      onTime = millis();
      state_timer = 5;
      disp_evse = 3;
      break;
    case 5:
      if ((millis() - onTime) > 3000)
      {
        state_timer = 6;
      }
      break;
    case 6:
      state_timer = 0;
  }
}

void disp_lcd_meter()
{
  float instantCurrrent_A = eic.GetLineCurrentA();
  float instantVoltage_A  = eic.GetLineVoltageA();
  float instantPower_A = 0.0f;

  if (instantCurrrent_A < 0.15) {
    instantPower_A = 0;
  } else {
    instantPower_A = (instantCurrrent_A * instantVoltage_A) / 1000.0;
  }

  float instantCurrrent_B = eic.GetLineCurrentB();
  int instantVoltage_B  = eic.GetLineVoltageB();
  float instantPower_B = 0.0f;

  if (instantCurrrent_B < 0.15) {
    instantPower_B = 0;
  } else {
    instantPower_B = (instantCurrrent_B * instantVoltage_B) / 1000.0;
  }

  float instantCurrrent_C = eic.GetLineCurrentC();
  int instantVoltage_C = eic.GetLineVoltageC();
  float instantPower_C = 0.0f;

  if (instantCurrrent_C < 0.15) {
    instantPower_C = 0;
  } else {
    instantPower_C = (instantCurrrent_C * instantVoltage_C) / 1000.0;
  }
  switch (disp_evse)
  {
    case 1: if (disp_evse_A)
      {
        lcd.clear();
        lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
        if (notFaulty_A)
        {
          lcd.print("*****CHARGING 1*****"); // You can make spaces using well... spaces
          lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
          lcd.print("VOLTAGE(v):");
          lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
          lcd.print(String(instantVoltage_A));
          lcd.setCursor(0, 2);
          lcd.print("CURRENT(A):");
          lcd.setCursor(12, 2); // Or setting the cursor in the desired position.
          lcd.print(String(instantCurrrent_A));
          lcd.setCursor(0, 3);
          lcd.print("POWER(KW) :");
          lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
          lcd.print(String(instantPower_A));
        }
        else
        {

          switch (fault_code_A)
          {
            case -1: break; //It means default.
            case 0: lcd.print("Connector1-Over Voltage");
              break;
            case 1: lcd.print("Connector1-Under Voltage");
              break;
            case 2: lcd.print("Connector1-Over Current");
              break;
            case 3: lcd.print("Connector1-Under Current");
              break;
            case 4: lcd.print("Connector1-Over Temp");
              break;
            case 5: lcd.print("Connector1-Under Temp");
              break;
            case 6: lcd.print("Connector1-GFCI"); // Not implemented in AC001
              break;
            case 7: lcd.print("Connector1-Earth Disc");
              break;
            default: lcd.print("*****FAULTED 1*****"); // You can make spaces using well... spacesbreak;
          }
        }
      }
      break;
    case 2: if (disp_evse_B)
      {
        lcd.clear();
        lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
        if (notFaulty_B)
        {
          lcd.print("*****CHARGING 2*****"); // You can make spaces using well... spaces
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
        }
        else
        {
          switch (fault_code_B)
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

      }
      break;
    case 3: if (disp_evse_C)
      {
        lcd.clear();
        lcd.setCursor(0, 0); // Or setting the cursor in the desired position.
        if (notFaulty_C)
        {
          lcd.print("*****CHARGING 3*****"); // You can make spaces using well... spaces
          lcd.setCursor(0, 1); // Or setting the cursor in the desired position.
          lcd.print("VOLTAGE(v):");
          lcd.setCursor(12, 1); // Or setting the cursor in the desired position.
          lcd.print(String(instantVoltage_C));
          lcd.setCursor(0, 2);
          lcd.print("CURRENT(A):");
          lcd.setCursor(12, 2); // Or setting the cursor in the desired position.
          lcd.print(String(instantCurrrent_C));
          lcd.setCursor(0, 3);
          lcd.print("POWER(KW) :");
          lcd.setCursor(12, 3); // Or setting the cursor in the desired position.
          lcd.print(String(instantPower_C));
        }
        else
        {
          switch (fault_code_C)
          {
            case -1: break; //It means default.
            case 0: lcd.print("Connector3-Over Voltage");
              break;
            case 1: lcd.print("Connector3-Under Voltage");
              break;
            case 2: lcd.print("Connector3-Over Current");
              break;
            case 3: lcd.print("Connector3-Under Current");
              break;
            case 4: lcd.print("Connector3-Over Temp");
              break;
            case 5: lcd.print("Connector3-Over Temp");
              break;
            case 6: lcd.print("Connector3-GFCI"); // Not implemented in AC001
              break;
            case 7: lcd.print("Connector3-Earth Disc");
              break;
            default: lcd.print("*****FAULTED 3*****"); // You can make spaces using well... spacesbreak;
          }
        }



      }
      break;
    default: Serial.println(F("**Display default**"));
      break;
  }
}
#endif
