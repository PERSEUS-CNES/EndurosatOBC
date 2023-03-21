


/**
 * @file struc.h
 * @author Team ENSSAT
 * @brief Fichier contenant toute les strucures necessaires à l'OBC
 * @date 2023-02-04
 */

//#include <inttypes.h> 
//#include <stdio.h> 
//#include <stdbool.h>
//#include <mqueue.h>
#include "sbgEComLib.h"
#include <pthread.h>

/*! -----------------------------------------------    Les fichiers    -----------------------------------------------*/


/*-------------------Initialisation des fonctions utilisées dans centrale.c ---------------------------------------*/

unsigned char main_centrale();
void * envoie_message(void *arg);
unsigned char test_message(unsigned char msg);


/**
 * @brief Structure qui sotck les differents fichiers de stockage.
 * 
 * @date 04/02/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 */
typedef struct Variables_fichiers {
	int compteur_fichiers;                //Nombre de fichier ouvert
	bool file_open[5];                    //Tableau avec les etats des fichiers : true = fichier ouvert ; false = fichier ferme
	char nom_fichier[30];                 //Tableau contenant le nom des differents fichiers
	FILE * fichier_donnees_GPS;           //Fichier des donnes du GPS
	FILE * fichier_donnees_IMU;	          //Fichier des donnes de l'IMU
	FILE * fichier_donnees_pression;      //Fichier des donnes de pression
	FILE * fichier_donnees_Magnetometers; //Fichier des donnes du magnetometre
	FILE * fichier_donnees_EKF;           //Fichier des donnes de l'EKF
} Variables_fichiers ;
typedef Variables_fichiers Variables_fichiers;

/*! -----------------------------------------------    Les connexions    -----------------------------------------------*/

/**
 * @brief Structure qui stock l etat des connexion avec les differents peripheriques.
 * 
 * @date 04/02/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 */
typedef struct Variables_connexion {
	bool Centrale_trouve;              //Boolean de connexion avec la central inertiel
	bool Roulis_trouve;                //Boolean de connexion avec le systeme de controle du roulis
	bool Parafoil_trouve;              //Boolean de connexion avec la systeme de parafoil
	int nb_peripheriques;              //Nombre de perfipherique connecte

} Variables_connexion ;
typedef Variables_connexion Variables_connexion;

/*! -----------------------------------------------    Les files de messages    -----------------------------------------------*/

/**
 * @brief Structure qui stock le nom des differentes files de message.
 * 
 * @date 04/02/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 */
/*typedef struct File_de_message {
	mqd_t file_message_Centrale ;         //ID de la file de message pour la communication avec la centrale inertiel
	mqd_t file_message_Emetteur ;         //ID de la file de message pour la communication avec l'emetteur
	mqd_t file_message_Sauvegarde ;       //ID de la file de message pour la communication avec le fils de sauvegarde
	mqd_t file_message_Roulis;            //ID de la file de message pour la communication avec le systeme de control de roulis
	mqd_t file_message_Parafoil ;         //ID de la file de message pour la communication avec le systeme parafoil


} File_de_message ;
typedef File_de_message File_de_message;*/

/*! -----------------------------------------------    Les donnes    -----------------------------------------------*/

/**
 * @brief Structure qui sotck les donnees du GPS.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct GPS_pos {
		double longitude       ;         //Longitude
        double altitude        ;         //Altitude
        double latitude        ;         //Latitude

        float undulation       ;         //Altitude difference between the geoid and the Ellipsoid in meters (Height above Ellipsoid = altitude + undulation)
        float latitudeAccuracy ;         //1 sigma latitude accuracy in meters	
        float longitudeAccuracy;         //1 sigma longitude accuracy in meters
        float altitudeAccuracy ;         //1 sigma altitude accuracy in meters

		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde


} GPS_pos ;
typedef  GPS_pos  GPS_pos;

/**
 * @brief Structure qui sotck les donnees de vélocité du GPS.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct GPS_vel {
	
		float velocity[3]      ;         //GPS North, East, Down velocity in m.s^-1
        float velocityAcc[3]   ;         //GPS North, East, Down velocity 1 sigma accuracy in m.s^-1

		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde

}GPS_vel;
typedef GPS_vel GPS_vel;

/**
 * @brief Structure qui sotck les donnees de l IMU.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct IMU {
	float accelerometers[3]  ;        //X, Y, Z accelerometers in m.s^-2
        float gyroscopes[3]	 ;        //X, Y, Z gyroscopes in rad.s^-1
        float temperature        ;        //Internal temperature in °C
        float deltaVelocity[3]   ;        //X, Y, Z delta velocity in m.s^-2
        float deltaAngle[3]	 ;        //X, Y, Z delta angle in rad.s^-1

		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde


} IMU ;
typedef  IMU IMU;

/**
 * @brief Structure qui sotck les donnees du Magnetometre.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct Magnetometers {
        float magnetometers[3] 	;        //X, Y, Z magnetometer data in A.U
		
		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde


} Magnetometers ;
typedef  Magnetometers Magnetometers;

/**
 * @brief Structure qui sotck les donnees de Pression.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle & OBC (ENSSAT)
 *
 */
typedef struct Pressure {
        float pressure		       ;    //Pressure value in Pascals
        float height		       ;    //Altitude or depth in meters (positive up)

		unsigned int timeStamp ;         //Temps depuis la mise en focntion de la centrale en micro seconde


} Pressure ;
typedef  Pressure Pressure;

/**
 * @brief Structure qui sotck les donnees des angles d'euler de l'EKF.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle  & OBC (ENSSAT)
 *
 */

typedef struct EKF_Euler
{
  float euler[3];
  float eulerStdDev[3];
  unsigned int timeStamp;
}EKF_Euler;


/**
 * @brief Structure qui sotck les donnees des angles de Quaternion de l'EKF.
 * 
 * @date 04/02/2023
 * 
 * @author Team Centrale inertielle  & OBC (ENSSAT)
 *
 */
typedef struct EKF_Quat
{
  float quaternion[4];
  unsigned int timeStamp;
}EKF_Quat;

typedef struct Clock
{
  unsigned short year;
  unsigned char  month;
  unsigned char day;
  unsigned char hour;
  unsigned char minute;
  unsigned char second;
  unsigned long int nanoSeconds;
}Clock;

extern struct Clock SBGClock;
extern struct EKF_Euler SBGEKF_Euler;
extern struct Pressure  SBGPressure;
extern struct IMU  SBGIMU;
extern struct GPS_vel  SBGGPS_vel;
extern struct GPS_pos  SBGGPS_pos;
extern struct EKF_Quat SBGEKF_Quat;
extern struct Magnetometers SBGMagnetometer;