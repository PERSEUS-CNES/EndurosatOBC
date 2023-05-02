#include <stdio.h>
#include <stdlib.h>


#include "VariableGlobale.h"
#include "Structure.h"

int main() {

    FILE* fichier = fopen("./Data/fichier_EKF.txt", "rb");
    if (fichier == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    EKF p2 ;

    fread(&p2, sizeof(EKF), 1, fichier);
    fclose(fichier);

    printf("EKF euleur0 %f\n" , p2.euler[0] ) ; 
    printf("EKF euleur1 %f\n" , p2.euler[1] ) ; 
    printf("EKF euleur2 %f\n" , p2.euler[2] ) ; 

    afficher_code_binaire_float(p2.euler[0]);

    printf("EKF eulerStdDev 0 %f\n" , p2.eulerStdDev[0] ) ; 
    printf("EKF eulerStdDev 1 %f\n" , p2.eulerStdDev[1] ) ; 
    printf("EKF eulerStdDev 2 %f\n" , p2.eulerStdDev[2] ) ; 

    //ToString_data_EKF(p2);

}