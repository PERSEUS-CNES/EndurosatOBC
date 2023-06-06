#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>
#include "FilsSS.h"
#include "./Librairies/Structure.h"
#include "./Librairies/ReceptionDataFileMessage.h"
#include "./Librairies/VariableGlobale.h"
#include <fcntl.h>

int FilsSS()
{
	trame_recep trame_recep_;
 //printf("ce %d %d .....\n", portRoulis, addrRoulis);
	
	//Roulis initialisation
	int socket_desc_roulis;
	sem_t * sem_roulis;
    	pthread_t * thread_roulis;
    	//socket_desc_roulis = init_socket_bind(portRoulis,addrRoulis);
    
    socket_desc_roulis = init_socket_bind(portOBC,addrOBC);
     
     int flag;
    flag = fcntl(socket_desc_roulis, F_GETFL);
    flag = flag | O_NONBLOCK;
    fcntl(socket_desc_roulis, F_SETFL, flag);
     
     printf("roulis init socket\n");
         
	  sem_init(&sem_roulis,0,0);
    printf("roulis sem init\n");
	void * arg_roulis[3] = {(void *) ROULIS, (void*) &socket_desc_roulis, (void*) &sem_roulis};
	pthread_create(&thread_roulis, NULL, (*routine_thread_send), arg_roulis);
	printf("roulis thread init\n");
	
	/*//Parafoil initialisation
	int socket_desc_parafoil;
	sem_t * sem_parafoil;
    	pthread_t * thread_parafoil;
    	socket_desc_parafoil = init_socket_bind(portOBC,addrOBC);
	sem_init(&sem_parafoil,0,0);
	void * arg_parafoil[3] = {(void*) PARAFOIL, (void*) &socket_desc_parafoil, (void*) &sem_parafoil};
	pthread_create(&thread_parafoil, NULL, (*routine_thread_send), arg_parafoil);
 printf("parafoil init\n");
	
	//Sequenceur initialisation
	int socket_desc_sequenceur;
	sem_t * sem_sequenceur;
    	pthread_t * thread_sequenceur;
    	socket_desc_sequenceur = init_socket_bind(portOBC,addrOBC);
	sem_init(&sem_sequenceur,0,0);
	void * arg_sequenceur[3] = {(void*) SEQUENCEUR, (void*) &socket_desc_sequenceur, (void*) &sem_sequenceur};
	pthread_create(&thread_sequenceur, NULL, (*routine_thread_send), arg_sequenceur);
 printf("sequenceur init\n");*/
	
    
	while(true){
   
		// Clean buffers:
		//memset(trame_recep_parafoil, '\0', sizeof(trame_recep_parafoil));
		//memset(trame_recep_roulis, '\0', sizeof(trame_recep_roulis));
		//memset(trame_recep_sequenceur, '\0', sizeof(trame_recep_sequenceur));
		printf("roulis recev go\n");
		recevt(ROULIS , socket_desc_roulis , trame_recep_); //trame recep roulis
    printf("roulis recev\n");
		sem_post(sem_roulis);
     printf("et là ?\n");

		
		/*recevt(PARAFOIL, socket_desc_parafoil, trame_recep_);
		sem_post(&sem_parafoil); 

		recevt(SEQUENCEUR, socket_desc_sequenceur, trame_recep_);
		sem_post(&sem_sequenceur); 		*/
		
	}
	
	// Close the socket:
	close(socket_desc_roulis);
	//close(socket_desc_parafoil);
	//close(socket_desc_sequenceur);
    
	return 0;

}

int init_socket(uint16_t port,char * adresse, struct sockaddr_in * server_addr){
    int socket_desc;
    
	// Create socket:
	socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    
	if(socket_desc < 0){
    	printf("Error while creating socket\n");
    	return -1;
	}
	printf("Socket created successfully\n");
    
	// Set port and IP:
	server_addr->sin_family = AF_INET;
	server_addr->sin_port = htons(port);
	server_addr->sin_addr.s_addr = inet_addr(adresse);
    
    return socket_desc;
}

int init_socket_bind(uint16_t port, char * adresse){
    struct sockaddr_in server_addr;
    int socket_desc = init_socket(port,adresse,&server_addr);
    
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
   		 printf("Couldn't bind to the port\n");
   		 //return -1;
   	 }
     printf("Done with binding %d\n", port);
     printf(".............................................................\n");


    return socket_desc;

}

	
	
void routine_thread_send(/*Destinataire destinataire, int socket_desc, sem_t * dest*/  void * arg){
	Destinataire * destinataire = (Destinataire*) arg;
	int * socket_desc = (int*) arg + 1;
	sem_t* dest = (sem_t*) arg + 2;
 printf("routine go\n");

	trame Trame;
	while (true){
		sem_wait(dest);
   printf("init recep\n");
		//memset(data, '\0', sizeof(data));
		Message  * data = malloc(sizeof(Message));
		*data = receptionSS() ;
   //int data = 2;
		printf("Message reçu par SS : type = %d\n" , data);
		
		sendt(data, *destinataire, *socket_desc, Trame);
   printf("j'envoi un message ?\n");
		free(data);
	}
}


		
		
void sendt(Message * data, Destinataire destinataire, int socket_desc, trame trame_){
	struct sockaddr_in dest_addr;
	socklen_t dest_struct_length;
	dest_addr.sin_family = AF_INET;
 //printf("sendt go\n");
	
	switch(destinataire){
		case OBC : ;
		
			break;
		
		case ROULIS : ;		
			dest_addr.sin_port = htons(portRoulis);
			dest_addr.sin_addr.s_addr = inet_addr(addrRoulis);
			dest_struct_length = sizeof(dest_addr);
			
			switch(data->type){
					case EKF_NAV_TYPE : ;
						EKF_nav ekf_nav = data->data.ekf_nav;
						trame_.Trame_roulis.TIME_STAMP_NAV = ekf_nav.timeStamp;
						trame_.Trame_roulis.VELOCITY_N = ekf_nav.velocity[0];
						trame_.Trame_roulis.VELOCITY_E = ekf_nav.velocity[1];
						trame_.Trame_roulis.VELOCITY_D = ekf_nav.velocity[2];
						break;
					case EKF_TYPE : ;
						EKF ekf = data->data.ekf;
						trame_.Trame_roulis.TIME_STAMP_EULER = ekf.timeStamp;
						trame_.Trame_roulis.YAW = ekf.euler[2];
						break;
					
					case IMU_TYPE : ;
						IMU imu = data->data.imu;
						trame_.Trame_roulis.TIME_STAMP_IMU = imu.timeStamp;
						trame_.Trame_roulis.GYRO_Z = imu.gyroscopes[2];
						break;
					default : ;
						printf("pas le bon");
						break;
			}
			//STARTID ENDID PACKET à faire .....................................
			
      printf("envoi ??\n");
      
			if (sendto(socket_desc, (void *)&trame_.Trame_roulis, sizeof(trame_.Trame_roulis), 0,
			(struct sockaddr*)&dest_addr, dest_struct_length) < 0){
				printf("OBC - Can't send to roulis\n");
				
			}
			printf("OBC - send to roulis\n");
					
			
			break;
			
		case PARAFOIL : ;	
			dest_addr.sin_port = htons(portParafoil);
			dest_addr.sin_addr.s_addr = inet_addr(addrParafoil);
			dest_struct_length = sizeof(dest_addr);
			
			switch(data->type){
					case EKF_NAV_TYPE : ;
						EKF_nav ekf_nav = data->data.ekf_nav;
						trame_.Trame_parafoil.TIME_STAMP_NAV = ekf_nav.timeStamp;
						trame_.Trame_parafoil.VELOCITY_N = ekf_nav.velocity[0];
						trame_.Trame_parafoil.VELOCITY_E = ekf_nav.velocity[1];
						trame_.Trame_parafoil.VELOCITY_D = ekf_nav.velocity[2];
						trame_.Trame_parafoil.LATITUDE = ekf_nav.position[0];
						trame_.Trame_parafoil.LONGITUDE = ekf_nav.position[1];
						trame_.Trame_parafoil.ALTITUDE = ekf_nav.position[2];
						break;
					case EKF_TYPE : ;
						EKF ekf = data->data.ekf;
						trame_.Trame_parafoil.TIME_STAMP_EULER = ekf.timeStamp;
						trame_.Trame_parafoil.YAW = ekf.euler[2]; 
						break;
					default : ;
						printf("pas le bon");
						break;
			}
			//STARTID ENDID PACKET à faire .....................................
			
			if (sendto(socket_desc, (void *)&trame_.Trame_parafoil, sizeof(trame_.Trame_parafoil), 0,
			(struct sockaddr*)&dest_addr, dest_struct_length) < 0){
				printf("OBC - Can't send to parafoil\n");
	
			}
			printf("OBC - send to parafoil\n");
		
			break;
		
		case SEQUENCEUR : ;
			dest_addr.sin_port = htons(portSequenceur);
			dest_addr.sin_addr.s_addr = inet_addr(addrSequenceur);
			dest_struct_length = sizeof(dest_addr);
			
			switch(data->type){
					case EKF_NAV_TYPE : ;
						EKF_nav ekf_nav = data->data.ekf_nav;
						trame_.Trame_sequenceur.TIME_STAMP_NAV = ekf_nav.timeStamp;
						trame_.Trame_sequenceur.VELOCITY_N = ekf_nav.velocity[0];
						trame_.Trame_sequenceur.VELOCITY_E = ekf_nav.velocity[1];
						trame_.Trame_sequenceur.VELOCITY_D = ekf_nav.velocity[2];
						trame_.Trame_sequenceur.LATITUDE = ekf_nav.position[0];
						trame_.Trame_sequenceur.LONGITUDE = ekf_nav.position[1];
						trame_.Trame_sequenceur.ALTITUDE = ekf_nav.position[2];
						break;
					case EKF_TYPE : ;
						EKF ekf = data->data.ekf;
						trame_.Trame_sequenceur.TIME_STAMP_EULER = ekf.timeStamp;
						trame_.Trame_sequenceur.ROLL = ekf.euler[0]; 
						trame_.Trame_sequenceur.PITCH = ekf.euler[1]; 
						trame_.Trame_sequenceur.YAW = ekf.euler[2]; 
						break;
					default : ;
						printf("pas le bon");
						break;
			}
			//STARTID ENDID PACKET à faire .....................................
			
			if (sendto(socket_desc, (void *)&trame_.Trame_sequenceur, sizeof(trame_.Trame_sequenceur), 0,
			(struct sockaddr*)&dest_addr, dest_struct_length) < 0){
				printf("OBC - Can't send to sequenceur\n");
			}
			printf("OBC - send to sequenceur\n");
		
			break;
			
		default :;
			break;
		
	}
	
	
}

void recevt(Destinataire destinataire, int socket_desc, trame_recep trame_recep_){ //status -> structure enregistrée
	struct sockaddr_in dest_addr;
	socklen_t dest_struct_length;
	status Status;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	printf("je rego recev\n");
	
	switch(destinataire){
		case OBC : ;
		
			break;
		
		case ROULIS : ;	
			dest_addr.sin_port = htons(portRoulis);
			dest_struct_length = sizeof(dest_addr);
      printf("initi recev roulis\n");
			
			// Receive client's message:
			if (recvfrom(socket_desc, (void *) &trame_recep_.Trame_recep_roulis, sizeof(trame_recep_.Trame_recep_roulis), 0,
				(struct sockaddr*)&dest_addr, &dest_struct_length) < 0){
				printf("OBC - Couldn't receive roulis\n");

			}
			printf("OBC - receive roulis\n");
			
			//STARTID ENDID PACKET à faire .....................................
			//memcpy à faire ...................................
			Status.Status_Roulis = trame_recep_.Trame_recep_roulis.Status_Roulis;
      printf("%d\n",	Status.Status_Roulis);
										
			
			break;
			
		case PARAFOIL : ;	
			dest_addr.sin_port = htons(portParafoil);
			dest_struct_length = sizeof(dest_addr);
			
			// Receive client's message:
			if (recvfrom(socket_desc, (void *) &trame_recep_.Trame_recep_parafoil, sizeof(trame_recep_.Trame_recep_parafoil), 0,
				(struct sockaddr*)&dest_addr, &dest_struct_length) < 0){
				printf("OBC - Couldn't receive parafoil\n");
			}
			printf("OBC - receive prafoil\n");
			
			//STARTID ENDID PACKET à faire .....................................
			//memcpy à faire ...................................
			Status.Status_Parafoil = trame_recep_.Trame_recep_parafoil.Status_Parafoil;
			Status.Status_Servo_1 = trame_recep_.Trame_recep_parafoil.Status_Servo_1;
			Status.Status_Servo_2 = trame_recep_.Trame_recep_parafoil.Status_Servo_2;
		
			break;
		
		case SEQUENCEUR : ;
			dest_addr.sin_port = htons(portSequenceur);
			dest_struct_length = sizeof(dest_addr);
			
			// Receive client's message:
			if (recvfrom(socket_desc, (void*) &trame_recep_.Trame_recep_sequenceur, sizeof(trame_recep_.Trame_recep_sequenceur), 0,
				(struct sockaddr*)&dest_addr, &dest_struct_length) < 0){
				printf("OBC - Couldn't receive sequenceur\n");
			}
			printf("OBC - receive requenceur\n");
			
			//STARTID ENDID PACKET à faire .....................................
			//memcpy à faire ...................................
			Status.Status_Sequenceur = trame_recep_.Trame_recep_sequenceur.Status_Sequenceur;
		
			break;
			
		default :;
			break;
		
	}
	
}
