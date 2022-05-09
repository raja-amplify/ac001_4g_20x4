/*
 * dwin.cpp
 * 
 * Copyright 2022 raja <raja@raja-IdeaPad-Gaming-3-15IMH05>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */
#include "dwin.h"

//Object instantiation should always take place in the cpp file
SoftwareSerial dwin(27,26);

unsigned char Buffer[80]; 
unsigned char Buffer_Len = 0;
unsigned char tcount = 0;

extern int8_t button;


/* @brief: The follow is a lookup table
 * Table : This makes implementation faster
 */ 
//The below messages belong to status of page 0,1 and 3 : Address is 0X52,0X00
// status - available, not available, fault - name of fault
unsigned char avail[28] =           {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X41,0X56,0X41,0X49,0X4c,0X41,0X42,0X4c,0X45,0XFF,0XFF,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20};
unsigned char not_avail[28] =       {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X4e,0X4f,0X54,0X20,0X41,0X56,0X41,0X49,0X4c,0X41,0X42,0X4c,0X45,0XFF,0XFF,0X20,0X20,0X20,0X20,0X20,0X20,0X20};
unsigned char fault_emgy[28] =      {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X66,0X61,0X75,0X6c,0X74,0X20,0X65,0X6d,0X67,0X79,0XFF,0XFF,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20};
unsigned char fault_noearth[28] =   {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X66,0X61,0X75,0X6c,0X74,0X20,0X6e,0X6f,0X20,0X65,0X61,0X72,0X74,0X68,0XFF,0XFF,0X20,0X20,0X20,0X20,0X20,0X20};
unsigned char fault_overVolt[28] =  {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X66,0X61,0X75,0X6c,0X74,0X20,0X6f,0X76,0X65,0X72,0X20,0X76,0X6f,0X6c,0X74,0X61,0X67,0X65,0XFF,0XFF,0X20,0X20};
unsigned char fault_underVolt[28] = {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X66,0X61,0X75,0X6c,0X74,0X20,0X75,0X6e,0X64,0X65,0X72,0X20,0X76,0X6f,0X6c,0X74,0X61,0X67,0X65,0XFF,0XFF,0X20};
unsigned char fault_overTemp[28] =  {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X66,0X61,0X75,0X6c,0X74,0X20,0X6f,0X76,0X65,0X72,0X20,0X74,0X65,0X6d,0X70,0X65,0X72,0X61,0X74,0X75,0X72,0X65};
unsigned char fault_overCurr[28] =  {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X66,0X61,0X75,0X6c,0X74,0X20,0X6f,0X76,0X65,0X72,0X20,0X63,0X75,0X72,0X72,0X65,0X6e,0X74,0XFF,0XFF,0X20,0X20};
unsigned char fault_underCurr[28] = {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X66,0X61,0X75,0X6c,0X74,0X20,0X75,0X6e,0X64,0X65,0X72,0X20,0X63,0X75,0X72,0X72,0X65,0X6e,0X74,0XFF,0XFF,0X20}; 
unsigned char fault_suspEV[28]    = {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X53,0X55,0X53,0X50,0X45,0X4e,0X44,0X45,0X44,0X20,0X45,0X56,0XFF,0XFF,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20}; 
unsigned char fault_suspEVSE[28]  = {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X53,0X55,0X53,0X50,0X45,0X4e,0X44,0X45,0X44,0X20,0X45,0X56,0X53,0X45,0XFF,0XFF,0X20,0X20,0X20,0X20,0X20,0X20}; 


unsigned char charging[28]        = {0X5A, 0XA5, 0X19, 0X82,0X52,0X00,0X43,0X48,0X41,0X52,0X47,0X49,0X4e,0X47,0XFF,0XFF,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20}; 

unsigned char ct[22]  =  {0X5A, 0XA5, 0X13, 0X82,0X52,0X00,0X43,0X6F,0X6E,0X6E,0X65,0X63,0X74,0X65,0X64,0X20,0X20,0X20,0X20,0X20,0X20,0X20};//connected
unsigned char nct[22] =  {0X5A, 0XA5, 0X13, 0X82,0X52,0X00,0X6E,0X6F,0X74,0X20,0X43,0X6F,0X6E,0X6E,0X65,0X63,0X74,0X65,0X64,0X20,0X20,0X20};//not connected 
unsigned char et[22]  =  {0X5A, 0XA5, 0X13, 0X82,0X52,0X00,0X45,0X74,0X68,0X65,0X72,0X6E,0X65,0X74,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20};//ethernet
unsigned char wi[22]  =  {0X5A, 0XA5, 0X13, 0X82,0X45,0X00,0X57,0X69,0X66,0X69,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20};//wifi
unsigned char tr[22]  =  {0X5A, 0XA5, 0X13, 0X82,0X54,0X00,0X54,0X61,0X70,0X20,0X52,0X46,0X49,0X44,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20};//tap rfid
unsigned char utr[22] =  {0X5A, 0XA5, 0X13, 0X82,0X54,0X00,0X52,0X46,0X49,0X44,0X20,0X75,0X6E,0X61,0X76,0X61,0X69,0X6C,0X61,0X6C,0X65,0X20};//rfid unavailable
unsigned char g[22] =    {0X5A, 0XA5, 0X13, 0X82,0X45,0X00,0X34,0X47,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20};//4g

//on page 0 display tap rfid or scan qr - address 5400 

unsigned char tap_rfid[30] =        {0X5A, 0XA5, 0X1B, 0X82,0X54,0X00, 0X54,0X61,0X70,0X20,0X52,0X46,0X49,0X44,0X2f,0X53,0X63,0X61,0X6e,0X20,0X51,0X52,0X20,0X63,0X6f,0X64,0X65,0XFF,0XFF,0X20};
unsigned char clear_tap_rfid[30] =  {0X5A, 0XA5, 0X1B, 0X82,0X54,0X00, 0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0X20,0XFF,0XFF,0X20};



unsigned char clu[22]  =  {0X5A, 0XA5, 0X13, 0X82,0X45,0X00,0X43,0X6F,0X6E,0X6E,0X65,0X63,0X74,0X65,0X64,0XFF,0XFF,0X20,0X20,0X20,0X20,0X20};//connected
unsigned char clun[22] =  {0X5A, 0XA5, 0X13, 0X82,0X45,0X00,0X6E,0X6F,0X74,0X20,0X43,0X6F,0X6E,0X6E,0X65,0X63,0X74,0X65,0X64,0XFF,0XFF,0X20};//not connected 

unsigned char v1[8] = {0X5A, 0XA5, 0X05, 0X82, 0X43, 0X00, 0X00, 0X23};
unsigned char v2[8] = {0X5A, 0XA5, 0X05, 0X82, 0X31, 0X00, 0X00, 0X23};
unsigned char v3[8] = {0X5A, 0XA5, 0X05, 0X82, 0X19, 0X00, 0X00, 0X23};

unsigned char i1[8] = {0X5A, 0XA5, 0X05, 0X82, 0X40, 0X00, 0X00, 0X23};
unsigned char i2[8] = {0X5A, 0XA5, 0X05, 0X82, 0X28, 0X00, 0X00, 0X23};
unsigned char i3[8] = {0X5A, 0XA5, 0X05, 0X82, 0X90, 0X00, 0X00, 0X23};
unsigned char e1[8] = {0X5A, 0XA5, 0X05, 0X82, 0X37, 0X00, 0X00, 0X23};
unsigned char e2[8] = {0X5A, 0XA5, 0X05, 0X82, 0X25, 0X00, 0X00, 0X23};
unsigned char e3[8] = {0X5A, 0XA5, 0X05, 0X82, 0X13, 0X00, 0X00, 0X23};

unsigned char kwh[8]= {0X5A, 0XA5, 0X05, 0X82, 0X50, 0X00, 0X00, 0X23};

unsigned char HR[8]= {0X5A, 0XA5, 0X05, 0X82, 0X71, 0X00, 0X00, 0X23};
unsigned char MINS[8]= {0X5A, 0XA5, 0X05, 0X82, 0X72, 0X00, 0X00, 0X23};
unsigned char SEC[8]= {0X5A, 0XA5, 0X05, 0X82, 0X73, 0X00, 0X00, 0X23};


unsigned char change_page[10] = {0X5A, 0XA5, 0X07, 0X82, 0X00, 0X84, 0X5A, 0X01, 0X00, 0X00}; // LAST BYTE is the page number

/*
* @brief: DWIN_read 
* This function is used to read from a particular location
*/
int8_t DWIN_read()
{
if (dwin.available())
  {
    //
    Buffer[Buffer_Len] = dwin.read();
    Serial.println(Buffer[Buffer_Len],HEX);
    Buffer_Len++;
    tcount = 5;
    if(Buffer_Len > 10)
    {
     Buffer_Len = 0; 
    }
   return -3;
  }
  else
  {
    Buffer_Len = 0;
    //if (Buffer[0] == 0X5A && Buffer[1] == 0XA5 && Buffer[3]==0x06 &&Buffer[4]==0x83 &&Buffer[5]==0x59 &&Buffer[6]==0x00 && Buffer[7]==0x01 && Buffer[8]==0x00)
    //start 1
    if(Buffer[0] == 0X01 &&  Buffer[1] == 0X00 &&  Buffer[2] == 0X01)
    { 
      
      button = 1;
      //clear the buffer.
      for(int i=0;i<80;i++)
    {
      Buffer[i] = 0X00;
    }
     return button;
    }
    //start 2
     else if(Buffer[0] == 0X01 &&  Buffer[1] == 0X00 &&  Buffer[2] == 0X00)
    { 
      button = 2;
      for(int i=0;i<80;i++)
    {
      Buffer[i] = 0X00;
    }
     return button;
    }
    //start 3
    else if(Buffer[0] == 0XFF &&  Buffer[1] == 0X01 &&  Buffer[2] == 0X00 && Buffer[3] == 0X01)
    { 
      button = 3;
      for(int i=0;i<80;i++)
    {
      Buffer[i] = 0X00;
    }
     return button;
    }
    //stop 1
    else if(Buffer[0] == 0X00 &&  Buffer[1] == 0X01 &&  Buffer[2] == 0X00 &&  Buffer[3] == 0X01)
    { 
      
      button = 1;
      for(int i=0;i<80;i++)
    {
      Buffer[i] = 0X00;
    }
     return button;
    }
    //stop 2
     else if(Buffer[0] == 0X00 &&  Buffer[1] == 0X01 &&  Buffer[2] == 0X00  &&  Buffer[3] == 0X00)
    { 
      button = 2;
      for(int i=0;i<80;i++)
    {
      Buffer[i] = 0X00;
    }
     return button;
    }
    //stop 3
    else if(Buffer[0] == 0XFF &&  Buffer[1] == 0X00 &&  Buffer[2] == 0X01 && Buffer[3] == 0X00 && Buffer[4] == 0X01)
    { 
      button = 3;
      for(int i=0;i<80;i++)
    {
      Buffer[i] = 0X00;
    }
     return button;
    }
    else
    {
    return -1;
    }

    
    
  }


  
return -2;
}

/*
* @brief: DWIN_SET(unsigned char *) 
* This function is used to set voltage,current and energy.
*/

uint8_t DWIN_SET(unsigned char *val,size_t len)
{
//Serial.write(val, 8);
if(dwin.write(val,len))
{
return DISP_OK;
}
else
{
	return DISP_FAIL;
}
}

/*
* @brief : flush_dwin
* This function flushes the available serial data.
*/
void flush_dwin()
{
  //To flush 
  Serial.println(DWIN_read());
  Serial.println(DWIN_read());
  Serial.println(DWIN_read());
}

/*
 * @brief: begin 
 * 
*/

void dwin_setup()
{
	dwin.begin(115200);
}
