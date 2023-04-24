#include <stdio.h>
#include <assert.h>
#include "ftd2xx.h"
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>

#define EMITTER_ID 0x2006 // id de l'emetteur
#define EMITTER_HEADER 0x50555345 // header des commandes 0x45535550

//unités pour l'utilisation du timer
#define SEC_IN_MICRO 				1000000
#define MILLI_IN_MICRO 				1000

extern uint8_t debug;
extern FT_HANDLE  ftHandle;

void purgeBuffer();
void lenghtQueue(DWORD* RxBytes);

void byteFlip(uint16_t * two_byte_int);

//lis la partie data du buffer de réponse envoyé par l'emetteur lors d'une requete 'Get Result'
void readData(uint8_t buffer[], uint8_t * data_target);

// envoie une commande via l'adaptateur rs485
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t send_command_request(uint32_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type,
                                uint8_t data[]);


//envoie une commande attendant une réponse après avoir envoyé une commande
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t send_GetResult_request(uint32_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type,
                                uint8_t * data,
                                uint8_t * reponse);
