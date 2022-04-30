/*
 * urlparse.cpp
 * 
 * Copyright 2021 raja <raja@raja-Inspiron-N5110>
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


#include "urlparse.h"
#include <Arduino.h>
#include <Preferences.h>

extern Preferences preferences;
extern String host_m;
extern int port_m;
//extern String protocol_m;
extern String path_m;
void urlparser()
{
String ws_url_prefix_m = "";
String protocol_m = "";
char port[]="";
int portNo = 0;
char host[]="";
int protocol=0;
char pt[10];

bool portFound = false;

	  /************************Preferences***********************************************/
  preferences.begin("credentials", false);

  ws_url_prefix_m = preferences.getString("ws_url_prefix", ""); //characters
  if (ws_url_prefix_m.length() > 0) {
    Serial.println("Fetched WS URL success: " + String(ws_url_prefix_m));
  } else {
    Serial.println(F("Unable to Fetch WS URL / Empty"));
    return;
  }

	
	//Regex object creation.
  // match state object
  MatchState *ms; // It is always advisible to use pointer for objects as we can free memory later.

  ms = new MatchState();

  int n = ws_url_prefix_m.length();
 
    // declaring character array
    char requestedURL[n + 1];
 char hostName[n + 1];
char pagePartOfURL[n + 1];
char domain[n + 1];
char buf[n + 1];
    // copying the contents of the
    // string to char array
    strcpy(requestedURL, ws_url_prefix_m.c_str());

    Serial.print(F("Requested URL is:"));
    Serial.println(requestedURL);
  
   strcpy(buf, requestedURL);
      // Now, find the spot where the domain/server ends and the rest of the URL starts
      int splitLocation = 0;
      boolean foundSplit = false;
     int length = sizeof(requestedURL);
      // Go through the terminal input, one character at a time and check for a forward-slash
      // if found, make sure the character before or after is not a forward-slash also, because
      // in that case we are dealing with the http:// part of a url.
      // if we find a single forward-slash, remember the position where that is in the char[]
      for (int i=0; i < length; i++) {
        if (foundSplit != true && requestedURL[i] == '/') {
          if (requestedURL[i-1] != '/' && requestedURL[i+1] != '/') {
            foundSplit = true;
            splitLocation = i;
            if(URL_DEBUG) Serial.println(F("First split"));
          }
        }
      }
      
      // if foundSplit is true, then requestedURL can be split, if false, there is only a domain/server and no page
      // now store the domain/server part in hostName[]
      if (foundSplit == true) {
        for (int j=0; j < splitLocation; j++) {
          hostName[j] = requestedURL[j];
        }
        
        // and the rest in pagePartOfURL[]
        for (int k=splitLocation; k < length; k++) {
          pagePartOfURL[k-splitLocation] = requestedURL[k];
        }
        hostName[splitLocation] = '\0';
        pagePartOfURL[length-splitLocation] = '\0';
      }  

      if(URL_DEBUG) Serial.println(F("Extracting"));
      if(URL_DEBUG) Serial.println(hostName);
      if(URL_DEBUG) Serial.println(pagePartOfURL);
      if(URL_DEBUG) Serial.println(F("Extracted"));

      /*
       * Logic for extracting the port number and the protocol
       */

    ms->Target (buf);  // set its address
  //Serial.println (buf);

  char wsscheck = ms->Match ("wss://");
  char wscheck = ms->Match ("ws://");
  char httpcheck = ms->Match ("http://");
  char httpscheck = ms->Match ("https://");


  //Serial.println("Checking for protocol:");
  if(wsscheck > 0)
    {
      if(URL_DEBUG) Serial.print(F("protocol = "));
    //  strcpy(protocol,pwss);
    protocol = 0;
      Serial.println(protocol);
      ms->Target (domain);  // set its address
      ms->GlobalReplace ("wss://", "");
    if(URL_DEBUG) Serial.print("host = ");
    if(URL_DEBUG) Serial.println(domain);

     }
    else if(wscheck > 0)
    {
      if(URL_DEBUG) Serial.print(F("protocol = "));
      //char protocol[] = "ws";
      protocol = 1;
      //strcpy(protocol,pws);
      if(URL_DEBUG) Serial.println(protocol);
      ms->Target (domain);  // set its address
      ms->GlobalReplace ("ws://", "");
    if(URL_DEBUG) Serial.print("host = ");
    if(URL_DEBUG) Serial.println(domain);
    }
    else if(httpcheck > 0)
    {
      if(URL_DEBUG) Serial.print(F("protocol = "));
     // strcpy(protocol,phttp);
     protocol = 2;
      Serial.println(protocol);
      ms->Target (domain);  // set its address
      ms->GlobalReplace ("http://", "");
    if(URL_DEBUG) Serial.print("host = ");
    if(URL_DEBUG) Serial.println(domain);

    }
    else if (httpscheck > 0)
    {
      if(URL_DEBUG) Serial.print(F("protocol = "));
      protocol = 3;
      //strcpy(protocol,phttps);
      Serial.println(protocol);
      ms->Target (domain);  // set its address
      ms->GlobalReplace ("https://", "");
    if(URL_DEBUG) Serial.print("host = ");
    if(URL_DEBUG) Serial.println(domain);

    } 

/*
 * Logic for port number
 */
 ms->Target (buf);  // set its address
 char portCheck = ms->Match(":.(%d+)"); // first match of anything after : gives the port 
if (portCheck > 0)
    {
    strcpy(port,ms->GetMatch (buf));
    ms->Target(port);
    ms->GlobalReplace (":", "");
    portNo = atoi(port);
    if(URL_DEBUG) Serial.print("port = ");
    if(URL_DEBUG) Serial.println(port);
    portFound = true;
    }
  else
  {
    portFound = false;

    if(httpcheck >0)
    {
      strcpy(port,"80");
      portNo = 80;
    }
    else if(wscheck > 0)
    {
      strcpy(port,"80");
      portNo = 80;
    }
    else if (httpscheck >0 )
    {
      strcpy(port,"443");
      portNo = 443;
    }
    else if (wsscheck > 0 )
    {
      strcpy(port,"443");
      portNo = 443;
    }
    
    if(URL_DEBUG) Serial.print("port = ");
    if(URL_DEBUG) Serial.println(portNo);
  }

/************
 * logic for domain name
 */
foundSplit = false;
//bool portFound = true;
for (int i=0; i < length; i++) {
        if (foundSplit != true && requestedURL[i] == ':' && portFound == true) {
          if (requestedURL[i-1] != '/' && requestedURL[i+1] != '/') {
            foundSplit = true;
            splitLocation = i;
          }
        }

        else if (foundSplit != true && requestedURL[i] == '/' && portFound == false) {
          if (requestedURL[i-1] != '/' && requestedURL[i+1] != '/') {
            foundSplit = true;
            splitLocation = i;
          }
        }
      }

      if (foundSplit == true) {
        for (int j=0; j < splitLocation; j++) {
          domain[j] = requestedURL[j];
        }
        domain[splitLocation] = '\0';
        if(URL_DEBUG) Serial.print(F("Domain = "));
        if(URL_DEBUG) Serial.println(domain); 
      }





  /*
   * Values at the end of scraping
   */

Serial.println(F("Values at the end of scraping"));

switch (protocol)
{
  case 0:
          strcpy(pt,"wss");
          ms->Target (domain);  // set its address
          ms->GlobalReplace ("wss://", "");
          
          break;
  case 1:
          strcpy(pt,"ws");
          ms->Target (domain);  // set its address
          ms->GlobalReplace ("ws://", "");
          
          break;
  case 2:
          strcpy(pt,"http");
          ms->Target (domain);  // set its address
          ms->GlobalReplace ("http://", "");
          
          break;
  case 3:
          strcpy(pt,"https");
          ms->Target (domain);  // set its address
          ms->GlobalReplace ("https://", "");
          break;
  default:
          Serial.println(F("Undefined protocol"));
}

    Serial.print(F("port ="));
    Serial.println(portNo);
    Serial.print(F("domain ="));
    Serial.println(domain);
    Serial.print(F("protocol ="));
    Serial.println(pt);
    Serial.print(F("context root ="));
    Serial.println(pagePartOfURL);

  // port_m , host_m and protocol_m are strings.

    int b_size = sizeof(domain) / sizeof(char);
  
    //host_m = convertToString(domain, b_size);

    host_m = String (domain);

    b_size = sizeof(pagePartOfURL) / sizeof(char);

    //contextRoot_m = convertToString(pagePartOfURL, b_size);
    path_m = String (pagePartOfURL);

    b_size = sizeof(pt) / sizeof(char);

    //protocol_m = convertToString(pt, b_size);
    protocol_m = String(pt);

//    strcpy(protocol_m,"wss"); // temporarily

    
    port_m = portNo;


}
