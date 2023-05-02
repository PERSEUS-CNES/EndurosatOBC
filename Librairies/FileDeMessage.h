/**
 * @file outils_OBC.h
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant tous les outils necessaires pour la communication avec l'OBC
 * @date 2023-02-04
 */

#ifndef FILEDEMESSAGE_H 
#define FILEDEMESSAGE_H

//#include <mqueue.h>
#include "Structure.h"


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
void CreationFileDeMessage (File_de_message * file_de_message) ;

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
void initialisation_File_de_message(File_de_message * F) ;

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
bool DetruitFileDeMessage (File_de_message * file_de_message) ;

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
bool FermeFileDeMessage (File_de_message * file_de_message) ;

#endif