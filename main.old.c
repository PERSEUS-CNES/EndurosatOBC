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

S_X_BAND_CMD_StackEntry stackEntry;

void incr(uint8_t *s)
{
	int i, tail, len;
	len = strlen(s);
	//"Log_"
	int index_begin = 4;
	
	/* find out how many digits need to be changed */
	for (tail = len - 1; tail >= index_begin && s[tail] == '9'; tail--);
	
	if (tail < index_begin) {
		/* special case: all 9s, string will grow */
		s[index_begin] = '1';
		for (i = index_begin + 1; i <= len; i++) s[i] = '0';
		s[len + 1] = '\0';
	} else { /* normal case; change tail to all 0, change prev digit by 1*/
		for (i = len - 1; i > tail; i--)
			s[i] = '0';
		s[tail] += 1;
	}
}

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
	// Safe shutdown 
	//455355502007 00 00 00 00 011300003FA1AA05
	
	//45535550200700 05 00 00 01130000228E1916
	
	
	uint8_t testMCP [] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00, 0x00, 0x00, 0x00, 0x00 };

	//---------------------------------------------------------------------------------------------------//
	//-- Ci-dessous un ensemble de commandes en "dure" pour tester differentes fonctions de l'emetteur --//
	uint8_t newTestCreateFile [] = {0x45, 0x53, 0x55 , 0x50 , 0x07, 0x20, 0x0E, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00,  0x00,  0x4E, 0x65, 0x77, 0x5F, 0x46, 0x69, 0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	uint8_t getresultCreate [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x06, 0x01,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	
	uint8_t deleteFile [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20, 0x0D, 0x00, 0x00, 0x00, 0x04, 0x01, 0x00, 0x00, 0x4E, 0x65, 0x77, 0x5F, 0x46, 0x69, 0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x00,0x00, 0x00, 0x00, 0x00, 0x00 };
	//uint8_t deleteAllFile [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20, 0x00, 0x00, 0x00, 0x00, 0x05, 0x01, 0x00, 0x00, 0x03, 0x48, 0xB6, 0x2A,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	
	uint8_t testWriteFile [] = {0x45, 0x53, 0x55 , 0x50 , 0x07, 0x20, 0x1A, 0x00, 0x00, 0x00 , 0x07, 0x01, 0x00, 0x00, 0x10, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0E, 0x0D, 0x0C, 0x0B, 0x0A, 0x09, 0x08, 0x07, 0x06, 0x05, 0x04, 0x03, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,0x00 } ;
	uint8_t getresultWrite [] = {0x45,0x53, 0x55, 0x50, 0x07, 0x20, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x07, 0x01,0x00, 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 ,0x00, 0x00, 0x00, 0x00 }; 

	uint8_t testOpenFile [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20, 0x0D, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00,  0x4E, 0x65, 0x77, 0x5F, 0x46, 0x69, 0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x00,0x00, 0x00, 0x00, 0x00, 0x00 };	
	uint8_t getresultOpen [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x08, 0x01, 0x00, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00  };
	
	uint8_t testReadFile [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20, 0x04, 0x00, 0x00, 0x00, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  };
	uint8_t getresultRead [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x09, 0x01, 0x00, 0x00, 0x00, 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

	uint8_t dirTest [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20, 0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0xBA, 0x70, 0x61, 0xB7,0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t getDirTest [] = {0x45, 0x53, 0x55, 0x50, 0x07, 0x20, 0x00, 0x00, 0x00, 0x00, 0x14, 0x01, 0x02, 0x01, 0xED, 0x2A, 0x22, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	//---------------------------------------------------------------------------------------------------//	
	int        retCode = -1; // Assume failure
	int        f = 0;
	FT_STATUS  ftStatus = FT_OK;
	int        portNum = 0 ; // Deliberately invalid
	DWORD      bytesToWrite = 0;
	DWORD      bytesWritten = 0;
	int 	   crc_value = 0;
	int        inputRate = -1; // Entered on command line
	int        baudRate = -1; // Rate to actually use
	int        rates[] = {50, 75, 110, 134, 150, 200, 
	                      300, 600, 1200, 1800, 2400, 4800, 
	                      9600, 19200, 38400, 57600, 115200, 
	                      230400, 460800, 576000, 921600,3000000};
	DWORD TxBytes;
	DWORD EventDWord;
	DWORD RxBytes = 32;
	DWORD BytesReceived;
	int count;
	uint8_t RxBuffer[256];
	if (argc > 1)
	{
		sscanf(argv[1], "%d", &portNum);
	}
	
	if (portNum < 0)
	{
		// Missing, or invalid.  Just use first port.
		portNum = 0;
	}
	
	if (portNum > 16)
	{
		// User probably specified a baud rate without a port number
		printf("Syntax: %s [port number] [baud rate]\n", argv[0]);
		portNum = 0;
	}
	
	if (argc > 2)
	{
		sscanf(argv[2], "%d", &inputRate);

		for (f = 0; f < (int)(ARRAY_SIZE(rates)); f++)
		{
			if (inputRate == rates[f])
			{
				// User entered a rate we support, so we'll use it.
				baudRate = inputRate;
				break;
			}
		}
	}

	ftStatus = initialize_FTDI(baudRate, portNum);
	
	if(ftStatus != FT_OK)
	{
		printf("Unable to initialize FTDI. Software termination.\n");
		goto exit;
	}
	
	//---------------------------------------------------------------------------------------------------//
	//-- Supression préalable du fichier, pour éviter des erreurs lors de la création à venir --//
	uint32_t crc=0;
	/*
	crc=crc32(0,deleteFile,27);
	printf("\n  crc value  %.4X \n",crc);
	memcpy(deleteFile+27,&crc,sizeof(uint32_t));
	
	printf("\nSending Command df : 0x");
	printf("1\n");
	for(int i = 0; i < (int)sizeof(deleteFile); i++){
		printf("%.2X",deleteFile[i]);
	}*/
	
	/*purgeBuffer();
	ftStatus = FT_Write(ftHandle, 
	                    newTestCreateFile,
	                    bytesToWrite, 
	                    &bytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten, 
		       (int)bytesToWrite);
		goto exit;
	// }*/
	
	//---------------------------------------------------------------------------------------------------//		
	
	//-----------------------------------------------------------------------------------------------------------//
	//Essai de création d'un fichier en mode "intelligent" (via machine à état et appels de fonction haut-niveau)//
	bytesToWrite=32;
	RxBytes=32;
	uint8_t retStat = 0x00;
	//uint8_t fileName [] = {0x4E, 0x65, 0x77, 0x5F, 0x46, 0x69, 0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x00};
	static uint8_t fileName [30] = {'L','o','g','_','0','\0'};
	
	memset (&stackEntry, 0x00, sizeof (S_X_BAND_CMD_StackEntry));

    stackEntry.devID = S_BAND_TRNSM_DEFAULT_DEV_ID;
    stackEntry.ESTTC_request = 0;
    stackEntry.isFileCommand = 1;
    stackEntry.ESTTC_request_type = 'W';
    stackEntry.console = NULL; // X_BAND_TRNSM_CMD_ESTTC_console;
    stackEntry.retries = S_X_BAND_TRNSM_CMD_RETRY;
    stackEntry.timestamp = 0; //xTaskGetTickCount(); // Not eligible for microseconds....
    stackEntry.minDelay = S_X_BAND_STACK_DELAY;
    stackEntry.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
	stackEntry.retries = 60;
    stackEntry.type = S_X_BAND_TRNSM_NULL_TYPE;
    stackEntry.expectSlaveCommand = 1;

	stackEntry.command = S_X_BAND_TRNSM_CMD_DELL_ALL_F;
	do
	{
		retStat = S_X_BAND_TRNSM_DelFile(&stackEntry);
	} while (stackEntry.state != S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED);

	if(!retStat)
	{
		printf ("Success erase %d (retry %d)\n", retStat, stackEntry.retries);
	}
	else
	{
		printf ("\r\n S_X_BAND_TRNSM_UploadToXSBand_State_CREATEFILE ERROR = %d (retry %d)\n", retStat, stackEntry.retries);
		exit(0);
	}
	
	stackEntry.fileSize = 10;
	stackEntry.fileHandler = -1;
	stackEntry.retries = 60;
	stackEntry.command = S_X_BAND_TRNSM_CMD_CREATE_F;
    stackEntry.type = S_X_BAND_TRNSM_NULL_TYPE;
	stackEntry.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
	do
	{
		stackEntry.ESTTC_data_size = strlen(fileName) + 1;
		memcpy (stackEntry.ESTTC_data, fileName, stackEntry.ESTTC_data_size);
		retStat = S_X_BAND_TRNSM_CreateFile (&stackEntry);
		retStat = S_X_BAND_TRNSM_CreateFile (&stackEntry);
		incr(fileName);
		printf("i\n");
	} while(stackEntry.fileHandler == -1 && retStat != S_X_BAND_TRNSM_STAT_COMM_ERR);

	printf("stackEntry.fileHandler != -1 : %i\n", stackEntry.fileHandler != -1);
	printf("retStat != S_X_BAND_TRNSM_STAT_COMM_ERR : %i\n", retStat != S_X_BAND_TRNSM_STAT_COMM_ERR);
	printf("retStat : %i\n", retStat );
	
	printf("\n");
	for(int i = 0; i < strlen(fileName) + 1; i++){
		printf("%.2X",fileName[i]);
	}
	printf("\n");		

	if(!retStat && stackEntry.fileHandler != -1)
	{
        stackEntry.retries = S_X_BAND_TRNSM_CMD_RETRY;
        stackEntry.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
        memset (&S_X_BAND_TRNSM_FileDataBuffer, 0x00, sizeof (S_X_BAND_TRNSM_FileDataBuffer));
        S_X_BAND_TRNSM_FileDataBuffer.FileHandler = stackEntry.fileHandler;
        S_X_BAND_TRNSM_UploadToXSBand_FPos = 0;	
	}
	else
	{
		printf ("\r\n S_X_BAND_TRNSM_UploadToXSBand_State_CREATEFILE ERROR = %d (retry %d)\n", retStat, stackEntry.retries);
		exit(0);
	}
	S_X_BAND_TRNSM_FileDataBuffer.Size = 10;
	for(int i = 0; i < S_X_BAND_TRNSM_FileDataBuffer.Size; i++)
	{
		S_X_BAND_TRNSM_FileDataBuffer.Data[i] = i + 1;	
	}
	do
	{
		retStat = S_X_BAND_TRNSM_WriteFile(&stackEntry);
	} while (stackEntry.state != S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED);

	if(!retStat)
	{
		printf ("Success %d (retry %d)\n", retStat, stackEntry.retries);
		exit(0);
	}
	else
	{
		printf ("\r\n S_X_BAND_TRNSM_UploadToXSBand_State_CREATEFILE ERROR = %d (retry %d)\n", retStat, stackEntry.retries);
		exit(0);
	}
	stackEntry.command = S_X_BAND_TRNSM_CMD_DIR_F;
    stackEntry.retries = S_X_BAND_TRNSM_CMD_RETRY;
    stackEntry.state = S_X_BAND_TRNSM_CMD_STATE_CMD;
	stackEntry.type = S_X_BAND_TRNSM_NULL_TYPE;
    stackEntry.minDelay = 250;
	exit(0);
	
	//!!!!!!!!! Essai d'ouverture du fichier créé, suivi d'une écriture puis lecture pour vérification !!!!!!!!!//
	
	// retStat = S_X_BAND_TRNSM_OpenFile (&stackEntry);
	
		  
	// retStat = S_X_BAND_TRNSM_OpenFile (&stackEntry);
	// if (!retStat && stackEntry.state == S_X_BAND_TRNSM_CMD_STATE_CMD_FINISHED) {
		 
	
	//memcpy (&(stackEntry.ESTTC_data[stackEntry.ESTTC_data_size]), &stackEntry.fileSize, sizeof (stackEntry.fileSize));
    //stackEntry.ESTTC_data_size += sizeof (stackEntry.fileSize);
	// printf("40000\n");
    //retStat = S_X_BAND_TRNSM_SendCMD (stackEntry.command, stackEntry.type, stackEntry.ESTTC_data, stackEntry.ESTTC_data_size);
	
	
	//retStat = S_X_BAND_TRNSM_GetResult (stackEntry.command, stackEntry.type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
	//stackEntry.state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
	
	
	// stackEntry.command = S_X_BAND_TRNSM_CMD_WRITE_F;
	// retStat = S_X_BAND_TRNSM_SendCMD (stackEntry.devID, stackEntry.command, stackEntry.type, stackEntry.ESTTC_data, stackEntry.ESTTC_data_size);
	
	
	// retStat = S_X_BAND_TRNSM_GetResult (stackEntry->devID, stackEntry->command, stackEntry->type, S_X_BAND_TRNSM_Result_Rx_Buffer, &RxDataLenght);
	// stackEntry->state = S_X_BAND_TRNSM_CMD_STATE_CMD_GETRESULT_RES;
	
	//read
	// stackEntry->command = S_X_BAND_TRNSM_CMD_READ_F;
	// retStat = S_X_BAND_TRNSM_SendCMD (XSBandStack_Slave.devID, S_X_BAND_TRNSM_CMD_READ_F, S_X_BAND_TRNSM_NULL_TYPE, (uint8_t *)&XSBandStack_Slave.fileHandler, sizeof (XSBandStack_Slave.fileHandler));
	
	printf("5\n");
	lenghtQueue(&RxBytes);
	printf("Bytes red  : %i\n", RxBytes);
	ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);  
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			
			printf("Bytes red 1 : %i\n", RxBytes);
			patternCut(&RxBytes,RxBuffer);
		}
	}
		
	usleep(100);
	RxBytes=32;
	
	printf("Bytes red av : %i\n", RxBytes);
	
	lenghtQueue(&RxBytes);
	
	printf("Bytes red ap : %i\n", RxBytes);
	
	printf("Bytes red ccc  : %i\n", RxBytes);
	ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);  
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			printf("Bytes red 2 : %i\n", RxBytes);
			patternCut(&RxBytes,RxBuffer);
		}
	}
	
	usleep(100);
	//---------------------------------------------------------------------------------------------------//	
	
	
	//---------------------------------------------------------------------------------------------------//
	//-- Un ensemble d'exemples de mise en oeuvre de commandes en "dure" pour tester differentes fonctions de l'emetteur  --//
	
	crc=crc32(0,newTestCreateFile,28);
	printf("\n  crc value  %.4X \n",crc);
	memcpy(newTestCreateFile+28,&crc,sizeof(uint32_t));
	
	printf("\nSending Command 1tcf : 0x");
	
	for(int i = 0; i < (int)sizeof(newTestCreateFile); i++){
		printf("%.2X",newTestCreateFile[i]);
	}
	
	printf("\n");
	
	bytesToWrite=48;
	RxBytes=48;
	
	purgeBuffer();
	ftStatus = FT_Write(ftHandle, 
	                    newTestCreateFile,
	                    bytesToWrite, 
	                    &bytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten, 
		       (int)bytesToWrite);
		goto exit;
	}
	

 
	printf("Successfully wrote %d bytes\n", (int)bytesWritten);
 
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			printf("Bytes red : %i\n", RxBytes);
			patternCut(&RxBytes,RxBuffer);
		}
	}
	
	
	usleep(100);
	RxBytes=48;
	lenghtQueue(&RxBytes);
	ftStatus = 	FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			printf("\nBytes red : %i \n", RxBytes);
			patternCut(&RxBytes,RxBuffer);
		}
		
	}
	usleep(2000);
	
	
	
	//memcpy(testMCP,&SIZE_HEADER_MODULE,sizeof(uint8_t)*6);
	//memcpy(testMCP+6,&SIZE_DATA_LENGHT_0,sizeof(uint16_t));
	//memcpy(testMCP+8,&SIZE_COMMAND_STATUS,sizeof(uint16_t);
	//memcpy(testMCP+10,&SIZE_COMMAND_GET_RESULT,sizeof(uint16_t));
	//memcpy(testMCP+12,&SIZE_TYPE_GET_RESULT_CREATE,sizeof(uint16_t));
	//crc=crc32(0,getresultCreate,14);
	//printf("\n  crc value  %.4X \n",crc);
	//memcpy(getresultCreate+14,&crc,sizeof(uint32_t));
	//printf("Command GetResult Create %.2X",testMCP);
	
	
	
	printf("\nSending Command gt : 0x");
	for(int i=0;i<31;i++){
		printf("%.2X",getresultCreate[i]);
	}
	printf("\n");

	
	purgeBuffer();
	bytesToWrite=32;
	ftStatus = FT_Write(ftHandle, 
							getresultCreate,
							bytesToWrite, 
							&bytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten,
		       (int)bytesToWrite);
		goto exit;
	}
	printf("Successfully wrote %d bytes\n", (int)bytesWritten);

	usleep(2000);
	lenghtQueue(&RxBytes);
	
	uint8_t Handle[] = {0x00, 0x00, 0x00, 0x00};
	
	if (RxBytes) {
	
		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
				// FT_Read OK
			printf("\nBytes red gt : %i\n", RxBytes);
			patternCutHandle(&RxBytes,RxBuffer,&Handle);
			}
		}		
	}
	
	usleep(100);
	RxBytes=32;
	
	lenghtQueue(&RxBytes);
	
	if (RxBytes) {
	
		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
					// FT_Read OK
			printf("\nBytes red gt 2 : %i\n", RxBytes);
			patternCutHandle(&RxBytes,RxBuffer,&Handle);
			}
		}
	
	}
	
	usleep(2000);	
	
	
	for(int i = 0; i < (int)sizeof(testWriteFile); i++){
		printf("%.2X",testWriteFile[i]);
	}
	printf("\n");
	memcpy(testWriteFile+16,&Handle,sizeof(uint32_t));
	crc=crc32(0,testReadFile,40);
	printf("\n  crc value  %.4X \n",crc);
	memcpy(testWriteFile+40,&crc,sizeof(uint32_t));
	

	printf("\nSending Command wf : 0x");
	for(int i = 0; i < (int)sizeof(testWriteFile); i++){
		printf("%.2X",testWriteFile[i]);
	}
	printf("\n");
	
	bytesToWrite=48;
	RxBytes=48;
	lenghtQueue(&RxBytes);
	
	crc=crc32(0,testWriteFile,40);
	printf("\n  crc value  %.4X \n",crc);
	
	memcpy(testWriteFile+40,&crc,sizeof(uint32_t));
	
	for(int i = 0; i < (int)sizeof(testWriteFile); i++){
		printf("%.2X",testWriteFile[i]);
	}
	printf("\n");
	
	ftStatus = FT_Write(ftHandle, 
						testWriteFile,
						bytesToWrite, 
						&bytesWritten);
	
	if (ftStatus != FT_OK) {
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten,
		       (int)bytesToWrite);
		goto exit;
	}
	printf("Successfully wrote %d bytes\n", (int)bytesWritten);

	usleep(2000);
	
	lenghtQueue(&RxBytes);
	
	ftStatus = FT_GetStatus(ftHandle,&RxBytes,&TxBytes,&EventDWord);
	if (RxBytes) {
	
		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
				// FT_Read OK
				printf("\nBytes red : %i\n", RxBytes);
				patternCut(&RxBytes,RxBuffer);
			}
		}
	}
	usleep(2000);
	lenghtQueue(&RxBytes);
	ftStatus = FT_GetStatus(ftHandle,&RxBytes,&TxBytes,&EventDWord);
	if (RxBytes) {
	
	RxBytes=48;

		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
					// FT_Read OK
				printf("\nBytes red : %i\n", RxBytes);
				patternCut(&RxBytes,RxBuffer);
			}
		}
	
	}
	//memcpy(getresultWrite,&SIZE_PARTERN_GETRESULT_WRITE,sizeof(RxBytes));
	crc=crc32(0,getresultWrite,14);
	printf("\n  crc value  %.4X \n",crc);
	memcpy(getresultWrite+14,&crc,sizeof(uint32_t));
	
		
	printf("\nSending Command gwf : 0x");
	for(int i = 0; i < (int)sizeof(getresultWrite); i++){
		printf("%.2X",getresultWrite[i]);
	}
	printf("\n");

	purgeBuffer();
	bytesToWrite=32;
	ftStatus = FT_Write(ftHandle, 
							getresultWrite,
							bytesToWrite, 
							&bytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten,
		       (int)bytesToWrite);
		goto exit;
	}
	printf("Successfully wrote %d bytes\n", (int)bytesWritten);

	usleep(2000);
	lenghtQueue(&RxBytes);
	
	uint8_t HandleOR[] = {0x00, 0x00, 0x00, 0x00};
	
	if (RxBytes) {
	
		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
				// FT_Read OK
			printf("\nBytes red gt : %i\n", RxBytes);
			}
		}		
	}
	
	usleep(100);
	RxBytes=32;
	
	lenghtQueue(&RxBytes);
	
	if (RxBytes) {
	
		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
					// FT_Read OK
			printf("\nBytes red gt 2 : %i\n", RxBytes);
			}
		}
	
	}
	
	usleep(2000);	
	
	
	
	crc=crc32(0,testOpenFile,27);
	printf("\n  crc value  %.4X \n",crc);
	memcpy(testOpenFile+27,&crc,sizeof(uint32_t));
	
	
	
	
	printf("\nSending Command of : 0x");
	for(int i = 0; i < (int)sizeof(testOpenFile); i++){
		printf("%.2X",testOpenFile[i]);
	}
	printf("\n");

	bytesToWrite=32;
	RxBytes=32;
	lenghtQueue(&RxBytes);
	
	ftStatus = FT_Write(ftHandle, 
	                    testOpenFile,
	                    bytesToWrite, 
	                    &bytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten,
		       (int)bytesToWrite);
		goto exit;
	}
 


	printf("Successfully wrote %d bytes\n", (int)bytesWritten);

	lenghtQueue(&RxBytes);

	ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
				// FT_Read OK
			printf("\nBytes red : %i\n", RxBytes);
			patternCut(&RxBytes,RxBuffer);
			}
		}
	
	
	// Nous mettons un petit délais pour clarifier la communication 
	
	usleep(2000);	
	
	lenghtQueue(&RxBytes);

	ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
				// FT_Read OK
			printf("\nBytes red : %i\n", RxBytes);
			patternCut(&RxBytes,RxBuffer);
		}
	}
	
	
	usleep(2000);
	
	//memcpy(getresultOpen,&SIZE_PARTERN_GETRESULT_OPEN,sizeof(RxBytes));
	crc=crc32(0,getresultOpen,14);
	printf("\n  crc value  %.4X \n",crc);
	memcpy(getresultOpen+14,&crc,sizeof(uint32_t));
	
	
	printf("\nSending Command gropf : 0x");
	for(int i = 0; i < (int)sizeof(getresultOpen); i++){
		printf("%.2X",getresultOpen[i]);
	}
	printf("\n");

	bytesToWrite=32;
	RxBytes=32;
	lenghtQueue(&RxBytes);;
	

	
	purgeBuffer();
	ftStatus = FT_Write(ftHandle, 
	                    getresultOpen,
	                    bytesToWrite, 
	                    &bytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten, 
		       (int)bytesToWrite);
		goto exit;
	}
 
	printf("Successfully wrote %d bytes\n", (int)bytesWritten);
 
	lenghtQueue(&RxBytes);
	printf("Bytes red : %i\n", RxBytes);
	ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);  
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			printf("Bytes red : %i\n", RxBytes);
			patternCutHandle(&RxBytes,RxBuffer,&Handle);
		}
	}
	
	
	usleep(100);
	
	
	RxBytes=32;
	lenghtQueue(&RxBytes);
	ftStatus = 	FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			printf("\nBytes red : %i \n", RxBytes);
			patternCutHandle(&RxBytes,RxBuffer,&Handle); 
		}
		
	}
	usleep(2000);
	
	memcpy(testReadFile+14,&Handle,sizeof(uint32_t));
	crc=crc32(0,testReadFile,18);
	printf("\n  crc value  %.4X \n",crc);
	memcpy(testReadFile+18,&crc,sizeof(uint32_t));
	
	
	printf("\nSending Command rf : 0x");
	for(int i = 0; i < (int)sizeof(testReadFile); i++){
		printf("%.2X",testReadFile[i]);
	}
	printf("\n");
	
	bytesToWrite=32;
	RxBytes=32;
	lenghtQueue(&RxBytes);
	
	ftStatus = FT_Write(ftHandle, 
	                    testReadFile,
	                    bytesToWrite, 
	                    &bytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten,
		       (int)bytesToWrite);
		goto exit;
	}
 


	printf("Successfully wrote %d bytes\n", (int)bytesWritten);
	
	lenghtQueue(&RxBytes);
 
	ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
				printf("\nBytes red : %i\n", RxBytes);
				patternCut(&RxBytes,RxBuffer);
			}
		
		}
	

	
	usleep(2000);	
	
	lenghtQueue(&RxBytes);
	
	ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
				printf("\nBytes red : %i\n", RxBytes);
				//patternCut(&RxBytes,RxBuffer);
			}
		
		}
	
	usleep(2000);
	
	//memcpy(getresultRead,&SIZE_PARTERN_GETRESULT_READ,sizeof(RxBytes));
	crc=crc32(0,getresultRead,14);
	printf("\n  crc value  %.4X \n",crc);
	memcpy(getresultRead+14,&crc,sizeof(uint32_t));
	
	printf("\nSending Command grf	: 0x");
	for(int i=0;i<31;i++){
		printf("%.2X",getresultRead[i]);
	}
	printf("\n");

	bytesToWrite=32;
	RxBytes=32;
	lenghtQueue(&RxBytes);
	
	purgeBuffer();
	ftStatus = FT_Write(ftHandle, 
	                    getresultRead,
	                    bytesToWrite, 
	                    &bytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		goto exit;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten, 
		       (int)bytesToWrite);
		goto exit;
	}
 
	printf("Successfully wrote %d bytes\n", (int)bytesWritten);
 
	lenghtQueue(&RxBytes);
	printf("Bytes red : %i\n", RxBytes);
	ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);  
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			printf("Bytes red : %i\n", RxBytes);
			patternCut(&RxBytes,RxBuffer);
		}
	}
	
	
	usleep(100);
	
	
	RxBytes=48;
	lenghtQueue(&RxBytes);
	ftStatus = 	FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			printf("\nBytes red : %i \n", RxBytes);
			patternCut(&RxBytes,RxBuffer);
		}
		
	}
	usleep(2000);
	//---------------------------------------------------------------------------------------------------//	
	
	printf("Fin");
 	
	exit:
		if (ftHandle != NULL)
			FT_Close(ftHandle);
	
		return retCode;
}
