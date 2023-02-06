#include"centrale.h"

struct Clock SBGClock;
struct EKF_Euler SBGEKF_Euler;
struct EKF_Quat SBGEKF_Quat;
struct Pressure  SBGPressure;
struct IMU  SBGIMU;
struct GPS_vel  SBGGPS_vel;
struct GPS_pos  SBGGPS_pos;
struct Magnetometer SBGMagnetometer;
unsigned char msg_courant;

SbgErrorCode onLogReceived(SbgEComHandle *pHandle, SbgEComClass msgClass, SbgEComMsgId msg, const SbgBinaryLogData *pLogData, void *pUserArg)
{
	//
	// Handle separately each received data according to the log ID
	//
 msg_courant = msg;
	switch (msg)
	{
	case SBG_ECOM_LOG_EKF_EULER:
		//
		// Simply display euler angles in real time
		//
		printf("Euler Angles: %3.1f\t%3.1f\t%3.1f\tStd Dev:%3.1f\t%3.1f\t%3.1f   \r", 
				sbgRadToDegF(pLogData->ekfEulerData.euler[0]), sbgRadToDegF(pLogData->ekfEulerData.euler[1]), sbgRadToDegF(pLogData->ekfEulerData.euler[2]), 
				sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[0]), sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[1]), sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[2]));
    //
    // Transfers the data to our own structure so that we can save it later on
    //
    SBGEKF_Euler.euler[0] = pLogData->ekfEulerData.euler[0];
    SBGEKF_Euler.euler[1] = pLogData->ekfEulerData.euler[1]; 
    SBGEKF_Euler.euler[2] = pLogData->ekfEulerData.euler[2];
    SBGEKF_Euler.eulerStdDev[0] =  sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[0]);
    SBGEKF_Euler.eulerStdDev[1] =  sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[1]);
    SBGEKF_Euler.eulerStdDev[2] =  sbgRadToDegF(pLogData->ekfEulerData.eulerStdDev[2]);
    SBGEKF_Euler.timeStamp = pLogData->ekfEulerData.timeStamp;
		break;
   
    case 	SBG_ECOM_LOG_EKF_QUAT:
    SBGEKF_Quat.quaternion[0] =   pLogData->ekfQuatData.quaternion[0];
    SBGEKF_Quat.quaternion[1] =   pLogData->ekfQuatData.quaternion[1];
    SBGEKF_Quat.quaternion[2] =   pLogData->ekfQuatData.quaternion[2];
    SBGEKF_Quat.quaternion[3] =   pLogData->ekfQuatData.quaternion[3];
    SBGEKF_Quat.timeStamp = pLogData->ekfQuatData.timeStamp;
    break;
    
   case SBG_ECOM_LOG_PRESSURE:
		printf("\n Pressure: %3.1f\t%3.1f \r", 
				pLogData->pressureData.pressure, pLogData->pressureData.height);
    //
    // Transfers the data to our own structure so that we can save it later on
    //
        SBGPressure.pressure = pLogData->pressureData.pressure;
        SBGPressure.height = pLogData->pressureData.height;
        SBGPressure.timeStamp = pLogData->pressureData.timeStamp;
		break;

   case SBG_ECOM_LOG_IMU_DATA:
   
   
     printf("timestamp: %u\tAcc(m.s^-2): %3.1f\t%3.1f\t%3.1f\tGyroscope(rad.s^-1):%3.1f\t%3.1f\t%3.1f\tTemp(C):%3.1f\tdeltaVelo(m.s^-2):%3.1f\t%3.1f\t%3.1f\tDeltaAngle(rad.s^-1):%3.1f\t%3.1f\t%3.1f\t\r",
				pLogData->imuData.timeStamp,pLogData->imuData.accelerometers[0],pLogData->imuData.accelerometers[1],pLogData->imuData.accelerometers[2],pLogData->imuData.gyroscopes[0],pLogData->imuData.gyroscopes[1],pLogData->imuData.gyroscopes[2],pLogData->imuData.temperature,pLogData->imuData.deltaVelocity[0],pLogData->imuData.deltaVelocity[1],pLogData->imuData.deltaVelocity[2],pLogData->imuData.deltaAngle[0],pLogData->imuData.deltaAngle[1],pLogData->imuData.deltaAngle[2]);
   //
   //  Transfers the data to our own structure so that we can save it later on
   //
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
   break;
   
   case SBG_ECOM_LOG_MAG:
     printf("\n Magneto: %3.1f\t%3.1f\t%3.1f\t AccMag: %3.1f\t%3.1f\t%3.1f\t \r", 
				pLogData->magData.magnetometers[0], pLogData->magData.magnetometers[1],pLogData->magData.magnetometers[2], pLogData->magData.accelerometers[0], pLogData->magData.magnetometers[1], pLogData->magData.magnetometers[2]);
  SBGMagnetometer.magnetometers[0] = pLogData->magData.magnetometers[0];
  SBGMagnetometer.magnetometers[1] = pLogData->magData.magnetometers[1];
  SBGMagnetometer.magnetometers[2] = pLogData->magData.magnetometers[2];
  SBGMagnetometer.timeStamp = pLogData->magData.timeStamp;
  break;
  
  //Ne fonctionne pas car pas d'antenne GPS branchée      
   case SBG_ECOM_LOG_GPS2_POS:
     printf("\n Latitude: %3.1f\t Longitude: %3.1f\t Altitude: %3.1f\t\r",
       pLogData->gpsPosData.latitude, pLogData->gpsPosData.longitude, pLogData->gpsPosData.altitude);
       SBGGPS_pos.latitude = pLogData->gpsPosData.latitude;
       SBGGPS_pos.longitude = pLogData->gpsPosData.longitude;
       SBGGPS_pos.altitude = pLogData->gpsPosData.altitude;
       SBGGPS_pos.latitudeAccuracy = pLogData->gpsPosData.latitudeAccuracy;
       SBGGPS_pos.longitudeAccuracy = pLogData->gpsPosData.longitudeAccuracy;
       SBGGPS_pos.altitudeAccuracy = pLogData->gpsPosData.altitudeAccuracy;
       SBGGPS_pos.undulation = pLogData->gpsPosData.undulation;
       SBGGPS_pos.timeStamp = pLogData->gpsPosData.timeStamp;
   break;
   case SBG_ECOM_LOG_GPS2_VEL:
     printf("\n Velocity : %3.1f\t %3.1f\t %3.1f\t\r",
       pLogData->gpsVelData.velocity[0], pLogData->gpsVelData.velocity[1], pLogData->gpsVelData.velocity[2]);
       
       SBGGPS_vel.velocity[0] = pLogData->gpsVelData.velocity[0];
       SBGGPS_vel.velocity[1] = pLogData->gpsVelData.velocity[1];
       SBGGPS_vel.velocity[2] = pLogData->gpsVelData.velocity[2];
       SBGGPS_vel.velocityAcc[0] = pLogData->gpsVelData.velocityAcc[0];
       SBGGPS_vel.velocityAcc[1] = pLogData->gpsVelData.velocityAcc[1];
       SBGGPS_vel.velocityAcc[2] = pLogData->gpsVelData.velocityAcc[2];
       SBGGPS_vel.timeStamp = pLogData->gpsVelData.timeStamp;
    break;
   //Probleme format   
   case SBG_ECOM_LOG_UTC_TIME:
     printf("\n Heure : %d:%d:%d:%ld\t Date : %d/%d/%lu\r",
       pLogData->utcData.hour, pLogData->utcData.minute, pLogData->utcData.second, pLogData->utcData.nanoSecond, pLogData->utcData.day, pLogData->utcData.month, pLogData->utcData.year);
  
  SBGClock.year = pLogData->utcData.year;
  SBGClock.month = pLogData->utcData.month;
  SBGClock.day = pLogData->utcData.day;
  SBGClock.hour = pLogData->utcData.hour;
  SBGClock.minute = pLogData->utcData.minute;
  SBGClock.second = pLogData->utcData.second;
  SBGClock.nanoSeconds = pLogData->utcData.nanoSecond;
   break;
   //////////////////
	default:
		break;
	}
	
	return SBG_NO_ERROR;
}

unsigned char main_centrale(){
  SbgEComHandle			comHandle;
	SbgErrorCode			errorCode;
	SbgInterface			sbgInterface;
	int32					retValue = 0;
	SbgEComDeviceInfo		deviceInfo;

	//
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
				printf("Device : %0.9u found\n", deviceInfo.serialNumber);
			}
			else
			{
				fprintf(stderr, "ellipseMinimal: Unable to get device information.\n");
			}

			//
			// Configure some output logs to 25 Hz
			//
			if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_IMU_DATA, SBG_ECOM_OUTPUT_MODE_DIV_8) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_IMU_DATA.\n");
			}
			if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_EKF_EULER, SBG_ECOM_OUTPUT_MODE_DIV_8) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_EKF_EULER.\n");
			}
      if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_PRESSURE, SBG_ECOM_OUTPUT_MODE_DIV_8) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_PRESSURE.\n");
			}
      if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_MAG, SBG_ECOM_OUTPUT_MODE_DIV_8) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_MAG.\n");
			}
      if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_GPS2_POS, SBG_ECOM_OUTPUT_MODE_DIV_8) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_GPS2_POS.\n");
			}
      if (sbgEComCmdOutputSetConf(&comHandle, SBG_ECOM_OUTPUT_PORT_A, SBG_ECOM_CLASS_LOG_ECOM_0, SBG_ECOM_LOG_GPS2_VEL, SBG_ECOM_OUTPUT_MODE_DIV_8) != SBG_NO_ERROR)
			{
				fprintf(stderr, "ellipseMinimal: Unable to configure output log SBG_ECOM_LOG_GPS2_VEL.\n");
			}
			
			//
			// Display a message for real time data display
			//
			printf("sbgECom properly Initialized.\n");
			printf("sbgECom version %s\n\n", SBG_E_COM_VERSION_STR);
			printf("Euler Angles display with estimated standard deviation.\n");

			//
			// Define callbacks for received data
			//
			sbgEComSetReceiveLogCallback(&comHandle, onLogReceived, NULL);

			//
			// Loop until the user exist
			//
			while (1)
			{
				//
				// Try to read a frame
				//
				errorCode = sbgEComHandle(&comHandle);

				//
				// Test if we have to release some CPU (no frame received)
				//
				if (errorCode == SBG_NOT_READY)
				{
					//
					// Release CPU
					//
					sbgSleep(1);
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
		}
		else
		{
			//
			// Unable to initialize the sbgECom
			//
			fprintf(stderr, "ellipseMinimal: Unable to initialize the sbgECom library.\n");
			retValue = -1;
		}
		
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
	return retValue;
 
}