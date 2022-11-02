#ifndef OBC_H
#define OBC_H

/*! -----------------------------------------------   Library Declaration    -----------------------------------------------*/

#include <stdio.h>								// Standard Input/Output : printf(), scanf(), fopen(), fclose()					
#include <stdlib.h>								// Standard Library functions : free(), exit(), malloc()
#include <string.h>								// Strings : size_t type, NULL value, memcpy(), memset(), memmove()
#include <time.h>								// Time : time_t 

#include <unistd.h>								// Defines miscellaneous symbolic constants and types and functions
#include <errno.h>								// Error number for debuging 
#include <inttypes.h>							// fprintf(), fscanf()
#include <math.h>								// Mathematics functions the GNC 
#include <malloc.h>								// Malloc() pour l'attribution dynamique de mémoire des buffers 
#include <sched.h>								// Defines the sched_param structure for scheduling threads 
#include <signal.h>								// SIG_ERR : return value from signal() in case of error for debuging 
#include <assert.h>								// Verify assumptions made by the program and print a diagnostic message if this assumption is false
#include <fcntl.h>								// Manipulate file descriptor with open()
#include <stdbool.h>
#include <sys/time.h>
#include <pthread.h>							// Threads pour le lancement en parallèle de plusieurs fonctions 

//! User parameter 					Value				Access						Description
#define Xlink_verbose 				0					// 							Print informations in the console or not (0 : OFF / 1 : ON)
#define XSENS_verbose				0 

#define	TIME_WAIT					100
#define SEC_IN_MICRO 				1000000 
#define USB_LOOP_WAIT				10000
//! Error 
#define handle_error_en(en, msg) \
    do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)


/*! -----------------------------------------------   Globals Variables    -----------------------------------------------*/

extern struct timeval startoum;
extern struct timeval endoum;
extern long long unsigned start_time;
extern struct tm *tm_T0;

/*! -----------------------------------------------   Prototype   -----------------------------------------------*/

// Fonctionnement général et récupération des données USB 
void saisie_utilisateur(long long unsigned int* temps_acquisition, int* nb_essais_total);
void transfer(long long unsigned int* temps_acquisition);
#endif
