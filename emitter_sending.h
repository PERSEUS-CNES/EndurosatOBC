#include <stdlib.h>
#include <stdint.h>

//active ou éteint le mode de transmission
uint8_t tansmit_mode(uint8_t on);

//envoie un fichier
uint8_t sendFile(char fileName[]);