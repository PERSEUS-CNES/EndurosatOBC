
#include <float.h>
#include <stdint.h>
#include <sys/socket.h>
#include <semaphore.h>
#include <pthread.h>
#include "Structure.h"

#define portOBC 3000
#define portRoulis 3001
#define portParafoil 3002
#define portSequenceur 3003

#define addrOBC "192.168.1.2"
#define addrRoulis "192.168.1.3"
#define addrParafoil "192.168.1.4"
#define addrSequenceur "192.168.1.5"



typedef enum{
    	OBC,
    	ROULIS,
    	PARAFOIL,
	SEQUENCEUR,
} Destinataire;


typedef struct __attribute__((packed, aligned(1))) trame_roulis {
	uint8_t StartID;
	uint32_t PacketCounter;
	uint32_t TIME_STAMP_NAV;
	float VELOCITY_N;
        float VELOCITY_E;
	float VELOCITY_D;
	uint32_t TIME_STAMP_EULER;
	float YAW;
	uint32_t TIME_STAMP_IMU;
	float GYRO_Z;
	uint8_t EndID;
	
} trame_roulis ;


typedef struct __attribute__((packed, aligned(1))) trame_parafoil {
	uint8_t StartID;
	uint32_t PacketCounter;
	uint32_t TIME_STAMP_NAV;
	float VELOCITY_N;
    	float VELOCITY_E;
	float VELOCITY_D;
	double LATITUDE;
	double LONGITUDE;
	double ALTITUDE;
	uint32_t TIME_STAMP_EULER;
	float YAW;
	uint8_t EndID;
	
} trame_parafoil ;

typedef struct __attribute__((packed, aligned(1))) trame_sequenceur {
	uint8_t StartID;
	uint32_t PacketCounter;
	uint32_t TIME_STAMP_NAV;
	float VELOCITY_N;
        float VELOCITY_E;
	float VELOCITY_D;
	double LATITUDE;
	double LONGITUDE;
	double ALTITUDE;
	uint32_t TIME_STAMP_EULER;
	float ROLL;
	float PITCH;
	float YAW;
	uint8_t EndID;
	
} trame_sequenceur ;

typedef struct trame_recep_parafoil{
	uint8_t StartID;
	uint32_t PacketCounter;
	uint8_t Status_Parafoil;
	uint8_t Status_Servo_1;
	uint8_t Status_Servo_2;
	uint8_t EndID;
	
} trame_recep_parafoil ;

typedef struct trame_recep_sequenceur{
	uint8_t StartID;
	uint32_t PacketCounter;
	uint8_t Status_Sequenceur;
	uint8_t EndID;
	
} trame_recep_sequenceur ;

typedef struct trame_recep_roulis{
	uint8_t StartID;
	uint32_t PacketCounter;
	uint8_t Status_Roulis;
	uint8_t EndID;
	
} trame_recep_roulis ;

/*typedef struct statusSS{
	uint8_t Status_Roulis;
	uint8_t Status_Parafoil;
	uint8_t Status_Servo_1;
	uint8_t Status_Servo_2;
	uint8_t Status_Sequenceur;
}statusSS;*/

typedef struct trame{
	trame_roulis Trame_roulis;
	trame_parafoil Trame_parafoil;
	trame_sequenceur Trame_sequenceur;
} trame;


typedef struct trame_recep{
	trame_recep_roulis Trame_recep_roulis;
	trame_recep_parafoil Trame_recep_parafoil;
	trame_recep_sequenceur Trame_recep_sequenceur;
} trame_recep;

/*int init_socket(uint16_t ,char * ,struct sockaddr_in * );
int init_socket_bind(uint16_t , char * ,struct sockaddr_in * )
void routine_thread_send(void *);
void sendt(Message * , Destinataire , int , trame);
void recevt(Destinataire , int , trame_recep, struct sockaddr_in *);*/
