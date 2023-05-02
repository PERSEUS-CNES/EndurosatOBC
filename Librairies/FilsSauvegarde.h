/**
 * @file FilsSauvegarde.h
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant le code du fils Sauvegarde
 * @date 2023-02-04
 */

#ifndef FILSSAUVEGARDE_H 
#define FILSSAUVEGARDE_H

#include "Structure.h" 

/**
 * @brief indique si le programme se lancera en mode debug ou non
 **/
#define debug 0


/**
 * @brief nombre de donnée à sauvegarder avant d'update les fichirers
 **/			
#define Entries_before_update 4	

/**
 * @brief Fonction qui sauvegarde sur la carte SD les 
 * 
 * @date 04/04/2023
 * 
 * @param file_struct   -> Strurture contenant les identifiants des fichiers de sauvegarde
 * @param message       -> Message reçu qui contient les data
 * @param since_update  -> Compteur qui compte le nombre de message reçu depuis la dernière update des fichiers
 * 
 * @author Team OBC (ENSSAT)
 *
 */
void FilsSauvegarde();

#endif