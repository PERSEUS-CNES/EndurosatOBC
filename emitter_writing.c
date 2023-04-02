#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "emitter_commands.h"
#include "emitter_writing.h"

uint8_t createFile(char name[],uint32_t size , char fileHandle[]) // constitue la commande pour créer un fichier et l'envoie à l'emetteur
{
    //header 0x45535550
    //Module id 0xXXXX
    //data lenght  size du name
    //command status 0x0000
    //command 0x0106
    //type 0x0000
    //data
    //crc

	// construction de la commande à partir des informations récupérées de la documentation de l'emetteur
    uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = strlen(name)+5;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0106; // commande createFile
    uint16_t type = 0x0000;
	uint8_t  data_read[10];

    uint8_t *  data = malloc(sizeof(uint8_t)*(data_lenght));

    uint8_t status = 0;
	uint8_t try = 1;

    int comm_lenght = 48;
    memcpy(data,name,sizeof(uint8_t)*data_lenght);

	while(try){
		try = 0;
    printf("Création du fichier %s\n", name);
	// envoi de la commande
    status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
    
	if(!status)
        return 0;
        
    printf("commande de creation du fichier à  marchée\n");
    type = command;
    command = 0x0114; // commande de la requete 'getResult'
    comm_lenght = 32;
    data_lenght = 0;
    printf("send getResult pour la création du fichier \n");
	// envoi de la requete getResult et récuperation de la réponse
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_read);    
  
    if(!status)
        return 0;
	
        
    printf("send getResult  à  marché\n");
	printf("create result %2.X\n",(int)data_read[0]);
	if(data_read[0] == 0x00)
	{
		printf("le fichier à été créé; handle : \n");
		for(int i = 0; i < 4; i++)
		{
			fileHandle[i] = data_read[1+i];
			printf("%2.X",fileHandle[i]);

		}
	}
	else
	{
		printf("echec dans la création du fichier\n");
		return 0;
	}
	}
	free(data);
    return 1;
 
}

uint8_t deleteAllFiles()
{
	//header 0x45535550
    //Module id 0xXXXX
    //data lenght  0x0000
    //command status 0x0000
    //command 0x0105
    //type 0x0000
    //data
    //crc

	uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = 0x0000;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0105;
    uint16_t type = 0x0000;
	uint8_t *  data = {0x00};

	uint8_t status = 0;

	uint32_t comm_lenght = 32;
	printf("SUPRESSION DE TOUT LES FICHIERS\n");
	status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);

	if(!status)
	{
		//return 0;
	}

	type = command;
	command = 0x0114;
	uint8_t data_read[10];
	status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_read);
	printf("delete All result %d \n",(int)data_read[0]);
	if(!status)
	{
		printf("echec lors de la récupération de la réponse\n");
	}

	return 1;

}


uint8_t writeInFile(char fileHandle[], char content[], uint32_t packetNb)
{
	//header 0x45535550
    //Module id 0xXXXX
    //data lenght  2 (taille des données) + 4 (handle) + 4 ( nb packet) + N(données)
    //command status 0x0000
    //command 0x0105
    //type 0x0000
    //data
    //crc

	uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0107;
    uint16_t type = 0x0000;
	uint8_t *  data;
	uint16_t content_size = strlen(content); 
	uint16_t data_lenght = 10 + content_size;

	printf("\ncontent %s data_len = %d\n",content, (int)content_size);
	
	//uint32_t packetNb = 0; 
	data = malloc(sizeof(uint8_t)*(data_lenght));
	uint32_t comm_lenght = 32 + data_lenght;
	comm_lenght = 16*(1+((int)(comm_lenght/16))); //la longueur de la commande doit etre un multiple de 16

	memcpy(data,&content_size, sizeof(uint16_t));
	memcpy(data + 2, fileHandle,sizeof(uint8_t)*4);
	
	memcpy(data + 6,&packetNb, sizeof(uint32_t));
	for(int i = 0; i < content_size; i++)
	{
		data[i + 10] = content[i];
	}

	for(int i = 0; i < 10 + content_size; i++)
	{
		printf("%d ",(int)data[i]);
	}
	printf("\n");

	uint8_t status = 0;
	printf("ECRITURE\n");
	status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
	free(data);
	if(!status)
	{
		printf("la commande n'a pas pu s'envoyer\n");
		return 0;
	}
	data_lenght = 0;
	type = command;
	command = 0x0114;
	uint8_t data_read[10];

	comm_lenght = 32;
	status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_read);
	printf("write result %d \n",(int)data_read[0]);
	if(!status)
	{
		printf("echec lors de la récupération de la réponse\n");
		return 0;
	}
	else if (data_read[0] != 0x00)
	{
		printf("erreur lors de l'écruture du fichier \n");
		return 0;
	}

	return 1;	
}