/**
 * @file struc.h
 * @author Team ENSSAT
 * @brief Fichier contenant toute les strucures necessaires à l'OBC
 * @date 2023-02-04
 */

#ifndef STRUCT_H
#define STRUCT_H

#include <inttypes.h> 
#include <stdio.h> 
#include <stdbool.h>
#include <mqueue.h>

/*! -----------------------------------------------    Les fichiers    -----------------------------------------------*/

/**
 * @brief Structure qui sotck les differents fichiers de stockage.
 * 
 * @date 04/02/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 */
typedef struct variables_fichiers {
	int compteur_fichiers;                //Nombre de fichier ouvert
	bool file_open[7];                    //Tableau avec les etats des fichiers : true = fichier ouvert ; false = fichier ferme
	char nom_fichier[30];                 //Tableau contenant le nom des differents fichiers
	FILE * fichier_donnees_GPS_vel;       //Fichier des donnes du GPS_vel
	FILE * fichier_donnees_GPS_pos;       //Fichier des donnes du GPS_pos
	FILE * fichier_donnees_IMU;	          //Fichier des donnes de l'IMU
	FILE * fichier_donnees_pressure;      //Fichier des donnes de pression
	FILE * fichier_donnees_Magnetometers; //Fichier des donnes du magnetometre
	FILE * fichier_donnees_EKF;           //Fichier des donnes de l'EKF
	FILE * fichier_donnees_EKF_nav;		  //Fichier des donnes de navigation
	FILE * fichier_donnees_CLOCK;         //Fichier des donnes de l'horloge
} variables_fichiers ;
typedef variables_fichiers Variables_fichiers;

/*! -----------------------------------------------    Les connexions    -----------------------------------------------*/

/**
 * @brief Structure qui stock l etat des connexion avec les differents peripheriques.
 * 
 * @date 04/02/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 */
typedef struct variables_connexion {
	bool Centrale_trouve;              //Boolean de connexion avec la central inertiel
	bool Roulis_trouve;                //Boolean de connexion avec le systeme de controle du roulis
	bool Parafoil_trouve;              //Boolean de connexion avec la systeme de parafoil
	int nb_peripheriques;              //Nombre de perfipherique connecte

} variables_connexion ;
typedef variables_connexion Variables_connexion;

/*! -----------------------------------------------    Les donnees    -----------------------------------------------*/

/**
 * @brief Structure qui sotck les donnees du GPS.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct gps_pos {
		double longitude       ;         //Longitude
        double altitude        ;         //Altitude
        double latitude        ;         //Latitude

        float undulation       ;         //Altitude difference between the geoid and the Ellipsoid in meters (Height above Ellipsoid = altitude + undulation)
        float latitudeAccuracy ;         //1 sigma latitude accuracy in meters	
        float longitudeAccuracy;         //1 sigma longitude accuracy in meters
        float altitudeAccuracy ;         //1 sigma altitude accuracy in meters

		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde


} gps_pos ;
typedef  gps_pos  GPS_pos;


/**
 * @brief Structure qui sotck les donnees du GPS.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct ekf_nav {
		double position[3]     ;
		float velocity[3]      ;
		float velocityStdDev[3];
		

        float undulation       ;         //Altitude difference between the geoid and the Ellipsoid in meters (Height above Ellipsoid = altitude + undulation)
        float positionStdDev[3] ;         //1 sigma latitude accuracy in meters	
		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde


} ekf_nav ;
typedef  ekf_nav  EKF_nav;

/**
 * @brief Structure qui sotck les donnees de vélocité du GPS.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct gps_vel {
	
		float velocity[3]      ;         //GPS North, East, Down velocity in m.s^-1
        float velocityAcc[3]   ;         //GPS North, East, Down velocity 1 sigma accuracy in m.s^-1

		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde

}gps_vel;
typedef gps_vel GPS_vel;

/**
 * @brief Structure qui sotck les donnees de l IMU.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct imu {
	float accelerometers[3]  ;        //X, Y, Z accelerometers in m.s^-2
    float gyroscopes[3]	 ;        //X, Y, Z gyroscopes in rad.s^-1
    float temperature        ;        //Internal temperature in °C
    float deltaVelocity[3]   ;        //X, Y, Z delta velocity in m.s^-2
    float deltaAngle[3]	 ;        //X, Y, Z delta angle in rad.s^-1

	unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde


} imu ;
typedef  imu IMU;

/**
 * @brief Structure qui sotck les donnees du Magnetometre.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct magnetometers {
        float magnetometers[3] 	;        //X, Y, Z magnetometer data in A.U
		
		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde


} magnetometers ;
typedef  magnetometers Magnetometers;

/**
 * @brief Structure qui sotck les donnees de Pression.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct pressure {
        float pressure		       ;    //Pressure value in Pascals
        float height		       ;    //Altitude or depth in meters (positive up)

		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde


} pressure ;
typedef  pressure Pressure;

/**
 * @brief Structure qui sotck les donnees de l EKF.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct ekf {
        float euler[3]		        ;	//Roll, Pitch and Yaw angles in rad
		float eulerStdDev[3]		;   //Roll, Pitch and Yaw angles 1 sigma standard deviation in rad
		float quaternion[4]			;	//Orientation quaternion in W, X, Y, Z form

		unsigned int timeStamp      ;   //Temps depuis la mise en focntion de la centrale en micro seconde

} ekf ;
typedef  ekf EKF;

/**
 * @brief Structure qui sotck les donnees de l'horloge.
 * 
 * @date 04/04/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct clock_struct {
        uint16_t year		        ;	//Année
		uint8_t month				;   //Mois
		uint8_t day                 ;   //Jour
		uint8_t hour				;	//Heure
		uint8_t minute				;   //Minute
		uint8_t second				;   //Second
		uint32_t nanoSecond			;   //Nano seconde

} clock_struct ;
typedef  clock_struct CLOCK;

/*! -----------------------------------------------    Les files de messages    -----------------------------------------------*/

/**
 * @brief Structure qui stock le nom des differentes files de message.
 * 
 * @date 04/02/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 */
typedef struct file_de_message {
	mqd_t file_message_Centrale ;         //ID de la file de message pour la communication avec la centrale inertiel
	mqd_t file_message_Emetteur ;         //ID de la file de message pour la communication avec l'emetteur
	mqd_t file_message_Sauvegarde ;       //ID de la file de message pour la communication avec le fils de sauvegarde

	int nb_message_recu ;                 //Nombre de message reçu


} file_de_message ;
typedef file_de_message File_de_message;

/**
 * @brief Énumération des differents type de connexion.
 * 
 * @date 30/03/2023
 * 
 * @author Team OBC (ENSSAT)
 */
typedef enum {
    CENTRALE,
    EMETTEUR,
	SAUVEGARDE,
} PeripheriqueType;

/**
 * @brief Énumération des differents type de donnée.
 * 
 * @date 30/03/2023
 * 
 * @author Team OBC (ENSSAT)
 */
typedef enum {
    GPS_POS_TYPE,
    GPS_VEL_TYPE,
	IMU_TYPE,
	MAGNETOMETERS_TYPE,
	PRESSURE_TYPE,
	EKF_TYPE,
	EKF_NAV_TYPE,
	CLOCK_TYPE
} MessageType;

/**
 * @brief Structure qui serivira à l'envoie des meesgaes.
 * 
 * @date 30/03/2023
 * 
 * @author Team OBC (ENSSAT)
 */
typedef struct {
    MessageType type;    // Type de message
    union {
        GPS_pos gps_pos; 				// Données GPS
        GPS_vel gps_vel; 				// Données de vélocité GPS
		IMU imu ;        				// Données de l'IMU
		Magnetometers magnetometers ;   // Données du magnétomètre
		Pressure pressure; 				// Données de pression
		EKF ekf ; 						// Données de l'EKF
		EKF_nav ekf_nav;					// Données de navigation
		CLOCK clock ;					// Données de l'horloge
    } data;
} Message;

/*! -----------------------------------------------    Les fonctions     -----------------------------------------------*/

                      /*---------------------------    Convertion en string     ---------------------------*/


/**
 * @brief Fonction qui transforme GPS_pos en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param GPS_pos -> Structure contenant les données du GPS_pos
 */
void ToString_data_GPS_pos (GPS_pos GPS_pos) ;


/**
 * @brief Fonction qui transforme GPS_vel en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param GPS_vel -> Structure contenant les données du GPS_vel
 */
void ToString_data_GPS_vel (GPS_vel GPS_vel) ;

/**
 * @brief Fonction qui transforme IMU en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param IMU -> Structure contenant les données du IMU
 */
void ToString_data_IMU (IMU IMU) ;

/**
 * @brief Fonction qui transforme Magnetomètre en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param Magnetometers -> Structure contenant les données du Magnétomètre
 */
void ToString_data_Magnetometers (Magnetometers Magnetometers) ;



/**
 * @brief Fonction qui transforme Pressure en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param Pressure -> Structure contenant les données de Pressionn
 */
void ToString_data_Pressure (Pressure Pressure) ;


/**
 * @brief Fonction qui transforme EKF en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param EKF -> Structure contenant les données de l'EKF
 */
void ToString_data_EKF (EKF EKF) ;

/**
 * @brief Fonction qui transforme CLOCK en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param CLOCK -> Structure contenant les données de l'horloge
 */
void ToString_data_CLOCK (CLOCK CLOCK) ;

void afficher_code_binaire_float(float nombre);


#endif