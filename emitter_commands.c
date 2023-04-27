#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "ftd2xx.h"
#include "emitter_commands.h"
#include "emitter_config.h"

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
		printf("%.2X , ", data_target[i]);}
    printf(";\n");
}

void byteFlip(uint16_t * two_byte_int)
{
    uint16_t byte1 = *two_byte_int >> 8;
    uint16_t byte2 = *two_byte_int << 8;

    *two_byte_int = byte1 + byte2;
}

// envoie une commande à partir des differents elements qui lui sont donnés
uint8_t send_command_request(uint32_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type,
                                uint8_t data[])
{

	struct timeval start;  // timer
	struct timeval temps_actuel;

    uint32_t crc=0;
    DWORD bytesToWrite = 0;
    DWORD bytesWritten = 0;
    DWORD RxBytes = 32;
	DWORD BytesReceived;
    uint8_t RxBuffer[32];
    FT_STATUS  ftStatus = FT_OK;

    uint8_t command_buffer[command_size];
    for(int i = 0; i < command_size; i++) // initialisation
    {
        command_buffer[i] = 0x00;
    }
	for(int i = 0; i < 32; i++)
	{
		RxBuffer[i] = 0x00;
	}
    
    uint16_t current_position = 0; // position actuelle dans le buffer
    
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

    if(debug != 0)
	{
    	printf("commande : \n");
    	for(int i = 0; i < command_size; i++)
    	{
        	printf("%.2X " , command_buffer[i]);
    	}
    	printf("\n");
	}
	

	uint8_t loop_01 = 1; // true
	uint8_t loop_02 = 1;
	// sending the command
	while(loop_01)
	{
		loop_01 = 0; // false
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
	
		// timeout 
		gettimeofday(&start, NULL);
		uint32_t start_ms_resp_read = (start.tv_sec * SEC_IN_MICRO) + start.tv_usec;
		gettimeofday(&temps_actuel, NULL);
		uint32_t actuel_ms_resp_read = (temps_actuel.tv_sec * SEC_IN_MICRO) + temps_actuel.tv_usec;
		uint32_t current_cycle_resp_read =  actuel_ms_resp_read - start_ms_resp_read;
		loop_02 = 1;
		while(loop_02 && current_cycle_resp_read <10  * MILLI_IN_MICRO)
		{
			loop_02 = 0;
			RxBytes=0;
			
			gettimeofday(&start, NULL); // enregistre l'heure actuelle dans la variable start
			uint32_t start_ms_wait_resp = (start.tv_sec * SEC_IN_MICRO) + start.tv_usec;
	
			gettimeofday(&temps_actuel, NULL);
			uint32_t actuel_ms_wait_resp = (temps_actuel.tv_sec * SEC_IN_MICRO) + temps_actuel.tv_usec;
			uint32_t current_cycle_wait_resp =  actuel_ms_wait_resp - start_ms_wait_resp;
			while(current_cycle_wait_resp < 2 * MILLI_IN_MICRO && !RxBytes)
			{
			
				gettimeofday(&temps_actuel, NULL);
				actuel_ms_wait_resp = (temps_actuel.tv_sec * SEC_IN_MICRO) + temps_actuel.tv_usec;
				current_cycle_wait_resp = actuel_ms_wait_resp - start_ms_wait_resp;
				usleep(100);
				lenghtQueue(&RxBytes); // récupère le nombre de bytes que l'emetteur cherche a envoyer à l'obc
			}

			if(!RxBytes)
			{
				printf("Aucune reponse\n");

				//loop_01 = 1;
				//usleep(100);

				return 0;
				
			}
		
			if(RxBytes)
				ftStatus = 	FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived); // lis la réponse de l'emetteur
			if (RxBytes && ftStatus == FT_OK) { // si la réception à fonctionnée
				if (BytesReceived == RxBytes) { // si tous les bytes ont été lus
					// FT_Read OK
					printf("\nBytes red : %i \n", RxBytes);
					if(debug != 0)
					{
						printf("Reponse Commande : "); // affiche la réponse
						for(int i = 0; i < BytesReceived; i++)
						{
							printf("%.2X ", RxBuffer[i]);
						}
						printf("\n");
					}
					if(RxBuffer[8] == 0x05)
					{
						printf("ACKNOWLEDGED \n");
						loop_01 = 0x00; // false
					}
					else if(RxBuffer[8] == 0x06)
					{
						printf("NOT ACKNOWLEDGABLE\n");
						loop_01 = 0x00; // false
						return 0;
					}
					else if(RxBuffer[8] == 0x07)
					{
						printf("BUSY\n");
						usleep(100);
						loop_01 = 0;

						// retour dans la loop de lecture de la réponse
						gettimeofday(&temps_actuel, NULL);
						actuel_ms_resp_read = (temps_actuel.tv_sec * SEC_IN_MICRO) + temps_actuel.tv_usec;
						current_cycle_resp_read =  actuel_ms_resp_read - start_ms_resp_read;
						loop_02 = 1;
					}
					else if(RxBuffer[8] == 0x10)
					{
						printf("TEMPORARILLY0x01 NOT ACCEPTABLE\n");
						loop_01 = 1; //true
						usleep(1000);
					}
					else if(RxBuffer[0] != 0x45)
					{
						printf("WRONG HEADER\n");
						
						//loop_01 = 1;
					}

				}
		
			}
		}
		//printf("loop 01 = %d\n",(int)loop_01);
	}
	//usleep(1);

    return 1;
}

uint8_t send_GetResult_request(uint32_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type,
								uint8_t * data,
								uint8_t * reponse)
{
    uint32_t crc=0;
    DWORD bytesToWrite = 0;
    DWORD bytesWritten = 0;
    DWORD RxBytes = 32;
	DWORD BytesReceived;
    uint8_t RxBuffer[256];
    FT_STATUS  ftStatus = FT_OK;

	struct timeval start;  // timer
	struct timeval temps_actuel;

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
	memcpy(command_buffer + current_position,data,sizeof(uint8_t)*data_lenght);
    current_position = current_position + data_lenght;

    //crc
    crc=crc32(0,command_buffer,current_position);
    memcpy(command_buffer + current_position,&crc,sizeof(uint32_t));
	uint8_t loop_01 = 1;
	gettimeofday(&start, NULL);
	uint32_t start_ms_resp_read = (start.tv_sec * SEC_IN_MICRO) + start.tv_usec;
	gettimeofday(&temps_actuel, NULL);
	uint32_t actuel_ms_resp_read = (temps_actuel.tv_sec * SEC_IN_MICRO) + temps_actuel.tv_usec;
	uint32_t current_cycle_resp_read =  actuel_ms_resp_read - start_ms_resp_read;
	
	while(loop_01 ){
		RxBytes=0;
		loop_01 = 0;

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
			return 0;
		}
		printf("Successfully wrote %d bytes\n", (int)bytesWritten);

		RxBytes=0;
		// demarre le timeout pour attendre la réponse
		gettimeofday(&start, NULL); // enregistre l'heure actuelle dans la variable start
		uint32_t start_ms_wait_resp = (start.tv_sec * SEC_IN_MICRO) + start.tv_usec;
	
		gettimeofday(&temps_actuel, NULL);
		uint32_t actuel_ms_wait_resp = (temps_actuel.tv_sec * SEC_IN_MICRO) + temps_actuel.tv_usec;
		uint32_t current_cycle_wait_resp =  actuel_ms_wait_resp - start_ms_wait_resp;
		while(current_cycle_wait_resp < 1 * MILLI_IN_MICRO && !RxBytes)
		{
		
			gettimeofday(&temps_actuel, NULL);
			actuel_ms_wait_resp = (temps_actuel.tv_sec * SEC_IN_MICRO) + temps_actuel.tv_usec;
			current_cycle_wait_resp = actuel_ms_wait_resp - start_ms_wait_resp;
			usleep(100);
			lenghtQueue(&RxBytes); // récupère le nombre de bytes que l'emetteur cherche a envoyer à l'obc
		}

		if (RxBytes) { // si une réponse est envoyée via le module par l'emetteur
	
			ftStatus = FT_Read(ftHandle,RxBuffer,RxBytes,&BytesReceived);// récupère la réponse
			if (ftStatus == FT_OK) {
				if (BytesReceived == RxBytes) {
					// FT_Read OK
					printf("\nBytes red gt : %i\n", RxBytes);
					if(debug != 0)
					{
            			printf("Reponse : ");
           				for(int i = 0; i < 256; i++) // affiche la réponse
           				{
            				printf("%.2X ",RxBuffer[i]); 
           				}
		   				printf("\n");
					}
					
		    		if(RxBuffer[8] == 0x00)
		    		{
						readData(RxBuffer,reponse);  //récupère la partie data de la réponse
					}
					else if(RxBuffer[8] == 0x08)
					{
						printf("NO COMMAND FOR EXECUTION\n");
						usleep(1000);
						loop_01 = 0;
						return 0;
					}
					else if(RxBuffer[8] == 0x07)
					{
						printf("BUSY g\n");
						loop_01 = 1;
						gettimeofday(&temps_actuel, NULL);
						actuel_ms_resp_read = (temps_actuel.tv_sec * SEC_IN_MICRO) + temps_actuel.tv_usec;
						current_cycle_resp_read =  actuel_ms_resp_read - start_ms_resp_read;
					}
				}
			}			
		}
		else
		{
			printf("NO RESPONSE \n");
			return 0;
		}
		
	}
	///usleep(5000);	
    return 1;
}
