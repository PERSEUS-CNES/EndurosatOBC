#include "ftdi_manager.h"

FT_HANDLE  ftHandle = NULL;										// Gestion des périphériques USB (lib ftd2xx)

DWORD      bytesToWrite = 0;
DWORD      bytesWritten = 0;
DWORD RxBytes = 32;

void purgeBuffer(FT_HANDLE* ftHandle)
{
	FT_STATUS  ftStatus;
	ftStatus = FT_Purge(*ftHandle, FT_PURGE_RX | FT_PURGE_TX); // Purge both Rx and Tx buffers
	if(ftStatus == FT_OK)
	{
		printf("FT_Purge OK\n");
	}
	else
	{
		printf("FT_Purge failed\n");
	}
}

void lenghtQueue(FT_HANDLE* ftHandle, DWORD* RxBytes)
{
	int count;
	usleep(100);
	count = 200000;
	*RxBytes=0;	
	while(count > 0)
	{
		usleep(100);	
		FT_GetQueueStatus(*ftHandle, RxBytes);
		//printf("\n Queue status 22 : %i\n", *RxBytes);
		if(*RxBytes !=0)
		{
			break;
		}
		count--;
	}
}

void initialize_FTDI(){

	FT_STATUS  ftStatus;
	printf("Trying FTDI device %d at %d baud.\n", 0, 115200);
	ftStatus = FT_Open(0, &ftHandle);
	if (ftStatus != FT_OK) 
	{
		printf("FT_Open(%d) failed, with error %d.\n", 0, (int)ftStatus);
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
	
	ftStatus = FT_SetBaudRate(ftHandle, (ULONG)115200);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_SetBaudRate(%d) returned %d.\n", 
		       115200,
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

	ftStatus = FT_SetTimeouts(ftHandle, 0, 0);	// 3 seconds
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_SetTimeouts returned %d\n", (int)ftStatus);
		goto exit;
	}
	printf("FTDI OK\n");
	exit:
	if(ftStatus != FT_OK)
	{
		if (ftHandle != NULL)
			FT_Close(ftHandle);
	}
}