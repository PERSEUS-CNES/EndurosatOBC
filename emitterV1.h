#include "ftd2xx.h"
#include "es_crc32.h"


#define EMITTER_ID 0x2006 // id de l'emetteur
#define EMITTER_HEADER 0x50555345 // header des commandes

extern FT_HANDLE  ftHandle;

FT_STATUS initialize_FTDI(int baudRate, int portNum);
void purgeBuffer();
void lenghtQueue(DWORD* RxBytes);

void byteFlip(uint16_t * two_byte_int);

//lis la partie data du buffer de réponse envoyé par l'emetteur lors d'une requete 'Get Result'
void readData(uint8_t buffer[], uint8_t * data_target);

// envoie une commande via l'adaptateur rs485
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t send_command_request(uint8_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type,
                                uint8_t data[]);


//envoie une commande attendant une réponse après avoir envoyé une commande
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t send_GetResult_request(uint8_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type,
                                uint8_t * reponse);

//créé un fichier dans l'emetteur
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t createFile(char name[], char fileHandle[]);

//supprime tous les fichiers de l'emetteur
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t deleteAllFiles();

//écrit dans un fichier préalablement créé
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t writeInFile(char fileHandle[], char content[], uint32_t packetNb);

// ouvre un fichier préalablement créé
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t openFile(char name[], char fileHandle[]);

//lis le conenu d'un fichier préalablement ouvert
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t readFile(char fileHandle[], char lecture[]);