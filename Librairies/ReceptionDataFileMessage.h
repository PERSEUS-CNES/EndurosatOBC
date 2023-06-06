/**
 * @file ReceptionDataFileMessage.h
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant tous les outils necessaires pour la reception des messages de la file de message
 * @date 2023-04-04
 */


#ifndef RECEPTIONDATAFILEMESSAGE_H 
#define RECEPTIONDATAFILEMESSAGE_H

#include "Structure.h"
#include "VariableGlobale.h"

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
Message receptionCentrale(void) ;


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
Message receptionSauvegarde(void);


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
Message receptionSS(void);

Message receptionEnergie(void);


#endif
