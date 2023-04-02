#include <stdlib.h>
#include <stdint.h>

// ouvre un fichier préalablement créé
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t openFile(char name[], char fileHandle[]);

//lis le conenu d'un fichier préalablement ouvert
//retourne 1 en cas de succès et 0 en cas d'échec
uint8_t readFile(char fileHandle[], char lecture[]);
