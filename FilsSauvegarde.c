/**
 * @file FilsSauvegarde.c.c
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant code du fils Sauvegarde
 * @date 2023-04-05
 */


#include <inttypes.h> 
#include <stdio.h> 
#include <stdbool.h>
#include <mqueue.h>
#include <stdlib.h>


#include "./Librairies/Structure.h"
#include "./Librairies/FilsSauvegarde.h"
#include "./Librairies/ReceptionDataFileMessage.h"
#include "./Librairies/VariableGlobale.h"



/**
 * @brief Fonction qui ouvre les fichiers de sauvegarde et indique quel fichier c'est correctement ouvert dans la liste file_open
 * 
 * @date 2023-04-05
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param file_struct -> Structure contenant les données sur les fichiers de sauvegarde
 * @param mode -> indique avec quel mode on souhaite ouvrir les fichiers de sauvegarde (normalement toujours "a"), utilise les même mode que fopen
 */


void open_files(Variables_fichiers * file_struct,char* mode){
	
	/* ouvre les fichiers et stocke les pointeurs correspondant dans la strucutre des données de suavegarde */

	file_struct->fichier_donnees_GPS_pos=fopen("./Data/fichier_GPS_pos.txt",mode);
	file_struct->fichier_donnees_GPS_vel=fopen("./Data/fichier_GPS_vel.txt",mode);
	file_struct->fichier_donnees_IMU=fopen("./Data/fichier_IMU.txt",mode);
	file_struct->fichier_donnees_Magnetometers=fopen("./Data/fichier_Magnetometers.txt",mode);
	file_struct->fichier_donnees_pressure=fopen("./Data/fichier_pression.txt",mode);
	file_struct->fichier_donnees_EKF=fopen("./Data/fichier_EKF.txt",mode);
	file_struct->fichier_donnees_EKF_nav=fopen("./Data/fichier_EKF_nav.txt",mode);
	file_struct->fichier_donnees_CLOCK=fopen("./Data/fichier_CLOCK.txt",mode);
 	file_struct->fichier_donnees_ENERGIE=fopen("./Data/fichier_ENERGIE.txt",mode);
	file_struct->fichier_donnees_STATUS=fopen("./Data/fichier_STATUS.txt",mode);

	/* teste si les fichiers ont bien été ouvert */

	if (file_struct->fichier_donnees_GPS_pos!=NULL){
		file_struct->file_open[GPS_POS_TYPE]=true; 				//indique que le fichier est ouvert
		file_struct->compteur_fichiers+=1; 					//compte le nombre de fichier ouvert
	}
	if (file_struct->fichier_donnees_GPS_vel!=NULL){
		file_struct->file_open[GPS_VEL_TYPE]=true;
		file_struct->compteur_fichiers+=1;
	}
	if (file_struct->fichier_donnees_IMU!=NULL){
		file_struct->file_open[IMU_TYPE]=true;
		file_struct->compteur_fichiers+=1;
	}
	if (file_struct->fichier_donnees_Magnetometers!=NULL){
		file_struct->file_open[MAGNETOMETERS_TYPE]=true;
		file_struct->compteur_fichiers+=1;
	}
	if (file_struct->fichier_donnees_pressure!=NULL){
		file_struct->file_open[PRESSURE_TYPE]=true;
		file_struct->compteur_fichiers+=1;
	}
	if (file_struct->fichier_donnees_EKF!=NULL){
		file_struct->file_open[EKF_TYPE]=true;
		file_struct->compteur_fichiers+=1;
	}
	
		if (file_struct->fichier_donnees_EKF_nav!=NULL){
		file_struct->file_open[EKF_NAV_TYPE]=true;
		file_struct->compteur_fichiers+=1;
	}


	if (file_struct->fichier_donnees_CLOCK!=NULL){
		file_struct->file_open[CLOCK_TYPE]=true;
		file_struct->compteur_fichiers+=1;
	}
 if (file_struct->fichier_donnees_ENERGIE!=NULL){
		file_struct->file_open[ENERGIE_TYPE]=true;
		file_struct->compteur_fichiers+=1;
	}
	if (file_struct->fichier_donnees_STATUS!=NULL){
		printf("ouverture fichier status\n");
		file_struct->file_open[STATUS_TYPE]=true;
		file_struct->compteur_fichiers+=1;
	}
	

	if(debug){printf("open\n %d fichier(s) ouvert(s)\n",file_struct->compteur_fichiers);}
}


/**
 * @brief Fonction qui ferme les fichiers de sauvegarde et indique leur fermeture dans la liste file_open
 * 
 * @date 2023-04-05
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param file_struct -> Structure contenant les données sur les fichiers de sauvegarde
 */

void close_files(Variables_fichiers *file_struct){

	/* test si les fichiers sont ouvert et si oui les ferme */
	if(file_struct->file_open[GPS_POS_TYPE]){
		file_struct->file_open[GPS_POS_TYPE]=false; 				// indique que le fichier est fermé
		fclose(file_struct->fichier_donnees_GPS_pos);
		file_struct->compteur_fichiers-=1;
	}

	if(file_struct->file_open[GPS_VEL_TYPE]){
		file_struct->file_open[GPS_VEL_TYPE]=false;
		fclose(file_struct->fichier_donnees_GPS_vel);
		file_struct->compteur_fichiers-=1;
	}

	if(file_struct->file_open[IMU_TYPE]){
		file_struct->file_open[IMU_TYPE]=false;
		fclose(file_struct->fichier_donnees_IMU);
		file_struct->compteur_fichiers-=1;
	}

	if(file_struct->file_open[MAGNETOMETERS_TYPE]){
		file_struct->file_open[MAGNETOMETERS_TYPE]=false;
		fclose(file_struct->fichier_donnees_Magnetometers);
		file_struct->compteur_fichiers-=1;
	}

	if(file_struct->file_open[PRESSURE_TYPE]){
		file_struct->file_open[PRESSURE_TYPE]=false;
		fclose(file_struct->fichier_donnees_pressure);
		file_struct->compteur_fichiers-=1;
	}
	
	if(file_struct->file_open[EKF_TYPE]){
		file_struct->file_open[EKF_TYPE]=false;
		fclose(file_struct->fichier_donnees_EKF);
		file_struct->compteur_fichiers-=1;
	}
	
	if(file_struct->file_open[EKF_NAV_TYPE]){
		file_struct->file_open[EKF_NAV_TYPE]=false;
		fclose(file_struct->fichier_donnees_EKF_nav);
		file_struct->compteur_fichiers-=1;
	}

	if(file_struct->file_open[CLOCK_TYPE]){
		file_struct->file_open[CLOCK_TYPE]=false;
		fclose(file_struct->fichier_donnees_CLOCK);
		file_struct->compteur_fichiers-=1;
	}
 if(file_struct->file_open[ENERGIE_TYPE]){
		file_struct->file_open[ENERGIE_TYPE]=false;
		fclose(file_struct->fichier_donnees_ENERGIE);
		file_struct->compteur_fichiers-=1;
	}
	if(file_struct->file_open[STATUS_TYPE]){
		file_struct->file_open[STATUS_TYPE]=false;
		fclose(file_struct->fichier_donnees_STATUS);
		file_struct->compteur_fichiers-=1;
	}
	if(debug){printf("close\n");}


}


/**
 * @brief Fonction qui ferme puis ouvre les fichiers de sauvegarde, et remet le compteur since_update à 0
 * 
 * @date 2023-04-05
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param file_struct -> Structure contenant les données sur les fichiers de sauvegarde
 * @param since_update -> compte le nombre de donnée sauvegarder entre deux appel de cette fonction
 */

void update_file(Variables_fichiers *file_struct, int* since_update){
	close_files(file_struct);
	if(debug){printf("updated \n");}
	open_files(file_struct,"a");
	*since_update=0; 								// remet à 0 le nombre de sauvegarde depuis la mise à jour des fichiers
}

/**
 * @brief Fonction qui associe un type de message à un fichier de sauvegarde
 * 
 * @date 2023-04-05
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param file_struct -> Structure contenant les données sur les fichiers de sauvegarde
 * @param id -> type du message reçu
 * 
 * @return FILE* -> est un pointeur sur le fichier qui permet de sauvegarder les données du type de id @sim17
 */

FILE* ID2file(Variables_fichiers *file_struct,MessageType id){
	switch (id){ 						// test le type du message transmis
		case GPS_POS_TYPE:
			
			/* si le fichier est ouvert alors son pointeur est transmis */

			if(file_struct->file_open[GPS_POS_TYPE]){
				return file_struct->fichier_donnees_GPS_pos;
			}
			break;

		case GPS_VEL_TYPE:
			if(file_struct->file_open[GPS_VEL_TYPE]){
				return file_struct->fichier_donnees_GPS_vel;
			}
			break;

		case IMU_TYPE:
			if(file_struct->file_open[IMU_TYPE]){
				return file_struct->fichier_donnees_IMU;
			}
			break;

		case MAGNETOMETERS_TYPE:
			if(file_struct->file_open[MAGNETOMETERS_TYPE]){
				return file_struct->fichier_donnees_Magnetometers;
			}
			break;

		case PRESSURE_TYPE:
			if(file_struct->file_open[PRESSURE_TYPE]){
				return file_struct->fichier_donnees_pressure;
			}
			break;

		case EKF_TYPE:
			if(file_struct->file_open[EKF_TYPE]){
				return file_struct->fichier_donnees_EKF;
			}
			break;
		case EKF_NAV_TYPE:
			if(file_struct->file_open[EKF_NAV_TYPE]){
				return file_struct->fichier_donnees_EKF_nav;
			}
			break;
		case CLOCK_TYPE :
			if(file_struct->file_open[CLOCK_TYPE]){
				return file_struct->fichier_donnees_CLOCK;
			}
			break;
      case ENERGIE_TYPE :
			if(file_struct->file_open[ENERGIE_TYPE]){
				return file_struct->fichier_donnees_ENERGIE;
			}
			break;
		case STATUS_TYPE :
			printf("status fichier\n");
			if(file_struct->file_open[STATUS_TYPE]){
				return file_struct->fichier_donnees_STATUS;
			}
			break;
	}

	return NULL; 									// si le type du message n'est pas connue ou que le fichier correspondant est fermé
}

/**
 * @brief Fonction qui sauvegarde les données dans un fichier en fonction du type du message reçu
 * 
 * @date 2023-04-05
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param file -> pointeur vers le fichier dans lequelle il faut sauvegarder
 * @param id -> type du message reçu
 * @param message -> message reçu contenant les données à sauvegarder
 * 
 */


void save(FILE* file,MessageType id,Message message){
	switch (id){ 									// test le type du message à enregistrer

		/* sauvegarde les données contenues dans le message en focntion de son type */

		case GPS_POS_TYPE: ;
			GPS_pos gps_pos =message.data.gps_pos; 				// copie les données contenue dans la partie data du message dans la strucutre du type identifié afin de pouvoir acceder aux valeurs
			fprintf(file,";%f",gps_pos.longitude);
			fprintf(file,";");
			fprintf(file,"%f",gps_pos.altitude);
			fprintf(file,";");
			fprintf(file,"%f",gps_pos.latitude);
			fprintf(file,";");

			fprintf(file,"%f",gps_pos.undulation);
			fprintf(file,";");
			fprintf(file,"%f",gps_pos.latitudeAccuracy);
			fprintf(file,";");
			fprintf(file,"%f",gps_pos.longitudeAccuracy);
			fprintf(file,";");
			fprintf(file,"%f",gps_pos.altitudeAccuracy);
			fprintf(file,";");

			fprintf(file,";%u",gps_pos.timeStamp);

			break;

		case GPS_VEL_TYPE: ;
			GPS_vel gps_vel =message.data.gps_vel;
			fprintf(file,";%f",gps_vel.velocity[0]);
			fprintf(file,";%f",gps_vel.velocity[1]);
			fprintf(file,";%f",gps_vel.velocity[2]);

			fprintf(file,";%f",gps_vel.velocityAcc[0]);
			fprintf(file,";%f",gps_vel.velocityAcc[1]);
			fprintf(file,";%f",gps_vel.velocityAcc[2]);
			
			fprintf(file, ";%u" , gps_vel.timeStamp);

			break;

		case IMU_TYPE: ;
			IMU imu =message.data.imu;
			fprintf(file,";%f",imu.accelerometers[0]);
			fprintf(file,";%f",imu.accelerometers[1]);
			fprintf(file,";%f",imu.accelerometers[2]);

			fprintf(file,";%f",imu.gyroscopes[0]);
			fprintf(file,";%f",imu.gyroscopes[1]);
			fprintf(file,";%f",imu.gyroscopes[2]);

			fprintf(file,";%f",imu.temperature);

			fprintf(file,";%f",imu.deltaVelocity[0]);
			fprintf(file,";%f",imu.deltaVelocity[1]);
			fprintf(file,";%f",imu.deltaVelocity[2]);

			fprintf(file,";%f",imu.deltaAngle[0]);
			fprintf(file,";%f",imu.deltaAngle[1]);
			fprintf(file,";%f",imu.deltaAngle[2]);
			fprintf(file, ";%u" , imu.timeStamp);

			break;

		case MAGNETOMETERS_TYPE: ;
			Magnetometers magnetometers =message.data.magnetometers;
			fprintf(file,";%f",magnetometers.magnetometers[0]);
			fprintf(file,";%f",magnetometers.magnetometers[1]);
			fprintf(file,";%f",magnetometers.magnetometers[2]);
			fprintf(file, ";%u",magnetometers.timeStamp);

			break;

		case PRESSURE_TYPE: ;
			Pressure pressure =message.data.pressure;
			fprintf(file,";%f",pressure.pressure);
			fprintf(file,";%f",pressure.height);
			fprintf(file, ";%u" , pressure.timeStamp);

			break;

		case EKF_TYPE: ;
			EKF ekf =message.data.ekf;

			
			fprintf(file,"%lf",ekf.euler[0]);
			fprintf(file,";");
			fprintf(file,"%lf",ekf.euler[1]);
			fprintf(file,";");
			fprintf(file,"%lf",ekf.euler[2]);
			fprintf(file,";");

			fprintf(file,"%lf",ekf.eulerStdDev[0]);
			fprintf(file,";");
			fprintf(file,"%lf",ekf.eulerStdDev[1]);
			fprintf(file,";");
			fprintf(file,"%lf",ekf.eulerStdDev[2]);
			fprintf(file,";");


			fprintf(file,"%lf",ekf.quaternion[0]);
			fprintf(file,";");
			fprintf(file,"%lf",ekf.quaternion[1]);
			fprintf(file,";");
			fprintf(file,"%lf",ekf.quaternion[2]);
			fprintf(file,";");

			fprintf(file,"%u",ekf.timeStamp);
			fprintf(file,";");
			

			//fwrite(&ekf, sizeof(EKF), 1, file);

			break;
			
		case EKF_NAV_TYPE: ;
			EKF_nav ekf_nav =message.data.ekf_nav;

			
			fprintf(file,"%f",ekf_nav.position[0]);
			fprintf(file,";");
			fprintf(file,"%f",ekf_nav.position[1]);
			fprintf(file,";");
			fprintf(file,"%f",ekf_nav.position[2]);
			fprintf(file,";");

		fprintf(file,"%f",ekf_nav.positionStdDev[0]);
			fprintf(file,";");
			fprintf(file,"%f",ekf_nav.positionStdDev[1]);
			fprintf(file,";");
			fprintf(file,"%f",ekf_nav.positionStdDev[2]);
			fprintf(file,";");


			fprintf(file,"%f",ekf_nav.velocity[0]);
			fprintf(file,";");
			fprintf(file,"%f",ekf_nav.velocity[1]);
			fprintf(file,";");
			fprintf(file,"%f",ekf_nav.velocity[2]);
			fprintf(file,";");
			
			fprintf(file,"%f",ekf_nav.velocityStdDev[0]);
			fprintf(file,";");
			fprintf(file,"%f",ekf_nav.velocityStdDev[1]);
			fprintf(file,";");
			fprintf(file,"%f",ekf_nav.velocityStdDev[2]);
			fprintf(file,";");
			
			fprintf(file,"%f",ekf_nav.undulation);
			fprintf(file,";");
			
			fprintf(file,"%u",ekf_nav.timeStamp);
			fprintf(file,";");
			


			break;
			
		case CLOCK_TYPE : ;
			CLOCK clock = message.data.clock;
			fprintf(file,"/");
			fprintf(file,"%u",clock.day);	
			fprintf(file,"/");
			fprintf(file,"%u",clock.month);
			fprintf(file,"/");
			fprintf(file,"%u",clock.year);
			fprintf(file,"/");
			fprintf(file,"%u",clock.hour);
			fprintf(file,":");
			fprintf(file,"%u",clock.minute);
			fprintf(file,":");
			fprintf(file,"%u",clock.second);
			fprintf(file,":");
			fprintf(file,"%u",clock.nanoSecond);

			//ToString_data_CLOCK(clock);

			break;
      
   case ENERGIE_TYPE : ;
     SYSENERGIE energie = message.data.sysenergie;
			fprintf(file,"/");
			fprintf(file,"%f",energie.tension_batt_1);
      fprintf(file,"/");
			fprintf(file,"%f",energie.tension_batt_2);
      fprintf(file,"/");
			fprintf(file,"%f",energie.courant_batt_1);	
      fprintf(file,"/");
			fprintf(file,"%f",energie.courant_batt_2);			
      fprintf(file,"/");
			fprintf(file,"%f",energie.status_energie);	

			break;
	
	case STATUS_TYPE : ;
		printf("sauvergarde donnee\n");
     STATUS status = message.data.status;
			fprintf(file,"/");
			fprintf(file,"%d",status.Status_Roulis);
      fprintf(file,"/");
			fprintf(file,"%d",status.Status_Parafoil);
      fprintf(file,"/");
			fprintf(file,"%d",status.Status_Servo_1);	
      fprintf(file,"/");
			fprintf(file,"%d",status.Status_Servo_2);			
      fprintf(file,"/");
			fprintf(file,"%d",status.Status_Sequenceur);	

			break;

		
	}
	

}

/**
 * @brief Fonction mère qui gère l'ensemble de la procedure de sauvegarde des données
 * 
 * @date 2023-04-05
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param file -> pointeur vers le fichier dans lequelle il faut sauvegarder
 * @param id -> type du message reçu
 * @param message -> message reçu contenant les données à sauvegarder
 * 
 */

void store_data(Variables_fichiers *file_struct,Message message,int* since_update){
	MessageType id=message.type; 							// recupere le type du message transmis 
	if(debug){printf("id:%d \n",id);}
	FILE* file;
	file=ID2file(file_struct,id);							// identifie le fichier qui contient les données en fonction du type du message reçu

	if(debug){printf("debut save: \n");}

	save(file,id,message);								// sauvegarde les données dans le fichier

	if(debug){printf("fin save \n");}
	*since_update+=1;								// met à jour le compteur de sauvegarde depuis une update
	if(debug){printf("since_update:%d \n",*since_update);}
	if(*since_update>Entries_before_update){update_file(file_struct,since_update);} // met à jour les fichiers si suffisement de données ont été sauvegardé
	

}

void FilsSauvegarde(){
	
	VariablesFichiers.compteur_fichiers=0;
	
	int since_update=0;
	
	open_files(&VariablesFichiers,"a");
	//printf("size of message = %ld \n",sizeof(Message));
	if(debug){printf("compteur_fichier:%d\n",VariablesFichiers.compteur_fichiers);}
	while(1){
		Message  * data = malloc(sizeof(Message));
		//printf("\n data récupérée par sauvegarde");
		*data = receptionSauvegarde() ;
	
		//printf("Message reçu par sauvegarde : type = %d\n" , data->type);

		store_data(&VariablesFichiers,*data,&since_update);
		free(data);
	}
	close_files(&VariablesFichiers);

}