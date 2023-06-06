/**
 * @file outils_OBC.c
 * @author Team ENSSAT
 * @brief Fichier contenant tous les outils necessaires pour la communication avec l'OBC
 * @date 2023-02-04
 */

#include "./Librairies/FileDeMessage.h"
#include "./Librairies/VariableGlobale.h"

#include "./Librairies/Structure.h"


#include <errno.h>   // pour errno

/**
 * @brief Méthode qui creer les différentes files de messages
 * 
 * @date 11/03/2023
 * 
 * @param file_de_message -> strurture contenant les identifiants des files de messages
 * 
 * @author Team OBC (ENSSAT)
 *
 */
void CreationFileDeMessage (File_de_message * file_de_message) {

	struct mq_attr attr;
	attr.mq_flags = 0;
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = sizeof(Message);
	attr.mq_curmsgs = 0;

	file_de_message->file_message_Centrale = mq_open("/FileMessageCentrale", O_CREAT | O_RDWR, 0644, &attr);
	file_de_message->file_message_Emetteur = mq_open("/FileMessageEmetteur", O_CREAT | O_RDWR, 0644, &attr);
	file_de_message->file_message_Sauvegarde = mq_open("/FileMessageSauvegarde", O_CREAT | O_RDWR, 0644, &attr);
	file_de_message->file_message_SS = mq_open("/FileMessageSS", O_CREAT | O_RDWR, 0644, &attr);
	file_de_message->file_message_Energie = mq_open("/FileMessageEnergie", O_CREAT | O_RDWR, 0644, &attr);

	if (file_de_message->file_message_Centrale == -1) {
            perror("Erreur lors de la créarion de la file de message centrale: ");
            printf("errno = %d\n", errno);
    }
	if (file_de_message->file_message_Emetteur == -1) {
		perror("Erreur lors de la créarion de la file de message emetteur: ");
		printf("errno = %d\n", errno);
	}
	if (file_de_message->file_message_Sauvegarde == -1) {
		perror("Erreur lors de la créarion de la file de message sauvegarde: ");
		printf("errno = %d\n", errno);
	}
	if (file_de_message->file_message_SS == -1) {
		perror("Erreur lors de la créarion de la file de message SS: ");
		printf("errno = %d\n", errno);
	}
 if (file_de_message->file_message_Energie == -1) {
		perror("Erreur lors de la créarion de la file de message Energie: ");
		printf("errno = %d\n", errno);
	}

 printf("good\n");
	
		
}


/**
 * @brief Méthode d'initialisation de la structure File_de_message
 * 
 * @param F -> strurture contenant les identifiants des files de messages
 * 
 * @date 04/02/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 */
void initialisation_File_de_message(File_de_message * F) {
	F->file_message_Centrale = 0 ;                      //Initialisation de l'ID de la file de message avec la centrale innertielle
	F->file_message_Emetteur = 0 ;                      //Initialisation de l'ID de la file de message avec l'emetteur
	F->file_message_Sauvegarde = 0;                     //Initialisation de l'ID de la file de message avec le fils de sauvegarde
	F->file_message_SS = 0;								//Initialisation de l'ID de la file de message avec le fils SS
 F->file_message_Energie = 0;								//Initialisation de l'ID de la file de message avec le fils Energie
	//F.file_messaage_Roulis = 0 ;                       //Initialisation de l'ID de la file de message avec le systeme de roulis
	//F.file_messaage_Parafoil = 0 ;                     //Initialisation de l'ID de la file de message avec le systeme de parafoil

	F->nb_message_recu = 0 ;							   //Initialise ne nombre de message reçu à 0

}

/**
 * @brief Méthode qui détruit les différentes files de messages
 * 
 * @date 11/03/2023
 * 
 * @param file_de_message -> strurture contenant les identifiants des files de messages
 * 
 * @author Team OBC (ENSSAT)
 *
 */
bool DetruitFileDeMessage (File_de_message * file_de_message) {

	int ReturnValue[5];

	file_de_message->file_message_Centrale = (mqd_t) 0 ;
	ReturnValue[0] = mq_unlink ("/FileMessageCentrale");

	file_de_message->file_message_Emetteur = (mqd_t) 0 ;
	ReturnValue[1] = mq_unlink("/FileMessageEmetteur");

	file_de_message->file_message_Sauvegarde = (mqd_t) 0 ;
	ReturnValue[2] = mq_unlink("/FileMessageSauvegarde");
	
	file_de_message->file_message_SS = (mqd_t) 0 ;
	ReturnValue[3] = mq_unlink("/FileMessageSS");
 
 file_de_message->file_message_Energie = (mqd_t) 0 ;
	ReturnValue[4] = mq_unlink("/FileMessageEnergie");


	if (ReturnValue[0]==0 && ReturnValue[1]==0 && ReturnValue[2]==0 && ReturnValue[3]==0 && ReturnValue[4]==0) {
		return true;
	}
	else {
		return false;
	}
}

/**
 * @brief Méthode qui ferme les différentes files de messages
 * 
 * @date 11/03/2023
 * 
 * @param file_de_message -> strurture contenant les identifiants des files de messages
 * 
 * @author Team OBC (ENSSAT)
 *
 */
bool FermeFileDeMessage (File_de_message * file_de_message) {

	int ReturnValue[5];

	ReturnValue[0] = mq_close (file_de_message->file_message_Centrale);
	file_de_message->file_message_Centrale =(mqd_t) -1 ;

	ReturnValue[1] = mq_close (file_de_message->file_message_Emetteur);
	file_de_message->file_message_Emetteur = (mqd_t) -1 ;

	ReturnValue[2] = mq_close (file_de_message->file_message_Sauvegarde);
	file_de_message->file_message_Sauvegarde = (mqd_t) -1 ;
	
	ReturnValue[3] = mq_close (file_de_message->file_message_SS);
	file_de_message->file_message_SS = (mqd_t) -1 ;
 
 ReturnValue[4] = mq_close (file_de_message->file_message_Energie);
	file_de_message->file_message_Energie = (mqd_t) -1 ;


	if (ReturnValue[0]==0 && ReturnValue[1]==0 && ReturnValue[2]==0 && ReturnValue[3]==0 && ReturnValue[4]==0) {
		return true;
	}
	else {
		return false;
	}
}
