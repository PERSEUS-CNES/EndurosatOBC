#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "emitter_commands.h"
#include "emitter_sending.h"


// envoie le fichier séléctionné par son nom
// le fichier doit etre préalabmement créé avec la fonction create_file
// l'emetteur doit etre passé en mode "transmit" par la fonction transmit_mode()
uint8_t sendFile(char fileName[])
{
	uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = strlen(fileName)+1;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x010A;
    uint16_t type = 0x0050;
	uint8_t  data_read[10];

    uint8_t *  data = malloc(sizeof(uint8_t)*(data_lenght));

    uint8_t status = 0;


    int comm_lenght = 48;
    memcpy(data,fileName,sizeof(uint8_t)*data_lenght);

    
    #ifdef DEBUG_FUNC
    printf("envoi du fichier %s\n", fileName);
    #endif
    status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
    free(data);
	if(!status)
        return 0;
    
    #ifdef DEBUG_FUNC
    printf("commande transmit mode marchée\n");
    #endif
    data_lenght = 0x0002;
    uint8_t data_get[2];
    memcpy(data_get, &type,sizeof(uint8_t)*data_lenght);
    type = command;
    command = 0x0114;
    comm_lenght = 32;

    #ifdef DEBUG_FUNC
    printf("send getResult pour transmit mode \n");
    #endif

    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_get,data_read);    
  
    if(!status)
        return 0;
        
    #ifdef DEBUG_FUNC
    printf("send getResult  à  marché\n");
	printf("send result %.2X\n", data_read[0]);
    #endif

	if(data_read[0] == 0x00)
	{
        #ifdef DEBUG_FUNC
		printf("-------------------------------------------le fichier à été envoyé\n");
        #endif
	}
	else
	{
        #ifdef DEBUG_FUNC
		printf("echec dans l'envoi du fichier\n");
        #endif
		return 0;
	}
    return 1;
}

//change le mode de transmit si on = 1, et idle si on = 0
uint8_t transmit_mode(uint8_t on)
{
	uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = 0;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0110;
	if(on == 0x01)
	{
        #ifdef DEBUG_FUNC
		printf("Passage de l'emetteur en transmit mode\n");
        #endif
	}
	else
	{
        #ifdef DEBUG_FUNC
		printf("Passage de l'emetteur en idle mode\n");
        #endif
		command = 0x0111;
	}
    uint16_t type = 0x0000;
	uint8_t data[1];
	data[0] = 0;
	uint8_t data_read[2];

	uint8_t status = 0;

	int comm_lenght = 32;

	status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
	if(!status)
        return 0;
        
    #ifdef DEBUG_FUNC
    printf("commande de mode de transmission a marche \n");
    #endif
    uint8_t data_get[1] = {0};
    type = command;
    command = 0x0114;
    comm_lenght = 32;
    data_lenght = 0;

    #ifdef DEBUG_FUNC
    printf("send getResult pour le mode d'envoi \n");
    #endif
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_get,data_read);    
  
    if(!status)
        return 0;
        
    #ifdef DEBUG_FUNC
    printf("send getResult  à  marché\n");
	printf("transmit result %.2X\n", data_read[0]);
    #endif
	if(data_read[0] == 0x00)
	{
        #ifdef DEBUG_FUNC
		printf("le mode de transmission a ete changé à %d .\n", (int)on );
        #endif
	}
	else
	{
        #ifdef DEBUG_FUNC
		printf("echec changement de mode\n");
        #endif
		return 0;
	}
    return 1;
}


