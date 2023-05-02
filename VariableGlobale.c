/**
 * @file VariableGlobale.c
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant toute les varriables globales necessaires à l'OBC
 * @date 2023-03-31
 */

#include "./Librairies/VariableGlobale.h"

/*! -----------------------------------------------    Les variables globale     -----------------------------------------------*/

File_de_message FileDeMessage ; 

Variables_connexion foundModules ;

Variables_fichiers VariablesFichiers ;


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
void initialisation_Variables_connexion(Variables_connexion * foundModules) {
	foundModules->Centrale_trouve = false;             //Mise a false de la variable de connexion avec la centrale inertielle
	foundModules->Parafoil_trouve = false;             //Mise a false de la variable de connexion avec le systeme de controle du parafoil
	foundModules->Roulis_trouve = false;               //Mise a false de la variable de connexion avec le systeme de controle du roulis
	foundModules->nb_peripheriques = 0;                //Mise a 0 du nombre de pripheriques connectes
}

