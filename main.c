#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>


#include "emitterV1.h"




int main(int argc, char * argv[])
{
    /*uint8_t newTestCreateFile [] = {0x45, 0x53, 0x55 , 0x50 , 0x07, 0x20, 0x0E, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00,  0x00,  0x4E, 0x65, 0x77, 0x5F, 0x46, 0x69, 0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    uint8_t taille = sizeof(newTestCreateFile);
    printf(" taille avant fonction %d \n", (int)taille);
    send_command_request(newTestCreateFile,28,taille);*/
    
   /* uint8_t newTestCreateFile [] = {0x45, 0x53, 0x55 , 0x50 , 0x06, 0x20, 0x0E, 0x00, 0x00, 0x00, 0x06, 0x01, 0x00,  0x00,  0x4E, 0x65, 0x77, 0x5F, 0x46, 0x69, 0x6C, 0x65, 0x2E, 0x74, 0x78, 0x74, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00 , 0x00 , 0x00 , 0x00 , 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    unsigned int crc=crc32(0,newTestCreateFile,28);
	printf("\n  crc value  %.4X \n",crc);
	memcpy(newTestCreateFile+28,&crc,sizeof(uint32_t));
    printf("commande : ");
    for(int i = 0; i < 48; i++)
    {
        printf("%.2X " ,(int)newTestCreateFile[i]);
    }
    printf("\n");*/
    
    int port = 0;//atoi(argv[1]);
    int baud = 3000000;//atoi(argv[2]);
    uint8_t status = 0;//boolen
    uint32_t wait = 5000;
    
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

    uint16_t tailleFichier = 256;
    char lecture[tailleFichier];

    // création du fichier
    if(status)
        status = createFile(namefile,cfileHandle);
    usleep(wait);
 
    // écriture dans le fichier
    if(status)
        status = writeInFile(cfileHandle,ecrire,0);
    usleep(wait);

    

    // ouverture du fichier
    if(status)
        status = openFile(namefile,ofileHandle);
    usleep(wait);
    
    // lecture du fichier
    if(status)
        status = readFile(ofileHandle,lecture);
    usleep(wait);

    free(cfileHandle);
    free(ofileHandle);


    return 0;
}
