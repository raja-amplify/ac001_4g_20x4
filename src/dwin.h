/*
 * dwin.h
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
#ifndef DWIN_H
#define DWIN_H

#include <Arduino.h>
#include <SoftwareSerial.h>


/*
* @brief: dwinstatus
* This feature will avoid hardcoding of messages. 
*/
typedef enum dwinstatus { DISP_FAIL, DISP_OK };

static const char *dwinstatus_str[] = { "DISP_FAIL", "DISP_OK"};

typedef enum dwinREADstatus { DISP_NO_DAT, DISP_DAT_AVAIL, DISP_DAT_READ };

static const char *dwinREADstatus_str[] = { "DISP_NO_DAT", "DISP_DAT_AVAIL", "DISP_DAT_READ"};




int8_t DWIN_read();
uint8_t DWIN_SET(unsigned char *,size_t);
void dwin_setup();
void flush_dwin();

#endif
