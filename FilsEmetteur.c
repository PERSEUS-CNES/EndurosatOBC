#include "ftd2xx.h"
#include "es_crc32.h"

#include "emitter_config.h"
#include "emitter_reading.h"
#include "emitter_sending.h"
#include "emitter_writing.h"

uint8_t debug = 0;

int FilsEmetteur()
{

    // initialisation du port FTDI
    int port = 0;
    int baud = 3000000;
    uint8_t status = 0;//boolen
    uint32_t wait = 5000;
	
	
    
    FT_STATUS status_ft = initialize_FTDI(baud, port);
	
    // configuration de l'emetteur
    struct configuration param;
    param.symbol_rate = 5;
    param.transmit_power = 27; 
    param.MODCOD = 1;
    param.roll_off = 0;
    param.pilot_signal = 1;
    param.FEC_frame_size = 0;
    param.pretransmission_delay = 3000;
    param.center_frequency = 2450.0000;


    //passage en mode idle pour pouvoir tester le passage en mode transmission 
    status = transmit_mode(0);
	//exit(0);
    //usleep(wait);//delai necessaire entre 2 commandes

    // envoi de la config de l'emetteur
    //lors de nos tests nous obtenons une réponse "NO COMMAND FOR EXECUTION" de la part de l'emetteur
    //tant bien meme que la commande à bien été prise en compte
    status = set_emitter_config(&param, all_parameters);
    // pour les tests, un echec de la configuration ne conduit pas a un renvoi de la fonction
    // ceci sera implémenté lors de la version finale
	status = get_emitter_config(&param, all_parameters);
	//exit(0);
    //usleep(10000000); // delai de 10 secondes pour pouvoir observer la réponse

    //passage en mode transmission
    //en cas de succes, on s'attend à ce que l'emmeteur consome entre 0.48 et 0.51 amperes
    //pour la puissance séléctionnée
    //lors de nos tests, l'emetteur de consome que 0.27 amperes, peu importe la puissance séléctionnée
    //cela s'acompagne d'une absence de signal emis lors de l'envoi d'un fichier
  	
	status = transmit_mode(1);
	//exit(0);
    //usleep(10000000); // delai de 10 secondes pour pouvoir observer la réponse
	//exit(0);
    // creation du fichier à envoyer
	// suppression de tout les fichiers

    char file_name[] = "test_file.txt";
	uint8_t * fileHandle = malloc(sizeof(uint8_t)*4);
	//exit(0);
	//status = openFile(file_name, fileHandle);
while(1)
{
	
	Message  * data = malloc(sizeof(Message));
	printf("\n data récupérée par sauvegarde");
	*data = receptionEmetteur() ;
	
    status = deleteAllFiles();
	printf("1\n");
    uint32_t fileSize = 1000000; // fichier de 100 MB
	printf("4\n");
    //creation du fichier
    status = createFile(file_name,fileSize, fileHandle);

    //ecriture dans le fichier
    if(status)
    {
        //répartit le buffer en plusieurs buffers qui seront envoyés
        //un par un dans le fichier
        status = writeMultiple(fileHandle,data,fileSize);
    }
	//exit(0);
	printf("!!!!!!!!!!!!!\n");
    if(status)
    {
        status = sendFile(file_name);
    }
	//usleep(10000000); // delai de 10 secondes pour pouvoir observer la réponse
}
    printf("..........................\n");
    status = transmit_mode(0); // extinction de l'emetteur 




    return 0;
}