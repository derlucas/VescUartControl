/*
Copyright 2015 - 2017 Andreas Chaitidis Andreas.Chaitidis@gmail.com

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef _VESCUART_h
#define _VESCUART_h


#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WConstants.h"
#endif

#include "datatypes.h"

typedef void (*WriteCallback)  (const uint8_t *what, const size_t size);    // send a byte to serial port
typedef int  (*AvailableCallback)  ();    // return number of bytes available
typedef int  (*ReadCallback)  ();    // read a byte from serial port

int PackSendPayload(WriteCallback fSend,
                    uint8_t* payload, int lenPay);

int ReceiveUartMessage(AvailableCallback fAvailable,
                       ReadCallback fRead,
                       uint8_t* payloadReceived);

bool VescUartGetValue(WriteCallback fSend, AvailableCallback fAvailable,
                       ReadCallback fRead, struct bldcMeasure& values);


#endif

