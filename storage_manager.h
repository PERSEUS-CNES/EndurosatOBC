#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

/*! -----------------------------------------------   Types & structures    -----------------------------------------------*/

struct variables_fichiers {
	int compteur_fichiers;
	int file_open;
	char nom_fichier[30];
	FILE* fichier_donnees_Xsens1;
	FILE* fichier_donnees_Xsens2;	
	FILE* fichier_donnees_pression;
	FILE* fichier_donnees_vibration;
	FILE* fichier_annexe;
};

struct storageMetrics {
	int nb_frames_pression;
	int nb_frames_vibration;
	int nb_frames_xsens1;
	int nb_frames_xsens2;
	int verbose_mode;
	struct timeval start;  
	struct timeval end;
	struct timeval temps_actuel;
	struct timeval temps_fichier;
};

struct variables_connexion {
	int Xsens1_trouve;
	int Xsens2_trouve;
	int pression_trouve;
	int vibration_trouve;
	int nb_peripheriques;

};

extern long long unsigned fichier_ms;

extern struct storageMetrics actualMetrics;
extern struct variables_fichiers variables_stockage;
extern struct variables_connexion foundModules;

void initialisation_foundModules();
void initialisation_actualMetrics();
void initialisation_variables_stockage();
void connexion_peripherique();
void fermeture_fichiers();

void ouverture_initiale_stockage();
#endif