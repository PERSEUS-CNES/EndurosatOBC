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

//ce programme a pour but de tester la configuration et le passage au mode "transmit" de l'emetteur
int main(int argc, char * argv[])
{


    if(argc > 1) // ecrire DEBUG_ON en parametre afin de print intégralement les commandes envoyées
    {            // et les réponses
        if(!strcmp(argv[1],"DEBUG_ON"))
        debug = 1;
    }
    // initialisation du port FTDI
    int port = 0;
    int baud = 3000000;
    uint8_t status = 0;//boolen
    uint32_t wait = 5000;

    // configuration de l'emetteur
    struct configuration param;
    param.symbol_rate = 5;
    param.transmit_power = 32; 
    param.MODCOD = 1;
    param.roll_off = 0;
    param.pilot_signal = 1;
    param.FEC_frame_size = 0;
    param.pretransmission_delay = 3000;
    param.center_frequency = 2450.0000;

    //passage en mode idle pour pouvoir tester le passage en mode transmission 
    status = transmit_mode(0);
    usleep(wait);//delai necessaire entre 2 commandes

    // envoi de la config de l'emetteur
    //lors de nos tests nous obtenons une réponse "NO COMMAND FOR EXECUTION" de la part de l'emetteur
    //tant bien meme que la commande à bien été prise en compte
    status = set_emitter_config(&param, all_parameters);
    // pour les tests, un echec de la configuration ne conduit pas a un renvoi de la fonction
    // ceci sera implémenté lors de la version finale

    usleep(10000000); // delai de 10 secondes pour pouvoir observer la réponse

    //passage en mode transmission
    //en cas de succes, on s'attend à ce que l'emmeteur consome entre 0.48 et 0.51 amperes
    //pour la puissance séléctionnée
    //lors de nos tests, l'emetteur de consome que 0.27 amperes, peu importe la puissance séléctionnée
    //cela s'acompagne d'une absence de signal emis lors de l'envoi d'un fichier
    status = transmit_mode(1);
    usleep(10000000); // delai de 10 secondes pour pouvoir observer la réponse

    

    status = transmit_mode(0); // extinction de l'emetteur 




    return 0;
}