/**
 * @file outils_centrale.c
 * @author Team OBC (ENSSAT)
 * @brief Fichier contenant tous les outils necessaires pour la communication entre l'OBC et la centrale inertiel
 * @date 2023-02-04
 */

#include "./Librairies/EnvoieDataFileMessage.h"
#include "./Librairies/VariableGlobale.h"
#include "./Librairies/Structure.h"


/**
 * @brief Fonction qui envoie à l'OBC les données du GPS_pos
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param GPS_pos  -> Adresse de la structure contenant les données du GPS_pos
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_GPS_pos (GPS_pos GPS_pos ,unsigned int priorite , PeripheriqueType peripherique) {
    int transmition_ok ;

    Message message;

    message.type = GPS_POS_TYPE;
    message.data.gps_pos = GPS_pos ;

    switch (peripherique)
    {
    case CENTRALE :
        transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
        break;

    case EMETTEUR :
        transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
        break;

    case SAUVEGARDE :
        transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
        break;
		
	case ENVOI_SS :
		transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
        break;
 

    default:
        break;
    }

    //printf("Envoie GPS_pos fini !\n");

    return transmition_ok ;

}


/**
 * @brief Fonction qui envoie à l'OBC les données du GPS_vel
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param GPS_vel  -> Adresse de la structure contenant les données du GPS_vel
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_GPS_vel (GPS_vel GPS_vel , unsigned int priorite , PeripheriqueType peripherique) {
    
    int transmition_ok ;

    Message message;

    message.type = GPS_VEL_TYPE;
    message.data.gps_vel = GPS_vel ;

    switch (peripherique)
        {
        case CENTRALE :
            transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
            break;

        case EMETTEUR :
            transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
            break;

        case SAUVEGARDE :
            transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
            break;
		
		case ENVOI_SS :
			transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
			break;

        default:
            break;
    }
                    
    //printf("Envoie GPS_vel fini !\n");

    return transmition_ok ;

}


/**
 * @brief Fonction qui envoie à l'OBC les données de l'IMU
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param IMU -> Adresse de la structure contenant les données de l'IMU
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_IMU (IMU IMU , unsigned int priorite , PeripheriqueType peripherique) {
    
    int transmition_ok ;

    Message message;

    message.type = IMU_TYPE;
    message.data.imu = IMU ;

    switch (peripherique)
        {
        case CENTRALE :
            transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
            break;

        case EMETTEUR :
            transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
            break;

        case SAUVEGARDE :
            transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
            break;
		
		case ENVOI_SS :
			transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
			break;

        default:
            break;
    }    

    //printf("Envoie IMU fini !\n");

    return transmition_ok ;
}


/**
 * @brief Fonction qui envoie à l'OBC les données du Magnetometers
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param Magnetometers -> Adresse de la structure contenant les données du Magnetometers
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_Magnetometers (Magnetometers Magnetometers , unsigned int priorite , PeripheriqueType peripherique) {
    
    int transmition_ok ;

    Message message;

    message.type = MAGNETOMETERS_TYPE;
    message.data.magnetometers = Magnetometers ;

    switch (peripherique)
        {
        case CENTRALE :
            transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
            break;

        case EMETTEUR :
            transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
            break;

        case SAUVEGARDE :
            transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
            break;
		
		case ENVOI_SS :
			transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
			break;

        default:
            break;
    }

    //printf("Envoie Magnetometre fini !\n");

    return transmition_ok ;

}


 /** @brief Fonction qui envoie à l'OBC les données de Pression
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param Pressure -> Adresse de la structure contenant les données de Pression
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_Pressure (Pressure Pressure , unsigned int priorite , PeripheriqueType peripherique) {
    
    int transmition_ok ;

    Message message;

    message.type = PRESSURE_TYPE;
    message.data.pressure = Pressure ;

    switch (peripherique)
        {
        case CENTRALE :
            transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
            break;

        case EMETTEUR :
            transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
            break;

        case SAUVEGARDE :
            transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
            break;
			
		case ENVOI_SS :
			transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
			break;

        default:
            break;
    }        

    //printf("Envoie Pression fini !\n");

    return transmition_ok ;

}


/** @brief Fonction qui envoie à l'OBC les données de EKF_nav
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param EKF_nav -> Adresse de la structure contenant les données de navigation
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_EKF_nav (EKF_nav nav , unsigned int priorite , PeripheriqueType peripherique) {
    
    int transmition_ok ;

    Message message;

    message.type = EKF_NAV_TYPE;
    message.data.ekf_nav = nav ;

    switch (peripherique)
        {
        case CENTRALE :
            transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
            break;

        case EMETTEUR :
            transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
            break;

        case SAUVEGARDE :
            transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
            break;
			
		case ENVOI_SS :
			transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
			break;

        default:
            break;
    }        

    //printf("Envoie Pression fini !\n");

    return transmition_ok ;

}
 /** @brief Fonction qui envoie à l'OBC les données de l'EKF
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 *
 * @param Pressure -> Adresse de la structure contenant les données de l'EKF
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_EKF (EKF EKF , unsigned int priorite , PeripheriqueType peripherique) {
    int transmition_ok ;

    Message message;

    message.type = EKF_TYPE;
    message.data.ekf = EKF ;

    switch (peripherique)
        {
        case CENTRALE :
            transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
            break;

        case EMETTEUR :
            transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
            break;

        case SAUVEGARDE :
            transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
            break;
		
		case ENVOI_SS :
			transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
			break;

        default:
            break;
    }    

    //printf("Envoie EKF fini !\n");

    return transmition_ok ;

}

/**
 * @brief Fonction qui envoie à l'OBC les données du CLOCK
 * 
 * @date 07/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * 
 * @param CLOCK  -> Adresse de la structure contenant les données du CLOCK
 * @param priorite -> Priotité du message
 * @param peripherique -> Destinationn de l'envoie
 * 
 * @return transmition_ok -> Valeur de retour de l'envoi : 
 *                                                          - -1 -> envoie échoué
 *                                                          - 0  -> envoie reussi
 *
 */
int Envoie_data_CLOCK (CLOCK CLOCK ,unsigned int priorite , PeripheriqueType peripherique) {
    int transmition_ok ;

    Message message;

    message.type = CLOCK_TYPE;
    message.data.clock = CLOCK ;

    switch (peripherique)
    {
    case CENTRALE :
        transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
        break;

    case EMETTEUR :
        transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
        break;

    case SAUVEGARDE :
        transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
        break;
	
	case ENVOI_SS :
		transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
        break;

    default:
        break;
    }

    //printf("Envoie CLOCK fini !\n");

    return transmition_ok ;

}

int Envoie_data_Energie (SYSENERGIE SYSENERGIE ,unsigned int priorite , PeripheriqueType peripherique) {
    int transmition_ok ;
    
    Message message;

    message.type = ENERGIE_TYPE;
    message.data.sysenergie = SYSENERGIE ;

    switch (peripherique)
    {
    case CENTRALE :
        transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
        break;

    case EMETTEUR :
        transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
        break;

    case SAUVEGARDE :
        transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
        break;
		
	case ENVOI_SS :
		transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
        break;
 

    default:
        break;
    }

    //printf("Envoie GPS_pos fini !\n");

    return transmition_ok ;

}

int Envoie_data_Status (STATUS STATUS ,unsigned int priorite , PeripheriqueType peripherique) {
    int transmition_ok ;
    printf("je suis là\n");
    Message message;

    message.type = STATUS_TYPE;
    message.data.status = STATUS ;

    switch (peripherique)
    {
    case CENTRALE :
        transmition_ok = mq_send (FileDeMessage.file_message_Centrale, (const char*)&message, sizeof(Message) , priorite);
        break;

    case EMETTEUR :
        transmition_ok = mq_send (FileDeMessage.file_message_Emetteur, (const char*)&message, sizeof(Message) , priorite);
        break;

    case SAUVEGARDE :
        printf("Sauvegarde envoi data\n");
        printf("%d\n",FileDeMessage.file_message_Sauvegarde);
        transmition_ok = mq_send (FileDeMessage.file_message_Sauvegarde, (const char*)&message, sizeof(Message) , priorite);
        break;
		
	case ENVOI_SS :
		transmition_ok = mq_send (FileDeMessage.file_message_SS, (const char*)&message, sizeof(Message) , priorite);
        break;

    /*case ENVOI_STATUS :
        transmition_ok = mq_send (FileDeMessage.file_message_Status, (const char*)&message, sizeof(Message) , priorite);
        break;*/
 
    default:
        break;
    }

    printf("je suis là 2\n");

    return transmition_ok ;

}