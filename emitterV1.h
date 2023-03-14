#include "ftd2xx.h"
#include "es_crc32.h"


#define EMITTER_ID 0x2006 // id de l'emetteur
#define EMITTER_HEADER 0x50555345 // header des commandes 0x45535550

//unités pour l'utilisation du timer
#define SEC_IN_MICRO 				1000000
#define MILLI_IN_MICRO 				1000

struct configuration {
    uint8_t symbol_rate;
    uint8_t transmit_power;
    uint8_t MODCOD;
    uint8_t roll_off;
    uint8_t pilot_signal;
    uint8_t FEC_frame_size;
    uint16_t pretransmission_delay;
    float center_frequency;
};

//typedef emitter_config struct configuration;

extern FT_HANDLE  ftHandle;
extern uint8_t debug;

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

//change les paramètres de l'emetteur
uint8_t set_emitter_config(struct configuration * parametres);

//active ou éteint le mode de transmission
uint8_t tansmit_mode(uint8_t on);

//envoie un fichier
uint8_t sendFile(char fileName[]);