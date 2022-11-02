//
//	Programme OBC SERA IV
//	
// 	Cible : Raspberry Pi 4 modèle B+
//	OS : linux preampt rt
//	Date : 24/09/2021
//	Version 2.0
//
//	Description : programme de récupération des données USB et 
//				  de retransmission RF via l'émetteur Xlink.
//

//Déclaration des libraries 
#define _GNU_SOURCE     										// Accès à des fonctions non portables, hors POSIX  
#include "obc.h"							// Déclaration du header du projet
#include "storage_manager.h"
#include "ftdi_manager.h"

//Déclaration des variables de temps 
struct timeval startoum,										// Variable stockant la valeur de la date début du programme
			   endoum;											// Variable stockant la valeur de la date fin du programme	

long long unsigned start_time = 0;

void transfer(long long unsigned int* temps_acquisition) { 

	long long unsigned micros = 0, actuel_ms = 0, start_ms = 0, current_cycle = 0;
		
	actualMetrics.nb_frames_pression = 0;
	actualMetrics.nb_frames_vibration = 0;
	actualMetrics.nb_frames_xsens1 = 0;
	actualMetrics.nb_frames_xsens2 = 0;	
	
	unsigned char* data;
	data = (unsigned char*)malloc(64);
	memset(data, '\0', 64);
	
	int fd = open("/dev/random", O_RDONLY);
	
	gettimeofday(&actualMetrics.start, NULL); // enregistre l'heure actuelle dans la variable start
	start_ms = (actualMetrics.start.tv_sec * SEC_IN_MICRO) + actualMetrics.start.tv_usec;
	
	gettimeofday(&actualMetrics.temps_actuel, NULL);
	actuel_ms = ((actualMetrics.temps_actuel.tv_sec * SEC_IN_MICRO) + actualMetrics.temps_actuel.tv_usec);
	current_cycle =  actuel_ms - start_ms;
	
	while (current_cycle < *temps_acquisition * SEC_IN_MICRO) { // Tant que la duree d'acquisition n'est pas atteinte		
/*		
*		 Fermeture des fichier au bout de xTemps 
*		 On pourrais éventuellement changer notre méthode par un flush de la mémoire simple. 
*		 On conserve alors un fichier unique et on a alors une meilleur visibilité dans le répertoire. 
*/
		if (actuel_ms - fichier_ms > 30 * SEC_IN_MICRO) { // Si le dernier fichier date de plus de 30 secondes;
			gettimeofday(&actualMetrics.temps_fichier, NULL);
			fichier_ms = (actualMetrics.temps_fichier.tv_sec * SEC_IN_MICRO) + actualMetrics.temps_fichier.tv_usec;
			
			variables_stockage.compteur_fichiers++;
			variables_stockage.fichier_annexe = fopen("Data/annexe_fichiers.txt", "a");
			
			if (variables_stockage.file_open == 1) {
				if (foundModules.vibration_trouve == 1) {
					fclose(variables_stockage.fichier_donnees_vibration);
				}
				if (foundModules.Xsens1_trouve == 1) {
					fclose(variables_stockage.fichier_donnees_Xsens1);
				}
				
				if (foundModules.Xsens2_trouve == 1) {
					fclose(variables_stockage.fichier_donnees_Xsens2);
				}
				
				if (foundModules.pression_trouve == 1) {
					fclose(variables_stockage.fichier_donnees_pression);
				}
			}
			if (foundModules.Xsens1_trouve == 1) {
				sprintf(variables_stockage.nom_fichier, "Data/Xsens1_%d.bin", variables_stockage.compteur_fichiers); // On ouvre le fichier Xsens
				variables_stockage.fichier_donnees_Xsens1 = fopen(variables_stockage.nom_fichier, "a");
				fprintf(variables_stockage.fichier_annexe, variables_stockage.nom_fichier);
				fprintf(variables_stockage.fichier_annexe, "\n");
			}
			if (foundModules.Xsens2_trouve == 1) {
				sprintf(variables_stockage.nom_fichier, "Data/Xsens2_%d.bin", variables_stockage.compteur_fichiers); // On ouvre le fichier Xsens
				variables_stockage.fichier_donnees_Xsens2 = fopen(variables_stockage.nom_fichier, "a");
				fprintf(variables_stockage.fichier_annexe, variables_stockage.nom_fichier);
				fprintf(variables_stockage.fichier_annexe, "\n");
			}
			
			if (foundModules.pression_trouve == 1) {
				sprintf(variables_stockage.nom_fichier, "Data/pression_%d.bin", variables_stockage.compteur_fichiers); // On ouvre le fichier pression
				variables_stockage.fichier_donnees_pression = fopen(variables_stockage.nom_fichier, "a");
				fprintf(variables_stockage.fichier_annexe, variables_stockage.nom_fichier);
				fprintf(variables_stockage.fichier_annexe, "\n");
			}
			if (foundModules.vibration_trouve == 1) {
				sprintf(variables_stockage.nom_fichier, "Data/vibration_%d.bin", variables_stockage.compteur_fichiers); // On ouvre le fichier de vibration
				variables_stockage.fichier_donnees_vibration = fopen(variables_stockage.nom_fichier, "a");
				fprintf(variables_stockage.fichier_annexe, variables_stockage.nom_fichier);
				fprintf(variables_stockage.fichier_annexe, "\n");
			}
			fclose(variables_stockage.fichier_annexe);
			variables_stockage.file_open = 1;
		}
		
		// DO STUFF HERE
		// For example MANAGE I/O COMMUNICATION (IMU, Baro, GPS, ...)
		
		//Copy random data to buffer
		read(fd, data, 64);
		//Store buffer in files 
		fwrite(data, 64, 1, variables_stockage.fichier_donnees_Xsens2);
		actualMetrics.nb_frames_xsens2++;	
		fwrite(data, 64, 1, variables_stockage.fichier_donnees_Xsens1);
		actualMetrics.nb_frames_xsens1++;
		fwrite(data, 64, 1, variables_stockage.fichier_donnees_vibration);
		actualMetrics.nb_frames_vibration++;
		fwrite(data, 64, 1, variables_stockage.fichier_donnees_pression);
		actualMetrics.nb_frames_pression++;

		fflush(stdout);
		usleep(SEC_IN_MICRO / 2);
		
		gettimeofday(&actualMetrics.temps_actuel, NULL);
		actuel_ms = ((actualMetrics.temps_actuel.tv_sec * SEC_IN_MICRO) + actualMetrics.temps_actuel.tv_usec);

		current_cycle = actuel_ms - start_ms;
	}

	gettimeofday(&actualMetrics.end, NULL); // On récuppère l'heure de fin d'acquisition
	micros = ((actualMetrics.end.tv_sec * 1000000) + actualMetrics.end.tv_usec) - start_ms;
	printf("%d %d %d %d %.2Lf\n", actualMetrics.nb_frames_pression, actualMetrics.nb_frames_vibration,
								actualMetrics.nb_frames_xsens1,actualMetrics.nb_frames_xsens2, (long double) micros / 1000000.0);
}

void saisie_utilisateur(long long unsigned int* temps_acquisition, int* nb_essais_total) {
	while (*temps_acquisition < 1) {
		// printf("Veuillez saisir le temps prevu d'acquisition en secondes : ");
		// scanf("%llu", temps_acquisition);
		*temps_acquisition = 60;
	}
	while (*nb_essais_total < 1) {
		// printf("Veuillez saisir le nombre d'acquisitions a realiser : ");
		// scanf("%u", nb_essais_total);
		*nb_essais_total = 1000000;
	}
}

void set_thread_param(pthread_attr_t* tattr)
{
	int s;

	/* initialized with default attributes */
	s = pthread_attr_init (tattr);
	if (s != 0)
		handle_error_en(s, "pthread_attr_init");
	
	s = pthread_attr_setdetachstate(tattr, PTHREAD_CREATE_DETACHED);
    if (s != 0)
        handle_error_en(s, "pthread_attr_setdetachstate");
	
	s = pthread_attr_setinheritsched(tattr, PTHREAD_EXPLICIT_SCHED);
	if (s != 0)
		handle_error_en(s, "pthread_attr_setinheritsched");
	
	s = pthread_attr_setschedpolicy(tattr, SCHED_RR);
	if (s != 0)
		handle_error_en(s, "pthread_attr_setschedpolicy");
}

// Fonction principale du programme de l'OBC 
int main()
{
	int nb_essais = 0, nb_essais_total = 0;
	long long unsigned int temps_acquisition = 0;

	int err = 0;												// Booléen indiquant si une erreur a été rencontrée lors du lancement d'un thread
	//Paramètres de priorité du scheduleur pour les threads 
	pthread_attr_t tattr1;	
	// Création d'un attribut de thread
	set_thread_param(&tattr1);									// Appel à la fonction définisant le type de scheduling
	struct sched_param param1;									// Création d'un paramètre de scheduling 
	param1.sched_priority = 5;  								// Définition de la valeur de priorité temps réel pour les threads 
	
	err = pthread_attr_setschedparam (&tattr1, &param1);			// Affectation d'une priorité temps réel au paramètre tattr à partir de la valeur définie précédemment 
    if (err != 0)												// Si l'affectation de la priorité s'est terminé avec une erreur
        handle_error_en(err, "pthread_attr_setschedparam");		// Afficher un message d'erreur dans la oonsole 
		
	//Définition de la priorité du script en priorité temps réel
	struct sched_param sched_p;									// Création d'une structure d'ordonancement temps réel pour le programme 
	sched_p.sched_priority = 5; 								// Affectation d'une priorité temps réel entre 0 et 99
	if(sched_setscheduler(0, SCHED_RR, &sched_p) == -1)			// Affectation d'un ordonancement Round-robin avec le paramètre de priorité défini précédemment si l'opération se passe sans erreur
		perror ("sched_setscheduler \n");							// Sinon le programme se termine via la fonction perror()
	
	//RS485 initialisation
	initialize_FTDI();
	
	//Initialisation de l'heure du lancement du script
	gettimeofday(&startoum, NULL);								// Récupération de la date de lancement du script 
	start_time = ((startoum.tv_sec * SEC_IN_MICRO) + startoum.tv_usec);
	
	// Initialisation des variables locales et globales 

	// On vérifie la connexion aux sous-systèmes (IMU, GPS, Baro, Parafoil, Roulis)
	// Declaration et initilisation des structures
	
	initialisation_foundModules();
	initialisation_actualMetrics();
	initialisation_variables_stockage();
	connexion_peripherique(); 								// On se connecte aux peripheriques	
	if (foundModules.Xsens1_trouve == 0 && foundModules.Xsens2_trouve == 0 
		&& foundModules.pression_trouve == 0 && foundModules.vibration_trouve == 0) {
		printf("aucun module n'a ete reconnu verifiez les connecteurs !\n");
		printf("Fermeture...\n");
	}
	printf("Le programme a maintenant %u peripherique(s) connectes \n", foundModules.nb_peripheriques);
	saisie_utilisateur(&temps_acquisition, &nb_essais_total);
	printf("temps_acquisition = %lli s, nb_essais_total = %i \n", temps_acquisition, nb_essais_total);	
		
	ouverture_initiale_stockage();
	//Boucle principale du programme faisant l'acquisition des données USB
	for (nb_essais = 0; nb_essais < nb_essais_total; nb_essais++) {
		printf("passe %d sur %d\n", nb_essais + 1, nb_essais_total);
		
		//!Transfert des données acquises
		transfer(&temps_acquisition); 
	}
	fermeture_fichiers();
	
	printf("Fermeture sans probleme\n\a");						// Sert a faire un bruit si une console est ouverte pour indiquer la fin de l'acquisition
	
	return(0);													// Termine l'exécution du programme sans erreur avec le code 0 
}
