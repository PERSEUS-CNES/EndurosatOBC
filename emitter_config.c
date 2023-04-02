#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "emitter_config.h"
#include "emitter_commands.h"

uint8_t set_emitter_config(struct configuration * parametres) 
{
	uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = 0x000C;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0101;
    uint16_t type = 0x0048;
	uint8_t * data = malloc(sizeof(uint8_t)*data_lenght);
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
	
	data[0] = parametres -> symbol_rate;
	data[1] = parametres -> transmit_power;
	data[2] = parametres -> MODCOD;
	data[3] = parametres -> roll_off;
	data[4] = parametres -> pilot_signal;
	data[5] = parametres -> FEC_frame_size;

	memcpy(data + 6, &(parametres -> pretransmission_delay),sizeof(uint16_t));
	memcpy(data + 8, &(parametres -> center_frequency),sizeof(uint32_t));
	

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