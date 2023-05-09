#include "./Librairies/FilsCentrale.h"
#include "./Librairies/Structure.h"
#include "./Librairies/EnvoieDataFileMessage.h"

#include "./Centrale/sbgECom/src/sbgEComLib.h"
#include <pthread.h>

//Clock SBGClock;
EKF SBG_EKF ;
Pressure  SBGPressure;
IMU  SBGIMU;
GPS_vel  SBGGPS_vel;
GPS_pos  SBGGPS_pos;
EKF_nav SBGEKF_nav;
Magnetometers SBGMagnetometer;
CLOCK SBGClock;
unsigned char msg_EKF ;
unsigned char msg_EKF_nav ;
unsigned char msg_Pressure;
unsigned char msg_IMU;
unsigned char msg_GPS_vel;
unsigned char msg_GPS_pos;
unsigned char msg_Magnetometer;
unsigned char msg_Clock;
unsigned char msg_Quat;


//Initialisation des mutex pour éviter les problèmes d'accès simultanés pour envoyer vers l'obc et récupérer les données.

pthread_mutex_t mutex_msg_EKF = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutex_msg_EKF_nav = PTHREAD_MUTEX_INITIALIZER; 
pthread_mutex_t mutex_msg_Pressure = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_msg_IMU = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_msg_GPS_vel = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_msg_GPS_pos = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_msg_Clock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_msg_Magnetometer = PTHREAD_MUTEX_INITIALIZER;



/**
 * @brief Fonction de réception pour la centrale (issue des exemples)
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * @param ->	pHandle									Valid handle on the sbgECom instance that has called this callback.
 * @param ->	msgClass								Class of the message we have received
 * @param ->	msg										Message ID of the log received.
 * @param ->	pLogData								Contains the received log data as an union.
 * @param ->	pUserArg								Optional user supplied argument.
 * @return												SBG_NO_ERROR if the received log has been used successfully.
 */
 
SbgErrorCode onLogReceived(SbgEComHandle *pHandle, SbgEComClass msgClass, SbgEComMsgId msg, const SbgBinaryLogData *pLogData, void *pUserArg)
{
	//
	// Handle separately each received data according to the log ID
	//
	switch (msg)
	{
	case SBG_ECOM_LOG_EKF_EULER:
		//
		// Simply display euler angles in real time
		//
		/*printf("Euler Angles: %3.1f\t%3.1f\t%3.1f\tStd Dev:%3.1f\t%3.1f\t%3.1f   \r", 
				sbgRadToDegF(pLogData->ekfEulerData.euler[0]), sbgRadToDegF(pLogData->ekfEulerData.euler[1]), sbgRadToDegF(pLogData->ekfEulerData.euler[2]), 
				sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[0]), sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[1]), sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[2]));*/
    //
    // Transfers the data to our own structure so that we can save it later on
    //
    pthread_mutex_lock(&mutex_msg_EKF);
    
    SBG_EKF.euler[0] = pLogData->ekfEulerData.euler[0];
    SBG_EKF.euler[1] = pLogData->ekfEulerData.euler[1]; 
    SBG_EKF.euler[2] = pLogData->ekfEulerData.euler[2];
    SBG_EKF.eulerStdDev[0] =  sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[0]);
    SBG_EKF.eulerStdDev[1] =  sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[1]);
    SBG_EKF.eulerStdDev[2] =  sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[2]);
    SBG_EKF.timeStamp = pLogData->ekfEulerData.timeStamp;
    
    msg_EKF = 1;
    pthread_mutex_unlock(&mutex_msg_EKF);
    
		break;
	
	case SBG_ECOM_LOG_EKF_NAV:
		//
		// Simply display euler angles in real time
		//
		/*printf("Euler Angles: %3.1f\t%3.1f\t%3.1f\tStd Dev:%3.1f\t%3.1f\t%3.1f   \r", 
				sbgRadToDegF(pLogData->ekfEulerData.euler[0]), sbgRadToDegF(pLogData->ekfEulerData.euler[1]), sbgRadToDegF(pLogData->ekfEulerData.euler[2]), 
				sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[0]), sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[1]), sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[2]));*/
    //
    // Transfers the data to our own structure so that we can save it later on
    //
    pthread_mutex_lock(&mutex_msg_EKF_nav);
    
    SBGEKF_nav.position[0] = pLogData->ekfNavData.position[0];
    SBGEKF_nav.position[1] = pLogData->ekfNavData.position[1];
    SBGEKF_nav.position[2] = pLogData->ekfNavData.position[2];
    SBGEKF_nav.positionStdDev[0] = pLogData->ekfNavData.positionStdDev[0];
    SBGEKF_nav.positionStdDev[1] = pLogData->ekfNavData.positionStdDev[1];
    SBGEKF_nav.positionStdDev[2] = pLogData->ekfNavData.positionStdDev[2];
    SBGEKF_nav.velocity[0] = pLogData->ekfNavData.velocity[0];
	SBGEKF_nav.velocity[1] = pLogData->ekfNavData.velocity[1];
	SBGEKF_nav.velocity[2] = pLogData->ekfNavData.velocity[2];
	SBGEKF_nav.velocityStdDev[0] = pLogData->ekfNavData.velocityStdDev[0];
	SBGEKF_nav.velocityStdDev[1] = pLogData->ekfNavData.velocityStdDev[1];
	SBGEKF_nav.velocityStdDev[2] = pLogData->ekfNavData.velocityStdDev[2];
	SBGEKF_nav.undulation = pLogData->ekfNavData.undulation;
	SBGEKF_nav.timeStamp = pLogData->ekfNavData.timeStamp;
    
    msg_EKF_nav = 1;
    pthread_mutex_unlock(&mutex_msg_EKF_nav);
    
		break;
   
    case 	SBG_ECOM_LOG_EKF_QUAT:
    
     pthread_mutex_lock(&mutex_msg_EKF);
     
    SBG_EKF.quaternion[0] =   pLogData->ekfQuatData.quaternion[0];
    SBG_EKF.quaternion[1] =   pLogData->ekfQuatData.quaternion[1];
    SBG_EKF.quaternion[2] =   pLogData->ekfQuatData.quaternion[2];
    SBG_EKF.quaternion[3] =   pLogData->ekfQuatData.quaternion[3];
    SBG_EKF.timeStamp = pLogData->ekfQuatData.timeStamp;
    
   
    msg_EKF = 1;
    pthread_mutex_unlock(&mutex_msg_EKF);
    
    break;
    
   case SBG_ECOM_LOG_PRESSURE:
	//	printf("\n Pressure: %3.1f\t%3.1f \r", 
				//pLogData->pressureData.pressure, pLogData->pressureData.height);
    //
    // Transfers the data to our own structure so that we can save it later on
    //
     pthread_mutex_lock(&mutex_msg_Pressure);
     
        SBGPressure.pressure = pLogData->pressureData.pressure;
        SBGPressure.height = pLogData->pressureData.height;
        SBGPressure.timeStamp = pLogData->pressureData.timeStamp;
        
   
    msg_Pressure = 1;
    pthread_mutex_unlock(&mutex_msg_Pressure);
        
		break;

   case SBG_ECOM_LOG_IMU_DATA:
   
   
     /*printf("timestamp: %u\tAcc(m.s^-2): %3.1f\t%3.1f\t%3.1f\tGyroscope(rad.s^-1):%3.1f\t%3.1f\t%3.1f\tTemp(C):%3.1f\tdeltaVelo(m.s^-2):%3.1f\t%3.1f\t%3.1f\tDeltaAngle(rad.s^-1):%3.1f\t%3.1f\t%3.1f\t\r",
				pLogData->imuData.timeStamp,pLogData->imuData.accelerometers[0],pLogData->imuData.accelerometers[1],pLogData->imuData.accelerometers[2],pLogData->imuData.gyroscopes[0],pLogData->imuData.gyroscopes[1],pLogData->imuData.gyroscopes[2],pLogData->imuData.temperature,pLogData->imuData.deltaVelocity[0],pLogData->imuData.deltaVelocity[1],pLogData->imuData.deltaVelocity[2],pLogData->imuData.deltaAngle[0],pLogData->imuData.deltaAngle[1],pLogData->imuData.deltaAngle[2]);*/
   //
   //  Transfers the data to our own structure so that we can save it later on
   //
   pthread_mutex_lock(&mutex_msg_IMU);
   SBGIMU.accelerometers[0] = pLogData->imuData.accelerometers[0];
   SBGIMU.accelerometers[1] = pLogData->imuData.accelerometers[1];
   SBGIMU.accelerometers[2] = pLogData->imuData.accelerometers[2];
   SBGIMU.gyroscopes[0] = pLogData->imuData.gyroscopes[0];
   SBGIMU.gyroscopes[1] = pLogData->imuData.gyroscopes[1];
   SBGIMU.gyroscopes[2] = pLogData->imuData.gyroscopes[2];
   SBGIMU.gyroscopes[3] = pLogData->imuData.gyroscopes[3];
   SBGIMU.temperature =  pLogData->imuData.temperature;
   SBGIMU.deltaVelocity[0] =  pLogData->imuData.deltaVelocity[0];
   SBGIMU.deltaVelocity[1] =  pLogData->imuData.deltaVelocity[1];
   SBGIMU.deltaVelocity[2] =  pLogData->imuData.deltaVelocity[2];
   SBGIMU.deltaAngle[0] =  pLogData->imuData.deltaAngle[0];
   SBGIMU.deltaAngle[1] =  pLogData->imuData.deltaAngle[1];
   SBGIMU.deltaAngle[2] =  pLogData->imuData.deltaAngle[2];
   SBGIMU.timeStamp =  pLogData->imuData.timeStamp;
    msg_IMU = 1;
    pthread_mutex_unlock(&mutex_msg_IMU);
    
   break;
   
   case SBG_ECOM_LOG_MAG:
    /* printf("\n Magneto: %3.1f\t%3.1f\t%3.1f\t AccMag: %3.1f\t%3.1f\t%3.1f\t \r", 
				pLogData->magData.magnetometers[0], pLogData->magData.magnetometers[1],pLogData->magData.magnetometers[2], pLogData->magData.accelerometers[0], pLogData->magData.magnetometers[1], pLogData->magData.magnetometers[2]);*/
        
     pthread_mutex_lock(&mutex_msg_Magnetometer);   
  SBGMagnetometer.magnetometers[0] = pLogData->magData.magnetometers[0];
  SBGMagnetometer.magnetometers[1] = pLogData->magData.magnetometers[1];
  SBGMagnetometer.magnetometers[2] = pLogData->magData.magnetometers[2];
  SBGMagnetometer.timeStamp = pLogData->magData.timeStamp;
  
  
    msg_Magnetometer = 1;
    pthread_mutex_unlock(&mutex_msg_Magnetometer);
  break;
  
  //Ne fonctionne pas car pas d'antenne GPS branchée      
   case SBG_ECOM_LOG_GPS1_POS:
     /*printf("\n Latitude: %3.1f\t Longitude: %3.1f\t Altitude: %3.1f\t\r",
       pLogData->gpsPosData.latitude, pLogData->gpsPosData.longitude, pLogData->gpsPosData.altitude);*/
       
       pthread_mutex_lock(&mutex_msg_GPS_pos);
       SBGGPS_pos.latitude = pLogData->gpsPosData.latitude;
       SBGGPS_pos.longitude = pLogData->gpsPosData.longitude;
       SBGGPS_pos.altitude = pLogData->gpsPosData.altitude;
       SBGGPS_pos.latitudeAccuracy = pLogData->gpsPosData.latitudeAccuracy;
       SBGGPS_pos.longitudeAccuracy = pLogData->gpsPosData.longitudeAccuracy;
       SBGGPS_pos.altitudeAccuracy = pLogData->gpsPosData.altitudeAccuracy;
       SBGGPS_pos.undulation = pLogData->gpsPosData.undulation;
       SBGGPS_pos.timeStamp = pLogData->gpsPosData.timeStamp;
       
    
    msg_GPS_pos = 1;
    pthread_mutex_unlock(&mutex_msg_GPS_pos);
   break;
   case SBG_ECOM_LOG_GPS1_VEL:
    /* printf("\n Velocity : %3.1f\t %3.1f\t %3.1f\t\r",
       pLogData->gpsVelData.velocity[0], pLogData->gpsVelData.velocity[1], pLogData->gpsVelData.velocity[2]);*/
       
        pthread_mutex_lock(&mutex_msg_GPS_vel);
       SBGGPS_vel.velocity[0] = pLogData->gpsVelData.velocity[0];
       SBGGPS_vel.velocity[1] = pLogData->gpsVelData.velocity[1];
       SBGGPS_vel.velocity[2] = pLogData->gpsVelData.velocity[2];
       SBGGPS_vel.velocityAcc[0] = pLogData->gpsVelData.velocityAcc[0];
       SBGGPS_vel.velocityAcc[1] = pLogData->gpsVelData.velocityAcc[1];
       SBGGPS_vel.velocityAcc[2] = pLogData->gpsVelData.velocityAcc[2];
       SBGGPS_vel.timeStamp = pLogData->gpsVelData.timeStamp;
       
    
    msg_GPS_vel = 1;
    pthread_mutex_unlock(&mutex_msg_GPS_vel);
    break;
   case SBG_ECOM_LOG_UTC_TIME:
   /*  printf("\n Heure : %d:%d:%d:%ld\t Date : %d/%d/%lu\r",
       pLogData->utcData.hour, pLogData->utcData.minute, pLogData->utcData.second, pLogData->utcData.nanoSecond, pLogData->utcData.day, pLogData->utcData.month, pLogData->utcData.year);*/
       
  pthread_mutex_lock(&mutex_msg_Clock);
  
  SBGClock.year = pLogData->utcData.year;
  SBGClock.month = pLogData->utcData.month;
  SBGClock.day = pLogData->utcData.day;
  SBGClock.hour = pLogData->utcData.hour;
  SBGClock.minute = pLogData->utcData.minute;
  SBGClock.second = pLogData->utcData.second;
  
    
    msg_Clock = 1;
    pthread_mutex_unlock(&mutex_msg_Clock);
   break;
   //////////////////
	default:
		break;
	}
	
	return SBG_NO_ERROR;
}



//

/**
 * @brief Fonction test pour ajouter des valeurs à transmettre (dans le cas où on a pas la centrale)
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * @param ->	msg										Message ID of the log received.
 * @return												SBG_NO_ERROR if the received log has been used successfully.
 */
 
unsigned char test_message(unsigned char msg)
{
	//
	// Handle separately each received data according to the log ID
	//
	switch (msg)
	{
	case SBG_ECOM_LOG_EKF_EULER:
		//
		// Simply display euler angles in real time
		//
		/*printf("Euler Angles: %3.1f\t%3.1f\t%3.1f\tStd Dev:%3.1f\t%3.1f\t%3.1f   \r", 
				sbgRadToDegF(pLogData->ekfEulerData.euler[0]), sbgRadToDegF(pLogData->ekfEulerData.euler[1]), sbgRadToDegF(pLogData->ekfEulerData.euler[2]), 
				sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[0]), sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[1]), sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[2]));*/
    //
    // Transfers the data to our own structure so that we can save it later on
    //
    pthread_mutex_lock(&mutex_msg_EKF);
    
    SBG_EKF.euler[0] = 12;
    SBG_EKF.euler[1] = 13; 
    SBG_EKF.euler[2] = 14;
    SBG_EKF.eulerStdDev[0] =  15;
    SBG_EKF.eulerStdDev[1] =  16;
    SBG_EKF.eulerStdDev[2] =  17;
    SBG_EKF.timeStamp = 18;
    
    msg_EKF = 1;
    pthread_mutex_unlock(&mutex_msg_EKF);
    
		break;
   
    case 	SBG_ECOM_LOG_EKF_QUAT:
    
     pthread_mutex_lock(&mutex_msg_EKF);
     
    SBG_EKF.quaternion[0] =   20;
    SBG_EKF.quaternion[1] =   21;
    SBG_EKF.quaternion[2] =   22;
    SBG_EKF.quaternion[3] =   23;
    SBG_EKF.timeStamp = 24;
    
   
    msg_Quat = 1;
    pthread_mutex_unlock(&mutex_msg_EKF);
    
    break;
    
   case SBG_ECOM_LOG_PRESSURE:
	//	printf("\n Pressure: %3.1f\t%3.1f \r", 
				//pLogData->pressureData.pressure, pLogData->pressureData.height);
    //
    // Transfers the data to our own structure so that we can save it later on
    //
     pthread_mutex_lock(&mutex_msg_Pressure);
     
        SBGPressure.pressure = 30;
        SBGPressure.height = 31;
        SBGPressure.timeStamp = 32;
        
   
    msg_Pressure = 1;
    pthread_mutex_unlock(&mutex_msg_Pressure);
        
		break;

   case SBG_ECOM_LOG_IMU_DATA:
   
   
     
   //
   //  Transfers the data to our own structure so that we can save it later on
   //
   pthread_mutex_lock(&mutex_msg_IMU);
   SBGIMU.accelerometers[0] = 40;
   SBGIMU.accelerometers[1] = 41;
   SBGIMU.accelerometers[2] = 42;
   SBGIMU.gyroscopes[0] = 43;
   SBGIMU.gyroscopes[1] = 44;
   SBGIMU.gyroscopes[2] = 45;
   SBGIMU.gyroscopes[3] = 46;
   SBGIMU.temperature =  47;
   SBGIMU.deltaVelocity[0] =  48;
   SBGIMU.deltaVelocity[1] =  49;
   SBGIMU.deltaVelocity[2] =  50;
   SBGIMU.deltaAngle[0] =  51;
   SBGIMU.deltaAngle[1] = 52;
   SBGIMU.deltaAngle[2] =  53;
   SBGIMU.timeStamp =  54;
    msg_IMU = 1;
    pthread_mutex_unlock(&mutex_msg_IMU);
    
   break;
   
   case SBG_ECOM_LOG_MAG:
    /* printf("\n Magneto: %3.1f\t%3.1f\t%3.1f\t AccMag: %3.1f\t%3.1f\t%3.1f\t \r", 
				pLogData->magData.magnetometers[0], pLogData->magData.magnetometers[1],pLogData->magData.magnetometers[2], pLogData->magData.accelerometers[0], pLogData->magData.magnetometers[1], pLogData->magData.magnetometers[2]);*/
        
     pthread_mutex_lock(&mutex_msg_Magnetometer);   
  SBGMagnetometer.magnetometers[0] = 60;
  SBGMagnetometer.magnetometers[1] = 61;
  SBGMagnetometer.magnetometers[2] = 62;
  SBGMagnetometer.timeStamp = 63;
  
  
    msg_Magnetometer = 1;
    pthread_mutex_unlock(&mutex_msg_Magnetometer);
  break;
  
  //Ne fonctionne pas car pas d'antenne GPS branchée      
   case SBG_ECOM_LOG_GPS2_POS:
     /*printf("\n Latitude: %3.1f\t Longitude: %3.1f\t Altitude: %3.1f\t\r",
       pLogData->gpsPosData.latitude, pLogData->gpsPosData.longitude, pLogData->gpsPosData.altitude);*/
       
       pthread_mutex_lock(&mutex_msg_GPS_pos);
       SBGGPS_pos.latitude = 70;
       SBGGPS_pos.longitude = 71;
       SBGGPS_pos.altitude = 72;
       SBGGPS_pos.latitudeAccuracy = 73;
       SBGGPS_pos.longitudeAccuracy = 74;
       SBGGPS_pos.altitudeAccuracy = 75;
       SBGGPS_pos.undulation = 76;
       SBGGPS_pos.timeStamp = 77;
       
    
    msg_GPS_pos = 1;
    pthread_mutex_unlock(&mutex_msg_GPS_pos);
   break;
   case SBG_ECOM_LOG_GPS2_VEL:
    /* printf("\n Velocity : %3.1f\t %3.1f\t %3.1f\t\r",
       pLogData->gpsVelData.velocity[0], pLogData->gpsVelData.velocity[1], pLogData->gpsVelData.velocity[2]);*/
       
        pthread_mutex_lock(&mutex_msg_GPS_vel);
       SBGGPS_vel.velocity[0] = 80;
       SBGGPS_vel.velocity[1] = 81;
       SBGGPS_vel.velocity[2] = 82;
       SBGGPS_vel.velocityAcc[0] = 83;
       SBGGPS_vel.velocityAcc[1] = 84;
       SBGGPS_vel.velocityAcc[2] = 85;
       SBGGPS_vel.timeStamp = 86;
       
    
    msg_GPS_vel = 1;
    pthread_mutex_unlock(&mutex_msg_GPS_vel);
    break;
   //Probleme format   
   case SBG_ECOM_LOG_UTC_TIME:
   /*  printf("\n Heure : %d:%d:%d:%ld\t Date : %d/%d/%lu\r",
       pLogData->utcData.hour, pLogData->utcData.minute, pLogData->utcData.second, pLogData->utcData.nanoSecond, pLogData->utcData.day, pLogData->utcData.month, pLogData->utcData.year);*/
       
  pthread_mutex_lock(&mutex_msg_Clock); 
  /*
  SBGClock.year =90;
  SBGClock.month = 91;
  SBGClock.day = 92;
  SBGClock.hour = 93;
  SBGClock.minute = 94;
  SBGClock.second = 95;
  SBGClock.nanoSeconds = 96;
  */
  
    
    msg_Clock = 1;
    pthread_mutex_unlock(&mutex_msg_Clock);
   break;
   //////////////////
	default:
		break;
	}
	
	return SBG_NO_ERROR;
}



/**
 * @brief Fonction d'initialisation du thread de communication avec l'obc, initialisation de la communication avec la centrale.
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 */
unsigned char FilsCentrale(){
  SbgEComHandle			comHandle;
	SbgErrorCode			errorCode;
	SbgInterface			sbgInterface;
	int32					retValue = 0;
	SbgEComDeviceInfo		deviceInfo;
  msg_EKF = 0; //flag de réception de message de la centrale pour EKF
  msg_Pressure =0;//flag de réception de message de la centrale pour Pressure
  msg_IMU =0;    //flag de réception de message de la centrale pour l'IMU
  msg_GPS_vel =0; //flag de réception de message de la centrale pour la vélocité calculer par le GPS
  msg_GPS_pos = 0;//flag de réception de message de la centrale pour la position du GPS
  msg_Magnetometer = 0;//flag de réception de message de la centrale pour le Magnetomètre
  msg_Clock = 0; //flag de réception de message de la centrale pour la Clock interne
  pthread_t thr1; 
  int returnValue; //return value pour le thread

    //Premier thread :
      int * arg_thr1 = malloc(sizeof(int)); // pointeur à passer à la routine
      *arg_thr1 = 2; // Au cas ou on ait besoin de passer des données
      returnValue=pthread_create(&thr1, NULL, envoie_message, (void *) arg_thr1);
	  free(arg_thr1);
      //  ci-dessus (void *) arg_thr1 car transtypage nécessaire pour respect du type imposé en argument de la routine 
       
       //Test de la fonction
      /* while(msg){
       msg ++;
       test_message(20>msg); 
      
      }*/
     
      if (returnValue != 0) { fprintf(stderr, "Erreur pthread_create premier thread\n");}
      
      
  
	// Create an interface: 
	// We can choose either a serial for real time operation, or file for previously logged data parsing
	// Note interface closing is also differentiated !
	//
	errorCode = sbgInterfaceSerialCreate(&sbgInterface, "/dev/ttyUSB0", 115200);		// Example for Unix using a FTDI Usb2Uart converter
	//errorCode = sbgInterfaceSerialCreate(&sbgInterface, "COM7", 115200);							// Example for Windows serial communication
 

	//
	// Test that the interface has been created
	//
	if (errorCode == SBG_NO_ERROR)
	{
		//
		// Create the sbgECom library and associate it with the created interfaces
		//
		errorCode = sbgEComInit(&comHandle, &sbgInterface);

		//
		// Test that the sbgECom has been initialized
		//
		if (errorCode == SBG_NO_ERROR)
		{
			//
			// Get device inforamtions
			//
			errorCode = sbgEComCmdGetInfo(&comHandle, &deviceInfo);

			//
			// Display device information if no error
			//
			if (errorCode == SBG_NO_ERROR)
			{
				printf("Device : %u found\n", deviceInfo.serialNumber);
			}
			else
			{
				fprintf(stderr, "ellipseMinimal: Unable to get device information.\n");
			}

			//
			// Configure some output logs to 25 Hz
			//
			if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_IMU_DATA, SBG_ECOM_OUTPUT_MODE_NEW_DATA) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_IMU_DATA.\n");
			}
			if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_EKF_EULER, SBG_ECOM_OUTPUT_MODE_MAIN_LOOP ) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_EKF_EULER.\n");
			}
      if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_PRESSURE, SBG_ECOM_OUTPUT_MODE_DIV_2 ) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_PRESSURE.\n");
			}
      if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_MAG, SBG_ECOM_OUTPUT_MODE_DIV_2 ) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_MAG.\n");
			}
      if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_GPS1_POS, SBG_ECOM_OUTPUT_MODE_NEW_DATA) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_GPS1_POS.\n");
			}
      if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_GPS1_VEL, SBG_ECOM_OUTPUT_MODE_NEW_DATA) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_GPS1_VEL.\n");
			}
	  if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_EKF_NAV, SBG_ECOM_OUTPUT_MODE_MAIN_LOOP) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_EKF_NAV.\n");
			}
			
			//
			// Display a message for real time data display
			//
			printf("sbgECom properly Initialized.\n");
			printf("sbgECom version %s\n\n", SBG_E_COM_VERSION_STR);

			//
			// Define callbacks for received data
			//
			sbgEComSetReceiveLogCallback(&comHandle, onLogReceived, NULL);

			//
			// Loop until the user exist
			//
    }
            	
	  else
	  {
			//
			// Unable to initialize the sbgECom
			//
			fprintf(stderr, "ellipseMinimal: Unable to initialize the sbgECom library.\n");
			retValue = -1;
	  }

    // Initialise le thread d'envoie de message
			while (1)
			{
				//
				// Try to read a frame
				//
        //printf("rentré\n");
				errorCode = sbgEComHandle(&comHandle);

				//
				// Test if we have to release some CPU (no frame received)
				//
				if (errorCode == SBG_NOT_READY)
				{
					//
					// Release CPU
					//
					//sbgSleep(1);
				}
        else
				{
					fprintf(stderr, "Error\n");
				}
			}

			//
			// Close the sbgEcom library
			//
			sbgEComClose(&comHandle);
		
		  //
		  // Close the interface
		  //
		  sbgInterfaceSerialDestroy(&sbgInterface);		
	}
	else
	{
		//
		// Unable to create the interface
		//
		fprintf(stderr, "ellipseMinimal: Unable to create the interface.\n");
		retValue = -1;
	}

	//
	// Returns -1 if we have an error
	//
	return (unsigned char) retValue;
 
}







//Un thread qui fait l'envoie de message 

/**
 * @brief Fonction du thread pour envoyer les données vers l'obc
 * 
 * @date 31/03/2023
 * 
 * @author Team OBC (ENSSAT)
 * @param -> arg potentiel argument pour la communication par file de message
 */
void * envoie_message(){ 
  while(1){
  if(msg_EKF == 1){
    pthread_mutex_lock(&mutex_msg_EKF);
    /*printf("\nmsg EKF Euler Angles: %3.1f\t%3.1f\t%3.1f\tStd Dev:%3.1f\t%3.1f\t%3.1f\t Quaternion: %3.1f\t%3.1f\t%3.1f\t%3.1f\t timestamp : %u\t \n", 
				SBG_EKF.euler[0], SBG_EKF.euler[1], SBG_EKF.euler[2], 
				SBG_EKF.eulerStdDev[0], SBG_EKF.eulerStdDev[1], SBG_EKF.eulerStdDev[2],
        SBG_EKF.quaternion[0],SBG_EKF.quaternion[1],SBG_EKF.quaternion[2],SBG_EKF.quaternion[3],
        SBG_EKF.timeStamp);*/
    Envoie_data_EKF(SBG_EKF, 3,SAUVEGARDE);
    msg_EKF = 0;
    pthread_mutex_unlock(&mutex_msg_EKF);
  }
    if(msg_IMU == 1){
    pthread_mutex_lock(&mutex_msg_IMU);
    /*printf("\n msg IMU Acc(m.s^-2): %3.1f\t%3.1f\t%3.1f\tGyroscope(rad.s^-1):%3.1f\t%3.1f\t%3.1f\tTemp(C):%3.1f\tdeltaVelo(m.s^-2):%3.1f\t%3.1f\t%3.1f\tDeltaAngle(rad.s^-1):%3.1f\t%3.1f\t%3.1f\t timestamp: %u\t\n",
				SBGIMU.accelerometers[0],SBGIMU.accelerometers[1],SBGIMU.accelerometers[2],SBGIMU.gyroscopes[0],SBGIMU.gyroscopes[1],SBGIMU.gyroscopes[2],SBGIMU.temperature,SBGIMU.deltaVelocity[0],SBGIMU.deltaVelocity[1],SBGIMU.deltaVelocity[2],SBGIMU.deltaAngle[0],SBGIMU.deltaAngle[1],SBGIMU.deltaAngle[2],SBGIMU.timeStamp);*/
    Envoie_data_IMU(SBGIMU, 2, SAUVEGARDE);
    msg_IMU = 0;
    pthread_mutex_unlock(&mutex_msg_IMU);
  }
    if(msg_GPS_pos == 1){
    pthread_mutex_lock(&mutex_msg_GPS_pos);
    /*printf("\n GPS Pos: longitude %3.1f\t latitude %3.1f\t altitude %3.1f\t undulation %3.1f\t latitudeAccuracy %3.1f\t longitudeAccuracy%3.1f\t altitudeAccuracy%3.1f\t timeStamp %u\t\n", 
			SBGGPS_pos.longitude, SBGGPS_pos.latitude,SBGGPS_pos.altitude, SBGGPS_pos.undulation, SBGGPS_pos.latitudeAccuracy, SBGGPS_pos.longitudeAccuracy, SBGGPS_pos.altitudeAccuracy,SBGGPS_pos.timeStamp);*/
    Envoie_data_GPS_pos(SBGGPS_pos, 1, SAUVEGARDE);
    msg_GPS_pos = 0;
    pthread_mutex_unlock(&mutex_msg_GPS_pos);
  }
    if(msg_GPS_vel == 1){
    pthread_mutex_lock(&mutex_msg_GPS_vel);
    /*printf("\nGPS Velocity : %3.1f\t %3.1f\t %3.1f\t  Velocity Accuracy : %3.1f\t %3.1f\t %3.1f\t timeStamp : %u \n",
       SBGGPS_vel.velocity[0], SBGGPS_vel.velocity[1], SBGGPS_vel.velocity[2], SBGGPS_vel.velocityAcc[0],SBGGPS_vel.velocityAcc[1],SBGGPS_vel.velocityAcc[2],SBGGPS_vel.timeStamp );*/
    Envoie_data_GPS_vel(SBGGPS_vel, 4, SAUVEGARDE);
    msg_GPS_vel = 0;
    pthread_mutex_unlock(&mutex_msg_GPS_vel);
  }
  
  if(msg_Clock == 1){
    pthread_mutex_lock(&mutex_msg_Clock);
   /* printf("\n msg Clock Heure : %d:%d:%d:\t Date : %d/%d/%u\n",
       SBGClock.hour, SBGClock.minute, SBGClock.second, SBGClock.day, SBGClock.month, SBGClock.year);*/
    Envoie_data_CLOCK(SBGClock, 9, SAUVEGARDE);
    msg_Clock = 0;
    pthread_mutex_unlock(&mutex_msg_Clock);
  }
   if(msg_Pressure == 1){
    pthread_mutex_lock(&mutex_msg_Pressure);
   /*printf("\n msg pressure : %3.1f height %3.1f timeStamp %u\n",
       SBGPressure.pressure, SBGPressure.height, SBGPressure.timeStamp);*/
    Envoie_data_Pressure(SBGPressure, 5, SAUVEGARDE);
    msg_Pressure = 0;
    pthread_mutex_unlock(&mutex_msg_Pressure);
  }
   if(msg_Magnetometer == 1){
    pthread_mutex_lock(&mutex_msg_Magnetometer);
   /*printf("\n msg magnetometers %3.1f\t %3.1f\t %3.1f\t timeStamp %u  \n",
       SBGMagnetometer.magnetometers[0], SBGMagnetometer.magnetometers[1], SBGMagnetometer.magnetometers[2],SBGMagnetometer.timeStamp);*/
    Envoie_data_Magnetometers(SBGMagnetometer, 5, SAUVEGARDE);
    msg_Magnetometer = 0;
    pthread_mutex_unlock(&mutex_msg_Magnetometer);
  }
  if(msg_EKF_nav == 1){
    pthread_mutex_lock(&mutex_msg_EKF_nav);
   /*printf("\n msg magnetometers %3.1f\t %3.1f\t %3.1f\t timeStamp %u  \n",
       SBGMagnetometer.magnetometers[0], SBGMagnetometer.magnetometers[1], SBGMagnetometer.magnetometers[2],SBGMagnetometer.timeStamp);*/
    Envoie_data_EKF_nav(SBGEKF_nav, 1, SAUVEGARDE);
    msg_EKF_nav = 0;
    pthread_mutex_unlock(&mutex_msg_EKF_nav);
  }
  }
  exit(EXIT_FAILURE);
}


