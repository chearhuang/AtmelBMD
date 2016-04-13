#ifndef __BMD101__H_
#define __BMD101__H_

#include "stdio.h"
#include "usart.h"
typedef signed int INT;
typedef unsigned int UINT;
typedef unsigned short UCHAR;

#define Packet_Header    	0xAA

#define Payload_Length   	0x04

#define Poor_Signal      	0x02

#define Poor_Signal_Data  0xC8

#define Heart_Rate       	0x03


typedef struct NeuroSkyBMD
{
	UCHAR Rpayload_Length;
	UINT PayloadData[12];
	UINT HighData;
	UINT LowData;
	UINT RawData;
	UCHAR PoorSignal_Tag;
	UCHAR PoorSignal_Value;
	UCHAR HeartRate_Tag;
	UCHAR HeartRate_Value;
	UCHAR PacketCheckSum;
}BMD101;

#endif
