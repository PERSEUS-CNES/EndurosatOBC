/*
	Test app to send data to a terminal monitoring a second serial port.

	To build use the following gcc statement 
	(assuming you have the d2xx library in the /usr/local/lib directory).
	gcc -o timeouts main.c -L. -lftd2xx -Wl,-rpath /usr/local/lib
*/

#include <stdio.h>
#include <assert.h>
#include "ftd2xx.h"
#include <string.h>
#include <unistd.h>
#include "es_crc32.h"
#include <stdint.h>
#include <stdlib.h>
#include "simpleEndurosat.h"

/*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* INTERNAL DEFINES
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/

#define SIZE_HEADER_MODULE			(0x505553450720)
#define SIZE_DATA_LENGHT			4
#define SIZE_DATA_LENGHT_0			0x0000
#define SIZE_COMMAND_STATUS			0x0000
#define SIZE_COMMAND_CREATE			0x0601
#define SIZE_COMMAND_WRITE			0x0701
#define SIZE_COMMAND_OPEN			0x0801
#define SIZE_COMMAND_READ			0x0901
#define SIZE_TYPE 					0x0000
#define SIZE_COMMAND_GET_RESULT 	0x1401
#define SIZE_TYPE_GET_RESULT_CREATE	0x0601
#define SIZE_TYPE_GET_RESULT_WRITE	0x0701
#define SIZE_TYPE_GET_RESULT_OPEN	0x0801
#define SIZE_TYPE_GET_RESULT_READ	0x0901
#define SIZE_HANDLE 				4
#define SIZE_CRC					4



#define ARRAY_SIZE(x) sizeof((x))/sizeof((x)[0])



void reverse(char *x, int begin, int end)
{
   char c;

   if (begin >= end)
      return;

   c          = *(x+begin);
   *(x+begin) = *(x+end);
   *(x+end)   = c;

   reverse(x, ++begin, --end);
}


void patternCut(DWORD* RxBytes, uint8_t RxBuffer[])
{
	int patternDetected = 0;
	int patternCount[16] = {0};
	int i = 16;
	
	uint8_t esupPattern [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20};
	while(i<*RxBytes)
	{
		if(memcmp(RxBuffer+i, esupPattern, sizeof(uint8_t)*6) == 0)
		{
			patternCount[patternDetected] = i;
			patternDetected++;
		}
		i+=16;
	}
	if(patternDetected > 0)
	{
		for(int i= 0; i < patternCount[0]; i++)
		{
			printf("%.2X",RxBuffer[i]);
		}
		printf("\n");
		if(patternDetected > 1)
		{
			for(int i = 0; i++;i < patternDetected)
			{
				for(int j= patternCount[i]; j < patternCount[i+1]; j++){
					printf("%.2X",RxBuffer[j]);
				}
			}
			printf("\n");
		}
		for(int i= patternCount[patternDetected-1]; i <= *RxBytes; i++)
		{
			printf("%.2X",RxBuffer[i]);
		}
	}
	else
	{
		for(int i=0;i<*RxBytes;i++){
			printf("%.2X",RxBuffer[i]);
		}
	}

}

void patternCutHandle(DWORD* RxBytes, uint8_t RxBuffer[], uint8_t Handle[])
{
	int patternDetected = 0;
	int patternCount[16] = {0};
	int i = 16;
	uint8_t esupPattern [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20};

	while(i<*RxBytes)
	{
		if(memcmp(RxBuffer+i, esupPattern, sizeof(uint8_t)*6) == 0)
		{
			patternCount[patternDetected] = i;
			patternDetected++;
		}
		i+=16;
	}
	if(patternDetected > 0)
	{
		for(int i= 0; i < patternCount[0]; i++)
		{
			printf("%.2X",RxBuffer[i]);
		}
		printf("\n");
		if(patternDetected > 1)
		{
			for(int i = 0; i++;i < patternDetected)
			{
				for(int j= patternCount[i]; j < patternCount[i+1]; j++){
					printf("%.2X",RxBuffer[j]);
						
				}
			}
			printf("\n");
		}
		for(int i= patternCount[patternDetected-1]; i <= *RxBytes; i++)
			
		{
			printf("%.2X",RxBuffer[i]);
		}
		printf("\n");
		for(int i= 0; i < 4; i++)
		{
			Handle[i] = RxBuffer[patternCount[patternDetected-1]+15+i];
			printf("%.2X",Handle[i]);
		}
		printf("\n");
	}
	else
	{
		for(int i=0;i<*RxBytes;i++){
			printf("%.2X ",RxBuffer[i]);
		}
	}

}


FT_STATUS initialize_FTDI(int baudRate, int portNum)
{
	FT_STATUS  ftStatus = FT_OK;
	if (baudRate < 0)
	{
		baudRate = 3000000;
	}	
	
	printf("Trying FTDI device %d at %d baud.\n", portNum, baudRate);
	
	ftStatus = FT_Open(portNum, &ftHandle);
	if (ftStatus != FT_OK) 
	{
		printf("FT_Open(%d) failed, with error %d.\n", portNum, (int)ftStatus);
		printf("Use lsmod to check if ftdi_sio (and usbserial) are present.\n");
		printf("If so, unload them using rmmod, as they conflict with ftd2xx.\n");
		goto exit;
	}

	assert(ftHandle != NULL);

	ftStatus = FT_ResetDevice(ftHandle);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_ResetDevice returned %d.\n", (int)ftStatus);
		goto exit;
	}
	
	ftStatus = FT_SetBaudRate(ftHandle, (ULONG)baudRate);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_SetBaudRate(%d) returned %d.\n", 
		       baudRate,
		       (int)ftStatus);
		goto exit;
	}
	// Paquets de 8 bits, 1 Stop bit , Pas de parité
	ftStatus = FT_SetDataCharacteristics(ftHandle, 
	                                     FT_BITS_8,
	                                     FT_STOP_BITS_1,
	                                     FT_PARITY_NONE);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_SetDataCharacteristics returned %d.\n", (int)ftStatus);
		goto exit;
	}
	                          
	// Indicate our presence to remote computer
	ftStatus = FT_SetDtr(ftHandle);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_SetDtr returned %d.\n", (int)ftStatus);
		goto exit;
	}

	// Flow control is needed for higher baud rates
	ftStatus = FT_SetFlowControl(ftHandle, FT_FLOW_NONE, 0, 0);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_SetFlowControl returned %d.\n", (int)ftStatus);
		goto exit;
	}

	// Assert Request-To-Send to prepare remote computer
	/*
	ftStatus = FT_SetRts(ftHandle);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_SetRts returned %d.\n", (int)ftStatus);
		goto exit;
	}*/

	
	ftStatus = FT_SetTimeouts(ftHandle, 0, 0);	// 3 seconds
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_SetTimeouts returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	exit:
		return ftStatus;
}

int main(int argc, char *argv[])
{
	printf("Hello guys and girls\n");
}
