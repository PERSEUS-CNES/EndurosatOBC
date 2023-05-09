/**
 * @file EnvoieDataFileMessage.h
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant tous les outils necessaires pour la communication entre l'OBC et la centrale inertiel
 * @date 2023-03-11
 */

#ifndef ENVOIEFATAFILEMESSAGE_H 
#define ENVOIEFATAFILEMESSAGE_H

#include <stdio.h>
#include <string.h>

#include "Structure.h"


/**
 * @brief Fonction qui envoie à l'OBC les données du GPS_pos
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param GPS_pos  -> Adresse de la structure contenant les données du GPS_pos
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_GPS_pos (GPS_pos GPS_pos ,unsigned int priorite , PeripheriqueType peripherique) ;


/**
 * @brief Fonction qui envoie à l'OBC les données du GPS_vel
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param GPS_vel  -> Adresse de la structure contenant les données du GPS_vel
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_GPS_vel (GPS_vel GPS_vel , unsigned int priorite , PeripheriqueType peripherique) ;

/** @brief Fonction qui envoie à l'OBC les données de Pression
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param EKF_nav -> Adresse de la structure contenant les données de navigation
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_EKF_nav (EKF_nav EKF_nav , unsigned int priorite , PeripheriqueType peripherique) ;


/**
 * @brief Fonction qui envoie à l'OBC les données de l'IMU
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param IMU -> Adresse de la structure contenant les données de l'IMU
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_IMU (IMU IMU , unsigned int priorite , PeripheriqueType peripherique) ;

/**
 * @brief Fonction qui envoie à l'OBC les données du Magnetometers
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param Magnetometers -> Adresse de la structure contenant les données du Magnetometers
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_Magnetometers (Magnetometers Magnetometers , unsigned int priorite , PeripheriqueType peripherique) ;

 /** @brief Fonction qui envoie à l'OBC les données de Pression
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param Pressure -> Adresse de la structure contenant les données de Pression
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_Pressure (Pressure Pressure , unsigned int priorite, PeripheriqueType peripherique) ;

 /** @brief Fonction qui envoie à l'OBC les données de l'EKF
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 * @param Pressure -> Adresse de la structure contenant les données de l'EKF
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_EKF (EKF EKF , unsigned int priorite , PeripheriqueType peripherique) ;

/**
 * @brief Fonction qui envoie à l'OBC les données du CLOCK
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param CLOCK  -> Adresse de la structure contenant les données du CLOCK
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_CLOCK (CLOCK CLOCK ,unsigned int priorite , PeripheriqueType peripherique) ;

#endif