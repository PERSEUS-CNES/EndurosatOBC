#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "emitter_config.h"
#include "emitter_commands.h"

FT_HANDLE  ftHandle = NULL;
uint8_t set_emitter_config(struct configuration * parametres,  parameters config_changes) 
{
	uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = 0;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0101;
    uint16_t type = 0x0048;
	uint8_t * data;
	uint8_t lecture[1];

	uint8_t comm_lenght = 48;
	uint8_t status = 0;
	//memcpy(data,fileHandle,sizeof(uint8_t)*data_lenght); 
	 /* uint8_t symbol_rate;
    uint8_t transmit_power;
    uint8_t MODCOD;
    uint8_t roll_off;
    uint8_t pilot_signal;
    uint8_t FEC_frame_size;
    uint16_t pretransmission_delay;
    float center_frequency;*/
	switch (config_changes)
	{
		case all_parameters :
			data_lenght = 0x000C; 
			data = malloc(sizeof(uint8_t)*data_lenght);

			data[0] = parametres -> symbol_rate;
			data[1] = parametres -> transmit_power;
			data[2] = parametres -> MODCOD;
			data[3] = parametres -> roll_off;
			data[4] = parametres -> pilot_signal;
			data[5] = parametres -> FEC_frame_size;

			memcpy(data + 6, &(parametres -> pretransmission_delay),sizeof(uint16_t));
			memcpy(data + 8, &(parametres -> center_frequency),sizeof(uint32_t));	
		break;
		case parm_symbol_rate:
			data_lenght = sizeof(uint8_t);
			data = malloc(sizeof(uint8_t)*data_lenght);
			memcpy(data, &(parametres -> symbol_rate),data_lenght);
		break;
		case parm_transmit_power:
			data_lenght = sizeof(uint8_t);
			data = malloc(data_lenght);
			memcpy(data, &(parametres -> transmit_power),data_lenght);
		break;
		case parm_MODCOD:
			data_lenght = sizeof(uint8_t);
			data = malloc(data_lenght);
			memcpy(data, &(parametres -> MODCOD),data_lenght);
		break;
		case parm_roll_off:
			data_lenght = sizeof(uint8_t);
			data = malloc(data_lenght);
			memcpy(data, &(parametres -> roll_off),data_lenght);
		break;
		case parm_pilot_signal:
			data_lenght = sizeof(uint8_t);
			data = malloc(data_lenght);
			memcpy(data, &(parametres -> pilot_signal),data_lenght);
		break;
		case parm_FEC_frame_size:
			data_lenght = sizeof(uint8_t);
			data = malloc(data_lenght);
			memcpy(data, &(parametres -> FEC_frame_size),data_lenght);
		break;
		case parm_pretransmission_delay:
			data_lenght = sizeof(uint16_t);
			data = malloc(data_lenght);
			memcpy(data, &(parametres -> symbol_rate),data_lenght);
		break;
		case parm_center_frequency:
			data_lenght = sizeof(float);
			data = malloc(data_lenght);
			memcpy(data, &(parametres -> center_frequency),data_lenght);
		break;
	}
	
	

	printf("configuration");
    status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
    free(data);
	if(!status)
        return 0;
        
    printf("commande de configuration  à  marchée\n");
    type = command;
    command = 0x0114;
    comm_lenght = 32;
    data_lenght = 0;
    printf("send getResult pour la configuration \n");
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,lecture);    
  
    if(!status)
        return 0;
        
    printf("send getResult  à  marché\n");
	printf("config result %d\n",(int)lecture[0]);
	if(lecture[0] == 0x00)
	{
		printf("configuration acceptée\n");
		
	}
	else
	{
		printf("echec dans la configuration\n");
		return 0;
	}
    return 1;
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