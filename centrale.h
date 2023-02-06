
#include "sbgEComLib.h"

unsigned char init_centrale(SbgEComHandle* comHandle, SbgErrorCode* errorCode, SbgInterface* sbgInterface, SbgEComDeviceInfo* deviceInfo);
unsigned char main_centrale(void);


typedef struct GPS_pos
{
  double latitude;
  double longitude;
  double altitude;
  float undulation;
  float latitudeAccuracy;
  float longitudeAccuracy;
  float altitudeAccuracy;
  unsigned int timeStamp; 
}GPS_pos;

typedef struct GPS_vel
{
  float velocity[3];
  float velocityAcc[3];
  unsigned int timeStamp;
}GPS_vel;

typedef struct IMU
{
  float accelerometers[3];
  float gyroscopes[3];
  float temperature;
  float deltaVelocity[3];
  float deltaAngle[3];
  unsigned int timeStamp;
}IMU;

typedef struct Magnetometer
{
  float magnetometers[3];
  unsigned int timeStamp;
}Magnetometer;

typedef struct Pressure
{
  float pressure;
  float height;
  unsigned int timeStamp;
}Pressure;

typedef struct EKF_Euler
{
  float euler[3];
  float eulerStdDev[3];
  unsigned int timeStamp;
}EKF_Euler;

typedef struct EKF_Quat
{
  float quaternion[4];
  unsigned int timeStamp;
}EKF_Quat;

typedef struct Clock
{
  unsigned short year;
  unsigned char  month;
  unsigned char day;
  unsigned char hour;
  unsigned char minute;
  unsigned char second;
  unsigned long int nanoSeconds;
}Clock;

extern struct Clock SBGClock;
extern struct EKF_Euler SBGEKF_Euler;
extern struct Pressure  SBGPressure;
extern struct IMU  SBGIMU;
extern struct GPS_vel  SBGGPS_vel;
extern struct GPS_pos  SBGGPS_pos;
extern struct EKF_Quat SBGEKF_Quat;
extern struct Magnetometer SBGMagnetometer;