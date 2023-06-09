#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>  
#include "emitter_reading.h"
#include "emitter_commands.h"

uint8_t openFile(char name[], char fileHandle[])
{
	//header 0x45535550
    //Module id 0xXXXX
    //data lenght  size du name
    //command status 0x0000
    //command 0x0106
    //type 0x0000
    //data
    //crc

    uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = strlen(name)+1;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0108;
    uint16_t type = 0x0000;
	uint8_t  data_read[10];

    uint8_t *  data = malloc(sizeof(uint8_t)*(data_lenght));

    uint8_t status = 0;


    int comm_lenght = 48;
    memcpy(data,name,sizeof(uint8_t)*data_lenght);

    

    printf("Ouverture du fichier %s\n", name);
    status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
    free(data);
	if(!status)
        return 0;
        
    printf("commande d'ouverture du fichier à  marchée\n");
    uint8_t data_get[1] = {0};
    type = command;
    command = 0x0114;
    comm_lenght = 32;
    data_lenght = 0;
    printf("send getResult pour l'ouverture du fichier \n");
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_get,data_read);    
  
    if(!status)
        return 0;
        
    printf("send getResult  à  marché\n");
	printf("open result %.2X\n", data_read[0]);
	if(data_read[0] == 0x00)
	{
		printf("le fichier à été ouvert; handle : \n");
		for(int i = 0; i < 4; i++)
		{
			fileHandle[i] = data_read[1+i];
			printf("%.2X ",fileHandle[i]);

		}
		printf("\n");
	}
	else
	{
		printf("echec dans l'ouverture du fichier\n");
		return 0;
	}
    return 1;

}

uint8_t readFile(char fileHandle[], char lecture[])
{
	uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = 0X0004;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0109;
    uint16_t type = 0x0000;
	uint8_t * data = malloc(sizeof(uint8_t)*data_lenght);

	uint8_t comm_lenght = 48;
	uint8_t status = 0;
	memcpy(data,fileHandle,sizeof(uint8_t)*data_lenght); 

	printf("lecture");
    status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
    free(data);
	if(!status)
        return 0;
        
    printf("commande de lecture du fichier à  marchée\n");
    uint8_t data_get[1] = {0};
    type = command;
    command = 0x0114;
    comm_lenght = 32;
    data_lenght = 0;
    printf("send getResult pour la lecture du fichier \n");
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_get,lecture);    
  
    if(!status)
        return 0;
        
    printf("send getResult  à  marché\n");
	printf("read result %d\n",(int)lecture[0]);
	if(lecture[0] == 0x00)
	{
		printf("le fichier à été lu; texte : \n");
		for(int i = 0; i < 256; i++)
		{
			printf("%c ",lecture[i]);

		}
		printf("\n");
	}
	else
	{
		printf("echec dans la lecture du fichier\n");
		return 0;
	}
    return 1;


	
}