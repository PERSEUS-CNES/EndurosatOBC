#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>


#include "emitterV1.h"




int main(int argc, char * argv[])
{
    int port = 0;//atoi(argv[1]);
    int baud = 3000000;//atoi(argv[2]);
    uint8_t status = 0;//boolen
    uint32_t wait = 300000;
    
    FT_STATUS status_ft = initialize_FTDI(baud, port);
    // suppression de tout les fichiers
    status = deleteAllFiles();
    usleep(wait);
    char namefile[] = "New_File.txt";
    char ecrire[] = "Write_in_File";
    char * cfileHandle;
    cfileHandle = malloc(sizeof(uint8_t)*4);
    char * ofileHandle;
    ofileHandle = malloc(sizeof(uint8_t)*4);

    // création du fichier
    if(status)
        status = createFile(namefile,cfileHandle);
    usleep(wait);
 
    // écriture dans le fichier
    if(status)
        status = writeInFile(cfileHandle,ecrire,0);
    usleep(wait);

    uint16_t tailleFichier = 256;

    // ouverture du fichier
    if(status)
        status = openFile(namefile,ofileHandle);
    usleep(wait);
    
    // lecture du fichier
    char lecture[tailleFichier];
    if(status)
        status = readFile(ofileHandle,lecture);
    usleep(wait);

    free(cfileHandle);
    free(ofileHandle);

    

    return 0;
}
