/**
 * @brief Fonction d'initialisation du thread de communication avec l'obc, initialisation de la communication avec la centrale.
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 */

unsigned char FilsCentrale();




/**
 * @brief Fonction du thread pour envoyer les donn�es vers l'obc
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * @param -> arg potentiel argument pour la communication par file de message
 */
 
void * envoie_message();




/**
 * @brief Fonction test pour ajouter des valeurs � transmettre (dans le cas o� on a pas la centrale)
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * @param ->	msg										Message ID of the log received.
 * @return												SBG_NO_ERROR if the received log has been used successfully.
 */
 
unsigned char test_message(unsigned char msg);