//gcc -O0 -g -W -Wall -Wextra -Wconversion -Werror -mtune=native  -march=native  -std=c99  EnvoieDataFileMessage.c FileDeMessage.c FilsSauvegarde.c FilsTransmission.c ReceptionDataFileMessage.c Structure.c VariableGlobale.c test.c -o ./bin/test

#include "./Librairies/Structure.h"
#include "./Librairies/VariableGlobale.h"


#include "./Librairies/EnvoieDataFileMessage.h"
#include "./Librairies/FileDeMessage.h"
#include "./Librairies/ReceptionDataFileMessage.h"

#include "./Librairies/FilsSauvegarde.h"
#include "./Librairies/FilsTransmission.h"
#include "./Librairies/FilsCentrale.h"
#include "./Librairies/FilsSS.h"

#include <stdio.h>		/* stderr, stdout, fprintf, perror */
#include <unistd.h>		/* fork */
#include <sys/wait.h>	/* wait */
#include <stdlib.h>		/* exit */
#include <errno.h>   // pour errno


void envoie();

/**
 * @author Team OBC (ENSSAT)
 * @brief Fonction principale de l'OBC : Creation des differents fils (Transmission , Sauvegarde , Centrale inertielle)
 * @date 2023-02-14
 */
int main(void)
{
	remove("./Data/fichier_pression.txt");
	remove("./Data/fichier_Magnetometers.txt");
	remove("./Data/fichier_IMU.txt");
	remove("./Data/fichier_GPS_pos.txt");
	remove("./Data/fichier_GPS_vel.txt");
	remove("./Data/fichier_EKF.txt");
	remove("./Data/fichier_CLOCK.txt");
 remove("./Data/fichier_sysenergie.txt");
	//Création tableau ID des différents fils
	pid_t pid_fils[6] = {-1,-1,-1,-1,-1, -1} ;

	//Création & initialisation File_de_message
	//initialisation_File_de_message(&FileDeMessage);
    DetruitFileDeMessage (&FileDeMessage) ;
	CreationFileDeMessage(&FileDeMessage);
	printf("Création des Files de message : OK\n");


	//Création des fils
	for (int i = 0 ; i < 6 ; i ++ ) {
		pid_fils[i] = fork();
		if (pid_fils[i]==-1) {
			perror("Echec du fork");
			exit(EXIT_FAILURE);
		}

		if (pid_fils[i]==0) {
			break;
		}
	}

	//Code du fils n°1 : le centrale
	if (pid_fils[0] == 0) {
		printf("\nFils n°1 (centrale) : %d\n" , getpid());
		FilsCentrale();
        DetruitFileDeMessage (&FileDeMessage) ;
        printf("\nFils n°1 (centrale): J'ai fini !\n");
		exit(EXIT_SUCCESS);
	}

	//Code du fils n°2 : la transmission de donnees
	if (pid_fils[1] == 0) {
		printf("Fils n°2 (transmission): %d\n" , getpid());

        FilsTransmission();

        printf("Fils n°2 (transmission) : J'ai fini !\n");
		exit(EXIT_SUCCESS);
	}

    //Code du fils n°3 : la sauvegarde de donnees sur carte SD
    if (pid_fils[2] == 0) {
        printf("Fils n°3 (sauvegarde) : %d\n" , getpid());
        FilsSauvegarde();
        printf("Fils n°3 (sauvegarde) : J'ai fini !\n");
        exit(EXIT_SUCCESS);
	}

    //Code du fils n°4 : l'emetteur
	if (pid_fils[3] == 0) {
		printf("Fils n°4 (émetteur) : %d\n" , getpid());
        printf("Fils n°1 (émetteur): J'ai fini !\n");
		exit(EXIT_SUCCESS);
	}
	
	//Code du fils n°5 : l'envoi aux sous systèmes
	if (pid_fils[4] == 0) {
		printf("Fils n°5 (SS) : %d\n" , getpid());
    FilsSS();
    printf("Fils n°5 (SS): J'ai fini !\n");
		exit(EXIT_SUCCESS);
	}
 
 //Code du fils n°6 : l'envoi aux sous systèmes
	if (pid_fils[5] == 0) {
		printf("Fils n°6 (Energie) : %d\n" , getpid());
    FilsEnergie();
    printf("Fils n°6 (Energie): J'ai fini !\n");
		exit(EXIT_SUCCESS);
	}



	wait(NULL);
	wait(NULL);
	wait(NULL);
    wait(NULL);
	wait(NULL);
 wait(NULL);

    printf("j'ai fini ! (père)\n");

	exit(EXIT_SUCCESS);
}


void envoie()
{
    GPS_pos test ;

    test.longitude = (double) 45678.5678;
    test.altitude = (double) 567;
    test.latitude = (double) 5678.78;
    test.undulation = (float) 56780.56789;
    test.latitudeAccuracy = (float) 5678.678;
    test.longitudeAccuracy = (float) 3456789.098765;
    test.altitudeAccuracy = (float) 567.78;
    test.timeStamp = (unsigned int) 34567;

    Envoie_data_GPS_pos(test ,1 , CENTRALE);

    
    
    GPS_vel test1 ;
    test1.velocity[0] = (float) 5678.678;
    test1.velocity[1] = (float) 4567.678;
    test1.velocity[2] = (float) 98776.678;

    test1.velocityAcc[0] = (float) -34534.678;
    test1.velocityAcc[1] = (float) 345698.678;
    test1.velocityAcc[2] = (float) 09876.678;
    test1.timeStamp = (unsigned int) 34567;

    Envoie_data_GPS_vel(test1 , 2, CENTRALE);

    IMU test2 ;
    test2.accelerometers[0] = (float) 5678.678;
    test2.accelerometers[1] = (float) 4567.678;
    test2.accelerometers[2] = (float) 9076.678;

    test2.gyroscopes[0] = (float) -34534.678;
    test2.gyroscopes[1] = (float) 3698.678;
    test2.gyroscopes[2] = (float) 976.678;

    test2.temperature = (float) 34567;

    test2.deltaVelocity[0] = (float) -5678.678;
    test2.deltaVelocity[1] = (float) 4567.678;
    test2.deltaVelocity[2] = (float) 976.678;

    test2.deltaAngle[0] = (float) 3434.678;
    test2.deltaAngle[1] = (float) 3458.678;
    test2.deltaAngle[2] = (float) -9876.678;

    test2.timeStamp = (unsigned int) 34567;

    Envoie_data_IMU(test2 , 3, CENTRALE);
    //printf("Sizeof(IMU) : %ld\n" , sizeof(test2) );


    Magnetometers test3 ;

    test3.magnetometers[0] = (float) 45678.5678;
    test3.magnetometers[1] = (float) 87659.90876;
    test3.magnetometers[2] = (float) 745678.9876;

    test3.timeStamp = (unsigned int) 3450;

    Envoie_data_Magnetometers(test3 , 4, CENTRALE);

    Pressure test4 ;

    test4.pressure = (float) 34567.98765 ;
    test4.height = (float) 54678.8765;

    test4.timeStamp = (unsigned int) 34560;

    Envoie_data_Pressure(test4, 1, CENTRALE);

    EKF test5 ;

    test5.euler[0] = (float) 4568.5678;
    test5.euler[1] = (float) 87659.90876;
    test5.euler[2] = (float) 745678.9876;

    test5.eulerStdDev[0] = (float) 456789.9876;
    test5.eulerStdDev[1] = (float) 456789.9876;
    test5.eulerStdDev[2] = (float) 456789.9876;

    test5.quaternion[0] = (float) 45678.5678;
    test5.quaternion[1] = (float) 87659.90876;
    test5.quaternion[2] = (float) 745678.9876;

    test5.timeStamp = (unsigned int) 34678;

    Envoie_data_EKF(test5,5, CENTRALE);

    CLOCK test6 ;

    test6.year= (uint16_t) 1998 ;
    test6.month = (uint8_t) 6 ;
    test6.day = (uint8_t) 24 ;
    test6.hour = (uint8_t) 9 ;
    test6.minute = (uint8_t) 43 ;
    test6.second = (uint8_t) 28 ;
    test6.nanoSecond = (uint32_t) 46 ;

    Envoie_data_CLOCK(test6,5, CENTRALE);

    printf("Envoie fini !\n");


}