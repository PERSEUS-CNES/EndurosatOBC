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

//lis la partie data du buffer envoyé à la carte par l'emetteur
void readData(uint8_t buffer[] , uint8_t * data_target)
{
    uint16_t data_lenght;
    
    //recupere la taille de la partie data a la position 6 et 7 du buffer
	memcpy(&data_lenght,buffer + 6,sizeof(uint16_t));

    // recupere les données a chaque case de la partie data du buffer
	if(data_lenght == 0)
	{
		data_target[0] = 0x00;
	}
	for(int i = 0; i < data_lenght; i ++)
	{
		data_target[i] = buffer[14 + i];
	}
	printf("data : ");
	for(int i = 0; i < data_lenght; i++ ){
	printf("%d , ",(int)data_target[i]);}
    printf(";\n");
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
    
    
    int current_position = 0; // position actuelle dans le buffer
    
	// construction du buffer contenant la commande à envoyer
    //header
    memcpy(command_buffer, &header,sizeof(uint32_t));
    current_position = current_position + 4;

    //id
    memcpy(command_buffer + current_position,&id,sizeof(uint16_t));
    current_position += 2;
    
    //data_lenght
    memcpy(command_buffer + current_position,&data_lenght,sizeof(uint16_t));
    current_position += 2;
    
    //command_status
    memcpy(command_buffer + current_position,&command_status,sizeof(uint16_t));
    current_position += 2;

    //command
    memcpy(command_buffer + current_position,&command,sizeof(uint16_t));
    current_position += 2;
    
    //type
    memcpy(command_buffer + current_position,&type,sizeof(uint16_t));
    current_position += 2;

    //data
    memcpy(command_buffer + current_position,data,sizeof(uint8_t)*data_lenght);
    current_position = current_position + data_lenght;

    //crc
    crc=crc32(0,command_buffer,current_position);
    memcpy(command_buffer + current_position,&crc,sizeof(uint32_t));
    
    bytesToWrite=command_size;
	RxBytes=0;

    
    printf("commande : \n");
    for(int i = 0; i < command_size; i++)
    {
        printf("%.2X " ,(int)command_buffer[i]);
    }
    printf("\n");

  
	// sending the command
	purgeBuffer();
	
	// envoie la commande via l'adaptateur RS485
	ftStatus = FT_Write(ftHandle, 
	                    command_buffer,
	                    bytesToWrite, 
	                    &bytesWritten);
 
	if (ftStatus != FT_OK) // vérifie que le module RS485 a bien envoyé la commande
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		return 0;// error in the process of writing the command
	}
    if (bytesWritten != bytesToWrite)// vérifie que la commande à été entièrement envoyée
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten, 
		       (int)bytesToWrite);
		return 0;
	}

    printf("Successfully wrote %d bytes\n", (int)bytesWritten);
 
	/*if (ftStatus == FT_OK) {
		if (BytesReceived == RxBytes) {
			printf("Bytes red : %i\n", RxBytes);

		}
		else
		{
			printf("Erreur : reception de bytes manquants\n");
			return 0;
		}
	}*/
	
	
	usleep(100);
	RxBytes=0;
	lenghtQueue(&RxBytes); // récupère le nombre de bytes que l'emetteur cherche a envoyer à l'obc
	ftStatus = 	FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived); // lis la réponse de l'emetteur
	if (ftStatus == FT_OK) { // si la réception à fonctionnée
		if (BytesReceived == RxBytes) { // si tous les bytes ont été lus
			// FT_Read OK
			printf("\nBytes red : %i \n", RxBytes);
			printf("Reponse Commande : "); // affiche la réponse
			for(int i = 0; i < BytesReceived; i++)
			{
				printf("%d ",(int)RxBuffer[i]);
			}
			printf("\n");
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
                                uint16_t type,
								uint8_t * reponse)
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
    

	// construction du buffer contenant la commande à envoyer
   //header
    memcpy(command_buffer, &header,sizeof(uint32_t));
    current_position = current_position + 4;

    //id
    memcpy(command_buffer + current_position,&id,sizeof(uint16_t));
    current_position += 2;
    
    //data_lenght
    memcpy(command_buffer + current_position,&data_lenght,sizeof(uint16_t));
    current_position += 2;

    //command_status
    memcpy(command_buffer + current_position,&command_status,sizeof(uint16_t));
    current_position += 2;

    //command
    memcpy(command_buffer + current_position,&command,sizeof(uint16_t));
    current_position += 2;

    //type
    memcpy(command_buffer + current_position,&type,sizeof(uint16_t));
    current_position += 2;
    
    //data

    //crc
    crc=crc32(0,command_buffer,current_position);
    memcpy(command_buffer + current_position,&crc,sizeof(uint32_t));

	RxBytes=0;

	purgeBuffer();
	bytesToWrite=command_size;
	ftStatus = FT_Write(ftHandle, // envoie la requete via le module RS485
							command_buffer,
							bytesToWrite, 
							&bytesWritten);
	if (ftStatus != FT_OK) // verifie que l'envoi à fonctionné
	{
		printf("Failure.  FT_Write returned %d\n", (int)ftStatus);
		return 0;
	}
	
	if (bytesWritten != bytesToWrite) // vérifie que tous bytes ont été envoyé
	{
		printf("Failure.  FT_Write wrote %d bytes instead of %d.\n",
		       (int)bytesWritten,
		       (int)bytesToWrite);
		return 2;
	}
	printf("Successfully wrote %d bytes\n", (int)bytesWritten);

	usleep(2000);
	lenghtQueue(&RxBytes);

	if (RxBytes) { // si une réponse est envoyée via le module par l'emetteur
	
		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);// récupère la réponse
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
				// FT_Read OK
			printf("\nBytes red gt : %i\n", RxBytes);

            printf("Reponse : ");
           for(int i = 0; i < 256; i++) // affiche la réponse
           {
             printf("%.2X ",RxBuffer[i]); 
           }
		   printf("\n");
		   readData(RxBuffer,reponse);  //récupère la partie data de la réponse
           
			}
		}		
	}

	usleep(100);
	RxBytes=0;
	
	lenghtQueue(&RxBytes); // recommence pour une éventuelle 2nd réponse
	if (RxBytes) {
	
		ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);
		if (ftStatus == FT_OK) {
			if (BytesReceived == RxBytes) {
					// FT_Read OK
			printf("\nBytes red gt 2 : %i\n", RxBytes);
			printf("Reponse : ");
           for(int i = 0; i < 256; i++)
           {
             printf("%.2X ",RxBuffer[i]); 
           }
		   printf("\n");
			readData(RxBuffer,reponse);            
			}
		}
	
	}
	else
		printf("pas de 2nd réponse\n");
	
	usleep(2000);	
    return 1;
}

uint8_t createFile(char name[], char fileHandle[]) // constitue la commande pour créer un fichier et l'envoie à l'emetteur
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
    uint32_t header = 0x50555345;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = strlen(name)+1;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0106; // commande createFile
    uint16_t type = 0x0000;
	uint8_t  data_read[10];

    uint8_t *  data = malloc(sizeof(uint8_t)*(data_lenght));

    uint8_t status = 0;


    int comm_lenght = 48;
    memcpy(data,name,sizeof(uint8_t)*data_lenght);

    printf("Création du fichier %s\n", name);
	// envoi de la commande
    status = send_command_request(comm_lenght,header,id,data_lenght,command_status,command,type,data);
    free(data);
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

	uint32_t header = 0x50555345;//0x45 53 55 50;
    uint16_t id = EMITTER_ID;
    uint16_t data_lenght = 0x0000;
    uint16_t command_status = 0x0000;
    uint16_t command = 0x0105;
    uint16_t type = 0x0000;
	uint8_t *  data = {0x00};

	uint8_t status = 0;

	uint8_t comm_lenght = 32;
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

	uint32_t header = 0x50555345;//0x45 53 55 50;
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
	uint8_t comm_lenght = 32 + data_lenght;

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

	return 1;	
}

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

    uint32_t header = 0x50555345;//0x45 53 55 50;
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
    type = command;
    command = 0x0114;
    comm_lenght = 32;
    data_lenght = 0;
    printf("send getResult pour l'ouverture du fichier \n");
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,data_read);    
  
    if(!status)
        return 0;
        
    printf("send getResult  à  marché\n");
	printf("open result %2.X\n",(int)data_read[0]);
	if(data_read[0] == 0x00)
	{
		printf("le fichier à été ouvert; handle : \n");
		for(int i = 0; i < 4; i++)
		{
			fileHandle[i] = data_read[1+i];
			printf("%d ",fileHandle[i]);

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
	uint32_t header = 0x50555345;//0x45 53 55 50;
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
    type = command;
    command = 0x0114;
    comm_lenght = 32;
    data_lenght = 0;
    printf("send getResult pour la lecture du fichier \n");
    status = send_GetResult_request(comm_lenght,header,id,data_lenght,command_status,command,type,lecture);    
  
    if(!status)
        return 0;
        
    printf("send getResult  à  marché\n");
	printf("read result %d\n",(int)lecture[0]);
	if(lecture[0] == 0x00 || 1)
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
		printf("echec dans l'ouverture du fichier\n");
		return 0;
	}
    return 1;


	
}