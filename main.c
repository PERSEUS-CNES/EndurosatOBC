#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>


//#include "emitterV1.h"
#include "ftd2xx.h"
#include "es_crc32.h"

#include "emitter_config.h"
#include "emitter_reading.h"
#include "emitter_sending.h"
#include "emitter_writing.h"

uint8_t debug = 0;

// test l'ecriture de fichiers de taille importante
// le programme s'arrete si une des commandes ne fonctionne pas
int main(int argc, char * argv[])
{
   
    if(argc > 1)
    {
        if(!strcmp(argv[1],"DEBUG_ON"))
        debug = 1;
    }
    
    int port = 0;//atoi(argv[1]);
    int baud = 3000000;//atoi(argv[2]);
    uint8_t status = 0;//boolen
    uint32_t wait = 5000;
    
    FT_STATUS status_ft = initialize_FTDI(baud, port);
    
    // suppression de tout les fichiers
    status = deleteAllFiles();
    usleep(wait);
    char namefile[] = "file.txt";


    char * cfileHandle;
    cfileHandle = malloc(sizeof(uint8_t)*4);
    //char * ofileHandle;
    //ofileHandle = malloc(sizeof(uint8_t)*4);

    //uint16_t tailleFichier = 256;
    //char lecture[tailleFichier];
    
    // création du fichier
    const int taille_ecriture = 1000000; // taille de fichier : 1 giga octet
    uint8_t ecriture[taille_ecriture];
    for(int i = 0; i < taille_ecriture; i++)
    {
        if(i%2 == 0)
        {
            ecriture[i] = 'A';
        }
        else
        {
            ecriture[i] = 'B';
        }
    }
    //creation du fichier de 1 go
    if(status)
        status = createFile(namefile, taille_ecriture,cfileHandle);
    usleep(wait);
    

    // écriture dans le fichier
    if(status)
        status = writeMultiple(cfileHandle,ecriture,taille_ecriture);
    usleep(wait);

    // ouverture du fichier
    /*if(status)
        status = openFile(namefile,ofileHandle);
    usleep(wait);
    
    // lecture du fichier
    if(status)
        status = readFile(ofileHandle,lecture);
    usleep(wait);*/

    free(cfileHandle);
    //free(ofileHandle);

    printf("Fin de l'execution\n");
    return 0;
}
