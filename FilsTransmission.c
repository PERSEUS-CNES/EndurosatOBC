/**
 * @file FilsTransmission.c
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant le code du fils Transmission
 * @date 2023-04-04
 */

#include "./Librairies/Structure.h"
#include "./Librairies/VariableGlobale.h"

#include "./Librairies/ReceptionDataFileMessage.h"
#include "./Librairies/EnvoieDataFileMessage.h"

#include <stdio.h>		/* stderr, stdout, fprintf, perror */
#include <stdlib.h>

/**
 * @brief Fonction corps du fils Transmssion
 * 
 * @date 04/04/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 */
void FilsTransmission() {
    Message  * message = malloc(sizeof(Message));
    *message = receptionCentrale() ;

    switch (message->type) {
            case GPS_POS_TYPE: ;
                GPS_pos gps_pos_data = message->data.gps_pos;

                Envoie_data_GPS_pos(gps_pos_data,1, SAUVEGARDE);
                Envoie_data_GPS_pos(gps_pos_data,1, EMETTEUR);

                break;
            case GPS_VEL_TYPE: ;
                GPS_vel gps_vel_data = message->data.gps_vel;

                Envoie_data_GPS_vel(gps_vel_data , 1, SAUVEGARDE);
                Envoie_data_GPS_vel(gps_vel_data , 1, EMETTEUR);

                break;
            case IMU_TYPE: ;
                IMU imu_data = message->data.imu;
            
                Envoie_data_IMU(imu_data , 1, SAUVEGARDE);
                Envoie_data_IMU(imu_data , 1, EMETTEUR);

                break;
            case MAGNETOMETERS_TYPE: ;
                Magnetometers magnetometers_data = message->data.magnetometers;

                Envoie_data_Magnetometers(magnetometers_data , 1, SAUVEGARDE);
                Envoie_data_Magnetometers(magnetometers_data , 1, EMETTEUR);

                break;
            case PRESSURE_TYPE: ;
                Pressure pressure_data = message->data.pressure;

                Envoie_data_Pressure(pressure_data , 1, SAUVEGARDE);
                Envoie_data_Pressure(pressure_data , 1, EMETTEUR);

                break;
            case EKF_TYPE: ;
                EKF ekf_data = message->data.ekf;

                Envoie_data_EKF(ekf_data , 1, SAUVEGARDE);
                Envoie_data_EKF(ekf_data , 1, EMETTEUR);

                break;
            case CLOCK_TYPE: ;
                CLOCK clock_data = message->data.clock;

                Envoie_data_CLOCK(clock_data , 1, SAUVEGARDE);
                Envoie_data_CLOCK(clock_data , 1, EMETTEUR);

                break;
            default:
                //return 0;
                break;
        }

    free(message);
    FilsTransmission();
}