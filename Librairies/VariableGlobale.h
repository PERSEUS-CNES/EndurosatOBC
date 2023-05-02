/**
 * @file VariableGlobale.h
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant toute les varriables globales necessaires à l'OBC
 * @date 2023-03-14
 */

#ifndef VARIABLE_GLOBALE_H
#define VARIABLE_GLOBALE_H

#include "Structure.h"

/*! -----------------------------------------------    Les variables globale     -----------------------------------------------*/

/**
 * @author Team OBC (ENSSAT)
 * @brief Variable qui stocke les id des files de messages
 * @date 2023-03-14
 */
extern File_de_message FileDeMessage ; 

/**
 * @author Team OBC (ENSSAT)
 * @brief Variable qui stocke les états de connexion aux autres périphériques
 * @date 2023-03-14
 */
extern Variables_connexion foundModules ;

/**
 * @author Team OBC (ENSSAT)
 * @brief Variable qui stocke les id des fichiers de stockage
 * @date 2023-03-14
 */
extern Variables_fichiers VariablesFichiers ;

/*! -----------------------------------------------    Les fonctions     -----------------------------------------------*/

                      /*---------------------------    Innitialisation des Variables Globale     ---------------------------*/


/**
 * @brief Méthode d'initialisation de la structure Variables_connexion
 * 
 * @date 04/02/2023
 * 
 * @param foundModules -> strurture contenant les donnés de connexion au différents modules
 * 
 * @author Team OBC (ENSSAT)
 *
 */
void initialisation_Variables_connexion(Variables_connexion * foundModules) ;



#endif