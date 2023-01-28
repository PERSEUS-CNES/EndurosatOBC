#include "ftd2xx.h"
#include "es_crc32.h"


#define EMITTER_ID 0x2006 // id de l'emetteur

extern FT_HANDLE  ftHandle;

FT_STATUS initialize_FTDI(int baudRate, int portNum);
void purgeBuffer();
void lenghtQueue(DWORD* RxBytes);

void byteFlip(uint16_t * two_byte_int);

void readData(uint8_t buffer[], uint8_t * data_target);
uint8_t send_command_request(uint8_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type,
                                uint8_t data[]);

uint8_t send_GetResult_request(uint8_t command_size, 
                                uint32_t header,
                                uint16_t id,
                                uint16_t data_lenght,
                                uint16_t command_status,
                                uint16_t command,
                                uint16_t type,
                                uint8_t * reponse);
uint8_t createFile(char name[], char fileHandle[]);
uint8_t deleteAllFiles();
uint8_t writeInFile(char fileHandle[], char content[]);
uint8_t openFile(char name[], char fileHandle[]);
uint8_t readFile(char fileHandle[], char lecture[]);