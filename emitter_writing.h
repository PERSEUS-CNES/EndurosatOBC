#include <stdlib.h>
#include <stdint.h>

//créé un fichier dans l'emetteur
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t createFile(char name[], uint32_t size, char fileHandle[]);

//supprime tous les fichiers de l'emetteur
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t deleteAllFiles();

//écrit dans un fichier préalablement créé
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t writeInFile(char fileHandle[], char content[], uint32_t packetNb);
