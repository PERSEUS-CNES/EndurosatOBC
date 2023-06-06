/**
 * @file ReceptionDataFileMessage.c
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant tous les outils necessaires pour la reception des messages de la file de message
 * @date 2023-04-04
 */



#include "./Librairies/Structure.h"
#include "./Librairies/VariableGlobale.h"
#include "./Librairies/ReceptionDataFileMessage.h"

#include <errno.h>   // pour errno
#include <stdlib.h>	 /* exit */



/**
 * @brief Fonction qui reçois les données de la centrale inertiel
 * 
 * @date 04/04/2023
 * 
 * @return message -> Message reçu de la centrale inertiel
 *
 * @author Team OBC (ENSSAT)
 *
 */
Message receptionCentrale(void) {

    Message message;
    ssize_t recv_size = mq_receive(FileDeMessage.file_message_Centrale, (char*) &message, sizeof(Message), NULL);
    printf("message centrale\n");

    if (recv_size == -1) {
        perror("Erreur lors de la réception du message : ");
        printf("errno = %d\n", errno);
    }

    struct mq_attr attr;
    if (mq_getattr(FileDeMessage.file_message_Centrale, &attr) == -1) {
        perror("mq_getattr");
        exit(EXIT_FAILURE);
    }
           
    return message;

}

/**
 * @brief Fonction qui reçois les données du fils transmission
 * 
 * @date 04/04/2023
 * 
 * @return message -> Message reçu de la centrale inertiel
 *
 * @author Team OBC (ENSSAT)
 *
 */
Message receptionSauvegarde(void) {

    Message message;
	//printf("\n Reception file sauvegarde\n");
    ssize_t recv_size = mq_receive(FileDeMessage.file_message_Sauvegarde, (char*) &message, sizeof(Message), NULL);

    if (recv_size == -1) {
        perror("Erreur lors de la réception du message : ");
        printf("errno = %d\n", errno);
    }

    struct mq_attr attr;
    if (mq_getattr(FileDeMessage.file_message_Sauvegarde, &attr) == -1) {
        perror("mq_getattr");
        exit(EXIT_FAILURE);
    }
	//printf("\n reception file sauvegarde finie\n");
           
    return message ;

}

Message receptionEmetteur(void) {

    Message message;
	//printf("\n Reception file Emetteur \n");
    ssize_t recv_size = mq_receive(FileDeMessage.file_message_Emetteur, (char*) &message, sizeof(Message), NULL);

    if (recv_size == -1) {
        perror("Erreur lors de la réception du message : ");
        printf("errno = %d\n", errno);
    }

    struct mq_attr attr;
    if (mq_getattr(FileDeMessage.file_message_Emetteur, &attr) == -1) {
        perror("mq_getattr");
        exit(EXIT_FAILURE);
    }
	//printf("\n reception file Emetteur finie\n");
           
    return message ;

}

Message receptionSS(void) {

    Message message;
	 printf("\n Reception file SS \n");
    ssize_t recv_size = mq_receive(FileDeMessage.file_message_SS, (char*) &message, sizeof(Message), NULL);
    if (recv_size == -1) {
        perror("Erreur lors de la réception du message : ");
        printf("errno = %d\n", errno);
    }
    struct mq_attr attr;
    if (mq_getattr(FileDeMessage.file_message_SS, &attr) == -1) {
        perror("mq_getattr");
        exit(EXIT_FAILURE);
    }
	 printf("\n reception file SS finie\n");
           
    return message ;

}

Message receptionEnergie(void) {

    Message message;
	 printf("\n Reception file SS \n");
    ssize_t recv_size = mq_receive(FileDeMessage.file_message_Energie, (char*) &message, sizeof(Message), NULL);
    if (recv_size == -1) {
        perror("Erreur lors de la réception du message : ");
        printf("errno = %d\n", errno);
    }
    struct mq_attr attr;
    if (mq_getattr(FileDeMessage.file_message_Energie, &attr) == -1) {
        perror("mq_getattr");
        exit(EXIT_FAILURE);
    }
	 printf("\n reception file Energie finie\n");
           
    return message ;

}

Message receptionStatus(void) {

    Message message;
	 printf("\n Reception file Status \n");
    ssize_t recv_size = mq_receive(FileDeMessage.file_message_Status, (char*) &message, sizeof(Message), NULL);
    if (recv_size == -1) {
        perror("Erreur lors de la réception du message : ");
        printf("errno = %d\n", errno);
    }
    struct mq_attr attr;
    if (mq_getattr(FileDeMessage.file_message_Status, &attr) == -1) {
        perror("mq_getattr");
        exit(EXIT_FAILURE);
    }
	 printf("\n reception file Status finie\n");
           
    return message ;

}
