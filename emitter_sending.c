#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ftd2xx.h"
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

    

    printf("envoi du fichier %s\n", fileName);
    status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
    free(data);
	if(!status)
        return 0;
        
    printf("commande transmit mode marchée\n");
    free(data);
    data_lenght = 0x0002;
    uint8_t data_get[2];
    memcpy(data_get, &type,sizeof(uint8_t)*data_lenght);
    type = command;
    command = 0x0114;
    comm_lenght = 32;
    printf("send getResult pour transmit mode \n");
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_get,data_read);    
  
    if(!status)
        return 0;
        
    printf("send getResult  à  marché\n");
	printf("send result %2.X\n",(int)data_read[0]);
	if(data_read[0] == 0x00)
	{
		printf("-------------------------------------------le fichier à été envoyé\n");
	}
	else
	{
		printf("echec dans l'envoi du fichier\n");
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
		printf("Passage de l'emetteur en transmit mode\n");
	}
	else
	{
		printf("Passage de l'emetteur en idle mode\n");
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
        
    printf("commande de mode de transmission a marche \n");
    uint8_t data_get[1] = {0};
    type = command;
    command = 0x0114;
    comm_lenght = 32;
    data_lenght = 0;
    printf("send getResult pour le mode d'envoi \n");
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_get,data_read);    
  
    if(!status)
        return 0;
        
    printf("send getResult  à  marché\n");
	printf("transmit result %2.X\n",(int)data_read[0]);
	if(data_read[0] == 0x00)
	{
		printf("le mode de transmission a ete changé à %d .\n", (int)on );
	}
	else
	{
		printf("echec changement de mode\n");
		return 0;
	}
    return 1;
}


