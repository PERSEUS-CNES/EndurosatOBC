/**
 * @file struct.c
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant toute les fonctions utiles pour manipuler les structures définis
 * @date 2023-03-31
 */


#include "./Librairies/Structure.h"
#include <string.h>


/**
 * @brief Fonction qui transforme GPS_pos en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param GPS_pos -> Structure contenant les données du GPS_pos
 */
void ToString_data_GPS_pos (GPS_pos GPS_pos) {
    
    char message[359] = "" ;
    char marqeur[2] = ";" ;

    char longitude[65]="";
    snprintf(longitude, sizeof longitude,"%f%s",GPS_pos.longitude,marqeur);
    strcat(message, longitude);

    char altitude[65]="";
    snprintf(altitude, sizeof altitude,"%f%s",GPS_pos.altitude,marqeur);
    strcat(message, altitude);   

    char latitude[65]="";
    snprintf(latitude, sizeof latitude,"%f%s",GPS_pos.latitude,marqeur);
    strcat(message, latitude);   

    char undulation[33]="";
    snprintf(undulation, sizeof undulation,"%f%s",GPS_pos.undulation,marqeur);
    strcat(message, undulation);   

    char latitudeAccuracy[33]="";
    snprintf(latitudeAccuracy, sizeof latitudeAccuracy,"%f%s",GPS_pos.latitudeAccuracy,marqeur);
    strcat(message, latitudeAccuracy);   

    char longitudeAccuracy[33]="";
    snprintf(longitudeAccuracy, sizeof longitudeAccuracy,"%f%s",GPS_pos.longitudeAccuracy,marqeur);
    strcat(message, longitudeAccuracy); 

    char altitudeAccuracy[33]="";
    snprintf(altitudeAccuracy, sizeof altitudeAccuracy,"%f%s",GPS_pos.altitudeAccuracy,marqeur);
    strcat(message, altitudeAccuracy);   

    char timeStamp[32]="";
    snprintf(timeStamp, sizeof timeStamp,"%u",GPS_pos.timeStamp);
    sprintf(timeStamp, "%u%s",GPS_pos.timeStamp,marqeur);
    strcat(message, timeStamp);  
 
    printf("GPS_pos : %s\n" , message); 

}

/**
 * @brief Fonction qui transforme GPS_vel en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param GPS_vel -> Structure contenant les données du GPS_vel
 */
void ToString_data_GPS_vel (GPS_vel GPS_vel) {
    
    char message[320] = "" ;
    char marqeur[2] = ";" ;
 
    for (int k = 0 ; k < 3 ; k ++) {
        char velocity[33]="";
        snprintf(velocity, sizeof velocity,"%f%s",GPS_vel.velocity[k],marqeur);
        strcat(message, velocity);   
    }

    for (int k = 0 ; k < 3 ; k ++) {
        char velocityAcc[33]="";
        snprintf(velocityAcc, sizeof velocityAcc,"%f%s",GPS_vel.velocityAcc[k],marqeur);
        strcat(message, velocityAcc);
    }

    char timeStamp[32]="";
    snprintf(timeStamp, sizeof timeStamp,"%u",GPS_vel.timeStamp);
    strcat(message, timeStamp);   

    printf("GPS_vel : %s\n" , message); 

}


/**
 * @brief Fonction qui transforme IMU en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param IMU -> Structure contenant les données du IMU
 */
void ToString_data_IMU (IMU IMU) {
    
    char message[461] = "" ;
    char marqeur[2] = ";" ;
 
    for (int k = 0 ; k < 3 ; k ++) {
        char accelerometers[33];
        snprintf(accelerometers, sizeof accelerometers,"%f%s",IMU.accelerometers[k],marqeur);
        strcat(message, accelerometers); 

    }

    for (int k = 0 ; k < 3 ; k ++) {
        char gyroscopes[33]="";
        snprintf(gyroscopes, sizeof gyroscopes,"%f%s",IMU.gyroscopes[k],marqeur);
        strcat(message, gyroscopes);   
    }

    char temperature[33]="";
    snprintf(temperature, sizeof temperature,"%f%s",IMU.temperature,marqeur);
    strcat(message, temperature);   

    for (int k = 0 ; k < 3 ; k ++) {
        char deltaVelocity[33]="";
        snprintf(deltaVelocity, sizeof deltaVelocity,"%f%s",IMU.deltaVelocity[k],marqeur);
        strcat(message, deltaVelocity);   
    }

    for (int k = 0 ; k < 3 ; k ++) {
        char deltaAngle[33]="";
        snprintf(deltaAngle, sizeof deltaAngle,"%f%s",IMU.deltaAngle[k],marqeur);
        strcat(message, deltaAngle);   
    }

    char timeStamp[32]="";
    snprintf(timeStamp, sizeof timeStamp ,"%u",IMU.timeStamp);
    strcat(message, timeStamp);   

    printf("IMU : %s\n" , message); 

}


/**
 * @brief Fonction qui transforme Magnetomètre en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param Magnetometers -> Structure contenant les données du Magnétomètre
 */
void ToString_data_Magnetometers (Magnetometers Magnetometers) {
    
    char message[131] = "" ;
    char marqeur[2] = ";" ;
 
    for (int k = 0 ; k < 3 ; k ++) {
        char magnetometers[33]="";
        snprintf(magnetometers, sizeof magnetometers,"%f%s",Magnetometers.magnetometers[k],marqeur);
        strcat(message, magnetometers); 
    }

    char timeStamp[32]="";
    snprintf(timeStamp, sizeof timeStamp ,"%u",Magnetometers.timeStamp);
    strcat(message, timeStamp);   

    printf("Magnetometre : %s\n" , message); 

}

/**
 * @brief Fonction qui transforme Pressure en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param Pressure -> Structure contenant les données de Pressionn
 */
void ToString_data_Pressure (Pressure Pressure) {
    
    char message[98] = "" ;
    char marqeur[2] = ";" ;

    char pressure[33]="";
    snprintf(pressure, sizeof pressure ,"%f%s",Pressure.pressure,marqeur);
    strcat(message, pressure); 

    char height[33]="";
    snprintf(height, sizeof height ,"%f%s",Pressure.height,marqeur);
    strcat(message, height);  

    char timeStamp[32]="";
    snprintf(timeStamp, sizeof timeStamp ,"%u",Pressure.timeStamp);
    strcat(message, timeStamp);   

    printf("Pressure : %s\n" , message); 

}

/**
 * @brief Fonction qui transforme EKF en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param EKF -> Structure contenant les données de l'EKF
 */
void ToString_data_EKF (EKF EKF) {
    
    char message[263] = "" ;
    char marqeur[2] = ";" ;

    for (int k = 0 ; k < 3 ; k ++) {
        char euler[33]="";
        snprintf(euler, sizeof euler,"%f%s",EKF.euler[k],marqeur);
        strcat(message, euler);   
    }


    for (int k = 0 ; k < 3 ; k ++) {
        char eulerStdDev[33]="";
        snprintf(eulerStdDev, sizeof eulerStdDev,"%f%s",EKF.eulerStdDev[k],marqeur);
        strcat(message, eulerStdDev);   
    }

    for (int k = 0 ; k < 3 ; k ++) {
        char quaternion[33]="";
        snprintf(quaternion, sizeof quaternion,"%f%s",EKF.quaternion[k],marqeur);
        strcat(message, quaternion);   
    }

    char timeStamp[32]="";
    snprintf(timeStamp, sizeof timeStamp ,"%u",EKF.timeStamp);
    strcat(message, timeStamp);   

    printf("EKF : %s\n" , message); 

}

/**
 * @brief Fonction qui transforme CLOCK en chaine de carractère
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param CLOCK -> Structure contenant les données de l'horloge
 */
void ToString_data_CLOCK (CLOCK CLOCK) {
    char message[100] = "" ;
    char marqeur[2] = ";" ;

    char year[17]="";
    snprintf(year, sizeof year,"%d%s",CLOCK.year,marqeur);
    strcat(message, year);  

    char month[8]="";
    snprintf(month, sizeof month,"%d%s",CLOCK.month,marqeur);
    strcat(message, month);  

    char day[8]="";
    snprintf(day, sizeof day,"%d%s",CLOCK.day,marqeur);
    strcat(message, day); 

    char hour[8]="";
    snprintf(hour, sizeof hour,"%d%s",CLOCK.hour,marqeur);
    strcat(message, hour);  

    char minute[8]="";
    snprintf(minute, sizeof minute,"%d%s",CLOCK.minute,marqeur);
    strcat(message, minute); 

    char second[8]="";
    snprintf(second, sizeof second,"%d%s",CLOCK.second,marqeur);
    strcat(message, second);  

    char nanoSecond[33]="";
    snprintf(nanoSecond, sizeof nanoSecond,"%d%s",CLOCK.nanoSecond,marqeur);
    strcat(message, nanoSecond);   

    printf("CLOCK : %s\n" , message); 

}

#include <stdio.h>

void afficher_code_binaire_float(float nombre) {
    unsigned char* pointeur_octet = (unsigned char*) &nombre;
    int i;
    for (i = sizeof(float) - 1; i >= 0; i--) {
        unsigned char octet = *(pointeur_octet + i);
        int j;
        for (j = 7; j >= 0; j--) {
            printf("%d", (octet >> j) & 1);
        }
        printf(" ");
    }
    printf("\n");
}

