#include "ftd2xx.h"
#include "es_crc32.h"


#define EMITTER_ID 0x2006 // id de l'emetteur

extern FT_HANDLE  ftHandle;

FT_STATUS initialize_FTDI(int baudRate, int portNum);
void purgeBuffer();
void lenghtQueue(DWORD* RxBytes);

void byteFlip(uint16_t * two_byte_int);

uint8_t readData(uint8_t buffer[]);
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
                                uint16_t type);
uint8_t createFile(char name[]);