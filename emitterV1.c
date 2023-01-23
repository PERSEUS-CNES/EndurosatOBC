#include <stdio.h>
#include <assert.h>
#include "ftd2xx.h"
#include <string.h>
#include <unistd.h>
//#include "es_crc32.h"
#include <stdint.h>
#include <stdlib.h>
//#include "simpleEndurosat.h"

#include "emitterV1.h"



FT_HANDLE  ftHandle = NULL;

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
	// Paquets de 8 bits, 1 Stop bit , Pas de parit�
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

void purgeBuffer()
{
	int ftStatus = FT_OK;
	ftStatus = FT_Purge(ftHandle, FT_PURGE_RX | FT_PURGE_TX); // Purge both Rx and Tx buffers
	if(ftStatus == FT_OK)
	{
		printf("FT_Purge OK\n");
	}
	else
	{
		printf("FT_Purge failed\n");
	}
}

void lenghtQueue(DWORD* RxBytes)
{
	int count;
//	usleep(100);
	count = 400;
	*RxBytes=0;	
	while(count > 0)
	{	
		FT_GetQueueStatus(ftHandle,RxBytes);
		//printf("\n Queue status 22 : %i\n", *RxBytes);
		if(*RxBytes !=0 && *RxBytes%16 == 0)
		{
			break;
		}
		count--;
	}
	if(!(*RxBytes !=0 && *RxBytes%16 == 0))
	{
		*RxBytes=0;
	}
	printf("\n Queue status : %i\n", *RxBytes);
}

//lis la partie data du buffer

uint8_t readData(uint8_t buffer[] , uint8_t * * data)
{
    uint16_t data_lenght;
    
    //recupere la taille de la partie data a la position 6 et 7 du buffer
    //data_lenght = 0*buffer[7] + buffer[6]; 
	memcpy(&data_lenght,buffer + 6,sizeof(uint16_t));

    //printf(" taille donne %d\n",(int)data_lenght);
    // recupere les données a chaque case de la partie data du buffer
	*data = malloc(sizeof(uint8_t)*data_lenght);
    //printf("data ");
	memcpy(*data,buffer + 14, sizeof(uint8_t)*data_lenght);
    
    //printf("\n");
    return 0;
}

void byteFlip(uint16_t * two_byte_int)
{
    uint16_t byte1 = *two_byte_int >> 8;
    uint16_t byte2 = *two_byte_int << 8;

    *two_byte_int = byte1 + byte2;
}

// envoie une commande à partir des differents elements qui lui sont donnés
uint8_t send_command_request(uint8_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type,
                                uint8_t data[])
{



    // declarations
    uint32_t crc=0;
    DWORD bytesToWrite = 0;
    DWORD bytesWritten = 0;
    DWORD RxBytes = 32;
	DWORD BytesReceived;
    uint8_t RxBuffer[256];
    FT_STATUS  ftStatus = FT_OK;

    uint8_t command_buffer[command_size];
    for(int i = 0; i < command_size; i++) // initialisation
    {
        command_buffer[i] = 0x00;
    }
    
    
    int current_position = 0;
    
    //header
    memcpy(command_buffer, &header,sizeof(uint32_t));
    current_position = current_position + 4;

    //id
    //byteFlip(&id);
    memcpy(command_buffer + current_position,&id,sizeof(uint16_t));
    current_position += 2;
    
    //data_lenght
    //byteFlip(&data_lenght);
    memcpy(command_buffer + current_position,&data_lenght,sizeof(uint16_t));
    current_position += 2;
    //byteFlip(&data_lenght);
    
    //command_status
    //byteFlip(&command_status);
    memcpy(command_buffer + current_position,&command_status,sizeof(uint16_t));
    current_position += 2;

    //command
    //byteFlip(&command);
    memcpy(command_buffer + current_position,&command,sizeof(uint16_t));
    current_position += 2;
    
    
    //type
    //byteFlip(&type);
    memcpy(command_buffer + current_position,&type,sizeof(uint16_t));
    current_position += 2;

    //data
    memcpy(command_buffer + current_position,data,sizeof(uint8_t)*data_lenght);
    current_position = current_position + data_lenght;

    //crc
    crc=crc32(0,command_buffer,current_position);
    memcpy(command_buffer + current_position,&crc,sizeof(uint32_t));
    
    bytesToWrite=command_size;
	RxBytes=command_size;

    
    printf("commande : \n");
    for(int i = 0; i < command_size; i++)
    {
        printf("%.2X " ,(int)command_buffer[i]);
    }
    printf("\n");

  
	// sending the command
	purgeBuffer();
 
	ftStatus = FT_Write(ftHandle, 
	                    command_buffer,
	                    bytesToWrite, 
	                    &bytesWritten);
 
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		return 0;// error in the process of writing the command
	}
    if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten, 
		       (int)bytesToWrite);
		return 0;
	}

    printf("Successfully wrote %d bytes\n", (int)bytesWritten);
 
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			printf("Bytes red : %i\n", RxBytes);
			//patternCut(&RxBytes,RxBuffer);
		}
	}
	
	
	usleep(100);
	RxBytes=command_size;
	lenghtQueue(&RxBytes);
	ftStatus = 	FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
	if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			// FT_Read OK
			printf("\nBytes red : %i \n", RxBytes);
			//patternCut(&RxBytes,RxBuffer);
		}
		
	}
	usleep(2000);

    return 1;
}

uint8_t send_GetResult_request(uint8_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type)
{
    uint32_t crc=0;
    DWORD bytesToWrite = 0;
    DWORD bytesWritten = 0;
    DWORD RxBytes = 32;
	DWORD BytesReceived;
    uint8_t RxBuffer[256];
    FT_STATUS  ftStatus = FT_OK;
    

    uint8_t command_buffer[command_size];
    for(int i = 0; i < command_size; i++) // initialisation
    {
        command_buffer[i] = 0x00;
    }
    int current_position = 0;
    
   //header
    memcpy(command_buffer, &header,sizeof(uint32_t));
    current_position = current_position + 4;

    //id
    //byteFlip(&id);
    memcpy(command_buffer + current_position,&id,sizeof(uint16_t));
    current_position += 2;
    
    //data_lenght
    //byteFlip(&data_lenght);
    memcpy(command_buffer + current_position,&data_lenght,sizeof(uint16_t));
    current_position += 2;

    //command_status
    //byteFlip(&command_status);
    memcpy(command_buffer + current_position,&command_status,sizeof(uint16_t));
    current_position += 2;

    //command
    //byteFlip(&command);
    memcpy(command_buffer + current_position,&command,sizeof(uint16_t));
    current_position += 2;

    //type
    //byteFlip(&type);
    memcpy(command_buffer + current_position,&type,sizeof(uint16_t));
    current_position += 2;
    
    //data
    //memcpy(command_buffer + current_position,data,sizeof(uint8_t)*data_lenght);
    //current_position = current_position + data_lenght;

    //crc
    crc=crc32(0,command_buffer,current_position);
    memcpy(command_buffer + current_position,&crc,sizeof(uint32_t));

    bytesToWrite=command_size;
	RxBytes=command_size;
    

	purgeBuffer();
	bytesToWrite=command_size;
	ftStatus = FT_Write(ftHandle, 
							command_buffer,
							bytesToWrite, 
							&bytesWritten);
	if (ftStatus != FT_OK) 
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		return 2;
	}
	
	if (bytesWritten != bytesToWrite)
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten,
		       (int)bytesToWrite);
		return 2;
	}
	printf("Successfully wrote %d bytes\n", (int)bytesWritten);

	usleep(2000);
	lenghtQueue(&RxBytes);
	
	//uint8_t Handle[] = {0x00, 0x00, 0x00, 0x00};
//	uint8_t data_read;
 
   

	if (RxBytes) {
	
		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
				// FT_Read OK
			printf("\nBytes red gt : %i\n", RxBytes);
			//patternCutHandle(&RxBytes,RxBuffer,&Handle);
            //data_read = readData(RxBuffer);
            printf("Reponse : ");
           for(int i = 0; i < 256; i++)
           {
             printf("%.2X ",RxBuffer[i]); 
           }
           printf("\n");
			}
		}		
	}
	
	usleep(100);
	RxBytes=command_size;
	
	lenghtQueue(&RxBytes);
	

	/*if (RxBytes) {
	
		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
					// FT_Read OK
			printf("\nBytes red gt 2 : %i\n", RxBytes);
			//patternCutHandle(&RxBytes,RxBuffer,&Handle);
            
			}
		}
	
	}*/
	
	usleep(2000);	
    return 0;
}

uint8_t createFile(char name[]) // constitue la commande pour créer un fichier et l'envoie à l'emetteur
{
    //header 0x45535550
    //Module id 0xXXXX
    //data lenght  size du name
    //command status 0x0000
    //command 0x0106
    //type 0x0000
    //data
    //crc

    uint32_t header = 0x50555345;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = strlen(name);
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0106;
    uint16_t type = 0x0000;

    uint8_t *  data = malloc(sizeof(uint8_t)*(data_lenght));

    uint8_t status = 0;


    int comm_lenght = 48;
    memcpy(data,name,sizeof(uint8_t)*data_lenght);
    //data[data_lenght + 1] = 0x0A;
    //data_lenght  =+ 1;
    

    printf("Création du fichier %s\n", name);
    status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
    if(!status)
        return 0;
        
    printf("commande de creation du fichier à  marchée\n");
    type = command;
    command = 0x0114;
    comm_lenght = 32;
    data_lenght = 0;
    printf("send getResult pour la création du fichier \n");
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type);    
    if(status !=0)
        return 0;
        
    printf("send getResult  à  marché\n");
    return 1;
 
}
