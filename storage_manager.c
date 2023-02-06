#include "storage_manager.h"								// USB acquisition 

long long unsigned 	fichier_ms = 0;								// Temps ouverture fichier de stockage en microsecondes

struct storageMetrics actualMetrics;
struct variables_fichiers variables_stockage;
struct variables_connexion foundModules;

void initialisation_foundModules() {
	foundModules.Xsens1_trouve = 0;
	foundModules.Xsens2_trouve = 0;
	//foundModules->Accoust_trouve = 0;
	foundModules.pression_trouve = 0;
	foundModules.vibration_trouve = 0;
	foundModules.nb_peripheriques = 0;
}

void initialisation_actualMetrics() {
	actualMetrics.nb_frames_pression = 0;
	actualMetrics.nb_frames_vibration = 0;
	actualMetrics.nb_frames_xsens1 = 0;
	actualMetrics.nb_frames_xsens2 = 0;
	actualMetrics.verbose_mode = 0;
}

void initialisation_variables_stockage() {
	variables_stockage.compteur_fichiers = 1;
	variables_stockage.file_open = 0;
	variables_stockage.fichier_donnees_Xsens1 = NULL;
	variables_stockage.fichier_donnees_Xsens2 = NULL;
	//variables_stockage->fichier_donnees_Accoust = NULL;
	variables_stockage.fichier_donnees_pression = NULL;
	variables_stockage.fichier_donnees_vibration = NULL;
	variables_stockage.fichier_annexe = NULL;
}

void connexion_peripherique() {
	if (1 == 1) { // Connexion du module Xsens 1
		printf("Le module IMU 1 a été detecté ! (id = %04X)\n", 1);
		printf("Module Xsens 1 connecté avec succès\n");
		foundModules.Xsens1_trouve = 1; 
		foundModules.nb_peripheriques++;
	}

	if (1 == 1) { // Connexion du module pression (ID 4747:4d34)
		printf("Le module de pression a été detecté ! (id = %04X)\n", 2);
		printf("Module de pression connecté avec succès\n");
		foundModules.pression_trouve = 1;
		foundModules.nb_peripheriques++;
	}
	
	if (1 == 1) { // Connexion du module Xsens 2 (ID 4747:4d42)
		printf("Le module Xsens 2 a été detecté ! (id = %04X)\n", 3);
		printf("Module Xsens 2 connecté avec succès\n");
		foundModules.Xsens2_trouve = 1;
		foundModules.nb_peripheriques++;
	} 
	
	if (1 == 1) { // Connexion du module de vibration (ID 4747:4d35)
		printf("Le module de vibration a été detecté ! (id = %04X)\n", 4);
		printf("Module de vibration connecté avec succès\n");
		foundModules.vibration_trouve = 1;
		foundModules.nb_peripheriques++;
	}
	
}

void ouverture_initiale_stockage() {
	variables_stockage.fichier_annexe = fopen("Data/annexe_fichiers.txt", "a");
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
	gettimeofday(&actualMetrics.temps_fichier, NULL);
	fichier_ms = (actualMetrics.temps_fichier.tv_sec * 1000000) + actualMetrics.temps_fichier.tv_usec;
}

void fermeture_fichiers() {
	if (variables_stockage.file_open == 1) {
		
		if (foundModules.Xsens1_trouve == 1) {
			fclose(variables_stockage.fichier_donnees_Xsens1);
		}
		if (foundModules.Xsens2_trouve == 1) {
			fclose(variables_stockage.fichier_donnees_Xsens2);
		}
		if (foundModules.pression_trouve == 1) {
			fclose(variables_stockage.fichier_donnees_pression);
		}
		if (foundModules.vibration_trouve == 1) {
			fclose(variables_stockage.fichier_donnees_vibration);
		}
	}
}