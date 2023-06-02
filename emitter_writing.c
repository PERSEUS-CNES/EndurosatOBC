#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "emitter_commands.h"
#include "emitter_writing.h"
#include "debug.h"

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
	uint16_t name_lenght = strlen(name) + 1 ; //1 caractere en plus pour  prendre en compte \0
	uint32_t header = EMITTER_HEADER;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = name_lenght + 4; //taille des data : nom + 4 pour la taille
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0106; // commande createFile
    uint16_t type = 0x0000;
	uint8_t  data_read[10];
	
    uint8_t *  data = malloc(sizeof(uint8_t)*(data_lenght));

    uint8_t status = 0;
    int comm_lenght = 48;
    memcpy(data, name,sizeof(uint8_t)*name_lenght);
	memcpy(data+name_lenght, &size,sizeof(uint8_t)*4);
	

	

	#ifdef DEBUG_FUNC
    printf("Création du fichier %s\n", name);
	#endif
	// envoi de la commande
    status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
    
	if(!status)
        return 0;
        
	#ifdef DEBUG_FUNC
    printf("commande de creation du fichier à  marchée\n");
	#endif
	uint8_t data_get[1] = {0};
    type = command;
    command = 0x0114; // commande de la requete 'getResult'
    comm_lenght = 32;
    data_lenght = 0;
	#ifdef DEBUG_FUNC
    printf("send getResult pour la création du fichier \n");
	#endif
	// envoi de la requete getResult et récuperation de la réponse
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_get,data_read);    
  
    if(!status)
        return 0;
	
    #ifdef DEBUG_FUNC 
    printf("send getResult  à  marché\n");
	printf("create result %.2X\n", data_read[0]);
	#endif
	if(data_read[0] == 0x00)
	{
		#ifdef DEBUG_FUNC
		printf("le fichier à été créé; handle : \n");
		for(int i = 0; i < 4; i++)
		{
			fileHandle[i] = data_read[1+i];
			printf("%.2X",fileHandle[i]);

		}
		#endif
	}
	else
	{	
		#ifdef DEBUG_FUNC
		printf("echec dans la création du fichier\n");
		#endif
		return 0;
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
	#ifdef DEBUG_FUNC
	printf("SUPRESSION DE TOUT LES FICHIERS\n");
	#endif
	status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);

	if(!status)
	{
		return 0;
	}
	uint8_t data_get[1] = {0};
	type = command;
	command = 0x0114;
	uint8_t data_read[10];
	status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_get,data_read);
	#ifdef DEBUG_FUNC
	printf("delete All result %d \n",(int)data_read[0]);
	if(!status)
	{
		printf("echec lors de la récupération de la réponse\n");
	}
	#endif
	return 1;

}


uint8_t writeInFile(char fileHandle[], char content[],uint16_t content_size, uint32_t packetNb)
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
	uint16_t data_lenght = 10 + content_size;
	#ifdef DEBUG_FUNC
	printf("\ndata_len = %d\n",(int)content_size);
	#endif
	
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

	uint8_t status = 0;
	#ifdef DEBUG_FUNC
	printf("ECRITURE\n");
	#endif
	status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
	free(data);
	if(!status)
	{
		#ifdef DEBUG_FUNC
		printf("la commande n'a pas pu s'envoyer\n");
		#endif
		return 0;
	}
	data_lenght = 0;
	type = command;
	command = 0x0114;
	uint8_t data_read[10];
	uint8_t data_get[1] = {0};
	comm_lenght = 32;
	status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_get,data_read);
	#ifdef DEBUG_FUNC
	printf("write result %.2X \n", data_read[0]);
	#endif
	if(!status)
	{
		#ifdef DEBUG_FUNC
		printf("echec lors de la récupération de la réponse\n");
		#endif
		return 0;
	}
	else if (data_read[0] != 0x00)
	{
		#ifdef DEBUG_FUNC
		printf("erreur lors de l'écruture du fichier \n");
		#endif
		return 0;
	}

	return 1;	
}

//répartit l'écriture d'un buffer de plus de 1472 octets pour l'écrire dans un seul fichier
uint8_t writeMultiple(uint8_t fileHandle[], uint8_t content[], uint32_t buffer_size)
{
	uint32_t nbPackets = (buffer_size / BUFFER_MAX_LENGHT); // conne le nombre de packets à envoyer
	
	uint32_t current_Packet = 0;

	uint8_t loop = 1;
	uint8_t status = 1;
	uint8_t data_to_write[BUFFER_MAX_LENGHT];
	uint32_t lenght_to_write = 0;
	
	while(current_Packet < nbPackets && status) // envoie le contenu par tranches de 1472 bytes
	{
		#ifdef DEBUG_FUNC
		printf("---------------------------------Packet number = %d\n", (int)current_Packet);
		#endif
		memcpy(data_to_write, content + BUFFER_MAX_LENGHT*current_Packet, BUFFER_MAX_LENGHT);
		status = writeInFile(fileHandle, data_to_write, BUFFER_MAX_LENGHT, current_Packet);
		if(status)
		{
			current_Packet++;
		}
		else
		{
			#ifdef DEBUG_FUNC
			printf("Write ERROR !!!!!!!\n");
			#endif
			//printf("!!!!!!!!!!! Restart from begin\n");
			//deleteAllFiles();
			//current_Packet = 0;
			//exit(0);
		}
		usleep(5000);
	}

	// envoie le dernier packet
	lenght_to_write = buffer_size - (BUFFER_MAX_LENGHT * nbPackets);
	if(lenght_to_write > 0)
	{
		memcpy(data_to_write, content + BUFFER_MAX_LENGHT*current_Packet, lenght_to_write);
		status = writeInFile(fileHandle, data_to_write,lenght_to_write, current_Packet);	
	}
	loop = 0;
	usleep(5000);
	
	return 1;
}