
/**
 * @brief Méthode de connxion aux differents peripheriques. Ici nous avons uniqument la centrale inertiel
 * 
 * @date 04/02/2023
 * 
 * @param foundModules -> strurture contenant les donnés de connexion au différents modules
 * 
 * @author Team OBC (ENSSAT)
 *
 */
void connexion_peripherique(Variables_connexion * foundModules) ;


/**
 * @brief Methode qui ferme tous les fichiers ouvert.
 * 
 * @date 04/02/2023
 * 
 * @param variables_stockage -> strurture contenant les donnés de stcokage
 * 
 * @author Team OBC (ENSSAT)
 *
 */
void fermeture_fichiers(Variables_fichiers variables_stockage) ;


/**
 * @brief Méthode de connxion aux differents peripheriques. Ici nous avons uniqument la centrale inertiel
 * 
 * @date 04/02/2023
 * 
 * @param foundModules -> strurture contenant les donnés de connexion au différents modules
 * 
 * @author Team OBC (ENSSAT)
 *
 */
void connexion_peripherique(Variables_connexion * foundModules) {
	if (1 == 1) { // Connexion à la centrale inertielle
		printf("Le module centrale inertielle a été detecté ! (id = %04X)\n", 1);
		printf("Module centrale inertielle connecté avec succès\n");
		foundModules->Centrale_trouve = true; 
		foundModules->nb_peripheriques++;
	}
	if (1 == 1) { // Connexion au systeme de controle du roulis
		printf("Le module roulis a été detecté ! (id = %04X)\n", 1);
		printf("Module roulis connecté avec succès\n");
		foundModules->Centrale_trouve = true; 
		foundModules->nb_peripheriques++;
	}
	if (1 == 1) { // Connexion au systeme de controle du parafoil
		printf("Le module parafoil a été detecté ! (id = %04X)\n", 1);
		printf("Module parafoil connecté avec succès\n");
		foundModules->Centrale_trouve = true; 
		foundModules->nb_peripheriques++;
	}
}

/**
 * @brief Methode qui ferme tous les fichiers ouvert.
 * 
 * @date 04/02/2023
 * 
 * @param variables_stockage -> strurture contenant les donnés de stcokage
 * 
 * @author Team OBC (ENSSAT)
 *
 */
void fermeture_fichiers(Variables_fichiers variables_stockage) {
		
	if (variables_stockage.file_open[0] == true) {
		fclose(variables_stockage.fichier_donnees_GPS_pos);
	}
	if (variables_stockage.file_open[1] == true) {
		fclose(variables_stockage.fichier_donnees_GPS_vel);
	}
	if (variables_stockage.file_open[2] == true) {
		fclose(variables_stockage.fichier_donnees_IMU);
	}
	if (variables_stockage.file_open[3] == true) {
		fclose(variables_stockage.fichier_donnees_pression);
	}
	if (variables_stockage.file_open[4] == true) {
		fclose(variables_stockage.fichier_donnees_Magnetometers);
	}
	if (variables_stockage.file_open[5] == true) {
		fclose(variables_stockage.fichier_donnees_EKF);
	}
}

/**
 * @brief Methode qui permet la reception des messages de la centrale
 * 
 * @date 14/03/2023
 * 
 * @return message -> message contenu dans la file de message
 * 
 * @author Team OBC (ENSSAT)
 *
 */
char * ReceptionMessageCentrale(char * message) {

	ssize_t ReturnValue = - 1;
	mqd_t mq = FileDeMessage.file_message_Centrale ; 
	unsigned int msg_prio;
	
	ReturnValue = mq_receive (mq, message, sizeof(message),&msg_prio);

	if (ReturnValue==-1) {perror("mq_receive"); return NULL ;}

	
	if (FileDeMessage.nb_message_recu == 0) {
		printf("Le module centrale inertielle a été detecté ! (id = %04X)\n", 1);
		printf("Module centrale inertielle connecté avec succès\n");
		foundModules.Centrale_trouve = true; 
		foundModules.nb_peripheriques++;
		FileDeMessage.nb_message_recu ++ ;
	} 

	else {
		FileDeMessage.nb_message_recu ++ ;
		return message;
	}

	return 0;
}

/**
 * @brief Fonction qui confirme à l'OBC la bonne connexion avec la centrale
 * 
 * @date 14/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - False -> envoie échoué
 *                                                          - True  -> envoie reussi
 *
 */
bool ConnexionCentrale ();

/**
 * @brief Fonction qui confirme à l'OBC la bonne connexion avec la centrale
 * 
 * @date 14/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - False -> envoie échoué
 *                                                          - True  -> envoie reussi
 *
 */
bool ConnexionCentrale () {
    char message[20] = "Connexion Centrale" ;
    
    int envoie = mq_send (FileDeMessage.file_message_Centrale, message, sizeof(message) , 0);
    if (envoie == -1) {
        return false;
    }
    else {
        return true;
    }

}