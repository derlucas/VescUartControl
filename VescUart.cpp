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

#include "VescUart.h"
#include "buffer.h"
#include "crc.h"

bool UnpackPayload(uint8_t* message, int lenMes, uint8_t* payload, int lenPa);
bool ProcessReadPacket(uint8_t* message, bldcMeasure& values, int len);

int ReceiveUartMessage(AvailableCallback fAvailable,
                       ReadCallback fRead, uint8_t* payloadReceived) {

	//Messages <= 255 start with 2. 2nd byte is length
	//Messages >255 start with 3. 2nd and 3rd byte is length combined with 1st >>8 and then &0xFF

	int counter = 0;
	int endMessage = 256;
	bool messageRead = false;
	uint8_t messageReceived[256];
	int lenPayload = 0;

	while (fAvailable()) {

		messageReceived[counter++] = fRead();

		if (counter == 2) {//case if state of 'counter' with last read 1

			switch (messageReceived[0])
			{
			case 2:
				endMessage = messageReceived[1] + 5; //Payload size + 2 for sice + 3 for SRC and End.
				lenPayload = messageReceived[1];
				break;
			case 3:
				//ToDo: Add Message Handling > 255 (starting with 3)
				break;
			default:
				break;
			}

		}
		if (counter >= sizeof(messageReceived))
		{
			break;
		}

		if (counter == endMessage && messageReceived[endMessage - 1] == 3) {//+1: Because of counter++ state of 'counter' with last read = "endMessage"
			messageReceived[endMessage] = 0;	
			messageRead = true;
			break; //Exit if end of message is reached, even if there is still more data in buffer. 
		}
	}
	bool unpacked = false;
	if (messageRead) {
		unpacked = UnpackPayload(messageReceived, endMessage, payloadReceived, messageReceived[1]);
	}
	if (unpacked)
	{
		return lenPayload; //Message was read

	}
	else {
		return 0; //No Message Read
	}
}

bool UnpackPayload(uint8_t* message, int lenMes, uint8_t* payload, int lenPay) {
	uint16_t crcMessage = 0;
	uint16_t crcPayload = 0;
	//Rebuild src:
	crcMessage = message[lenMes - 3] << 8;
	crcMessage &= 0xFF00;
	crcMessage += message[lenMes - 2];
    
	//Extract payload:
	memcpy(payload, &message[2], message[1]);

	crcPayload = crc16(payload, message[1]);
	if (crcPayload == crcMessage) {
		return true;
	}
	else {
		return false;
	}
}

int PackSendPayload(WriteCallback fSend, uint8_t* payload, int lenPay) {
	uint16_t crcPayload = crc16(payload, lenPay);
	int count = 0;
	uint8_t messageSend[256];   // possible buffer overflow here

	if (lenPay <= 256) {
		messageSend[count++] = 2;
		messageSend[count++] = lenPay;
	}
	else {
		messageSend[count++] = 3;
		messageSend[count++] = (uint8_t)(lenPay >> 8);
		messageSend[count++] = (uint8_t)(lenPay & 0xFF);
	}
	memcpy(&messageSend[count], payload, lenPay); // possible buffer overflow here

	count += lenPay;
	messageSend[count++] = (uint8_t)(crcPayload >> 8);
	messageSend[count++] = (uint8_t)(crcPayload & 0xFF);
	messageSend[count++] = 3;
	messageSend[count] = NULL;

	//Sending package
	fSend(messageSend, count);

	//Returns number of send bytes
	return count;
}


bool ProcessReadPacket(uint8_t* message, bldcMeasure& values, int len) {
	COMM_PACKET_ID packetId;
	int32_t ind = 0;

	packetId = (COMM_PACKET_ID)message[0];
	message++;  // skip the message id byte
	len--;

	switch (packetId) {
        case COMM_GET_VALUES:
            ind = 14; // skip the first 14 bytes
            values.avgMotorCurrent = buffer_get_float32(message, 100.0, &ind);
            values.avgInputCurrent = buffer_get_float32(message, 100.0, &ind);
            values.dutyCycleNow = buffer_get_float16(message, 1000.0, &ind);
            values.rpm = buffer_get_int32(message, &ind);
            values.inpVoltage = buffer_get_float16(message, 10.0, &ind);
            values.ampHours = buffer_get_float32(message, 10000.0, &ind);
            values.ampHoursCharged = buffer_get_float32(message, 10000.0, &ind);
            ind += 8; // skip 9 bytes
            values.tachometer = buffer_get_int32(message, &ind);
            values.tachometerAbs = buffer_get_int32(message, &ind);
            return true;
            break;
        default:
            return false;
            break;
	}
}


bool VescUartGetValue(WriteCallback fSend, AvailableCallback fAvailable,
                      ReadCallback fRead, bldcMeasure& values) {
	uint8_t command[1] = { COMM_GET_VALUES };
	uint8_t payload[256];
	PackSendPayload(fSend, command, 1);
	
    delay(10); // let the VESC some time to answer
    
	int lenPayload = ReceiveUartMessage(fAvailable, fRead, payload);
	
    if (lenPayload > 0) {
		return ProcessReadPacket(payload, values, lenPayload);
	}
	else {
		return false;
	}
}

