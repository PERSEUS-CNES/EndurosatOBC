#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include "./Librairies/FilsSS.h"
#include "./Librairies/Structure.h"
#include "./Librairies/ReceptionDataFileMessage.h"
#include "./Librairies/VariableGlobale.h"
#include <errno.h>   // pour errno
#include <mqueue.h>
//server

int init_socket(uint16_t ,char * ,struct sockaddr_in * );
int init_socket_bind(uint16_t , char * ,struct sockaddr_in *);
void recevt(Destinataire , int , STATUS , 	struct sockaddr_in * );
void printBits(size_t const , void const * const );
void sendt(Message * , Destinataire , trame , int , struct sockaddr_in );
void routine_thread_send(void * );

int FilsSS(void){
	int socket_desc;
	struct sockaddr_in client_addr_ROULIS, client_addr_PARAFOIL, client_addr_SEQUENCEUR, server_addr;
	//char server_message[7], client_message[7];
	//socklen_t client_struct_length_ROULIS = sizeof(client_addr_ROULIS);
	STATUS status_;
	trame Trame;
	memset(&Trame,0,sizeof(trame));

	socket_desc = init_socket_bind(portOBC,addrOBC,&server_addr);

	/*sem_t * sem_roulis;
	pthread_t * thread_roulis;
	sem_init(&sem_roulis,0,0);

	void * arg_roulis[4] = {(void *) ROULIS, (void*) &socket_desc, (void*) &sem_roulis, (void *) &client_addr_ROULIS};
	printf("taille arg : %d\n",sizeof(arg_roulis));
	pthread_create(&thread_roulis, NULL, (*routine_thread_send), arg_roulis);*/
    
	while(true){
   
		
		//receive
		printf("while true je receptionne\n");
		recevt(ROULIS, socket_desc, status_, &client_addr_ROULIS);
		//recevt(PARAFOIL, socket_desc, status_, &client_addr_PARAFOIL);
		//recevt(SEQUENCEUR, socket_desc, status_, &client_addr_SEQUENCEUR);

		Envoie_data_Status(status_ , 3 , SAUVEGARDE);
		//Envoie_data_Status(status_ , 3 , EMETTEUR);
		
		printf("Received message from IP: %s and port: %i\n",
		inet_ntoa(client_addr_ROULIS.sin_addr), ntohs(client_addr_ROULIS.sin_port));
		/*printf("Received message from IP: %s and port: %i\n",
		inet_ntoa(client_addr_PARAFOIL.sin_addr), ntohs(client_addr_PARAFOIL.sin_port));*/
		/*printf("Received message from IP: %s and port: %i\n",
		inet_ntoa(client_addr_SEQUENCEUR.sin_addr), ntohs(client_addr_SEQUENCEUR.sin_port));*/

		/*sem_post(&sem_roulis);
		printf("sem_roulis : %d\n",sem_roulis);
		printf("j'ai donné la clé\n");*/


		Message  * data = malloc(sizeof(Message));
		*data = receptionSS() ;
		printf("Message reçu par SS : type = %d\n" , data->type);

		sendt(data, ROULIS, Trame, socket_desc, client_addr_ROULIS);
		//sendt(data, PARAFOIL, Trame, socket_desc, client_addr_PARAFOIL);
		//sendt(data, SEQUENCEUR, Trame, socket_desc, client_addr_SEQUENCEUR);
		
		//free(data);

	

	}

	// Close the socket:
	close(socket_desc);
    
	return 0;

}

void routine_thread_send(void * arg){
	/*printf("taille arg : %d\n",sizeof(* arg));
	Destinataire * destinataire = (Destinataire*) arg;
	printf("taille destinataire : %d\n", sizeof(*destinataire));
	int * socket_desc = (int*) arg + 5;
	printf("taille socket : %d\n", sizeof(*socket_desc));
	sem_t* dest = (sem_t*) arg + 9;
	printf("taille dest : %d\n", sizeof(*dest));
	struct sockaddr_in * client_addr = (struct sockaddr_in *) arg + 41;
	printf("taille clientaddr : %d\n", sizeof(*client_addr));
	trame Trame;
	while (true){
		printf("j'attend la clé\n");
		printf("dest : %d\n",*dest);
		sem_wait(dest);
		printf("je suis debloqué\n");

		Message  * data = malloc(sizeof(Message));
		*data = receptionSS() ;
		printf("Message reçu par SS : type = %d\n" , data->type);
   
   		//sendt(data, *destinataire, *socket_desc, Trame); ancien 
		//sendt(data, ROULIS, Trame, socket_desc, client_addr_ROULIS); marche dans le main
		sendt(data, *destinataire, Trame, *socket_desc, *client_addr);
		printf("j'ai envoyé des données\n");
		free(data);
	}*/
}

int init_socket(uint16_t port,char * adresse,struct sockaddr_in * server_addr){
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

int init_socket_bind(uint16_t port, char * adresse,struct sockaddr_in * server_addr){

    int socket_desc = init_socket(port,adresse,server_addr);

    if(bind(socket_desc, (struct sockaddr*)server_addr, sizeof(*server_addr)) < 0){
   		 printf("Couldn't bind to the port\n");
   		 return -1;
   	 }
     printf("Done with binding %d\n", port);
     //printf(".............................................................\n");


    return socket_desc;

}

void recevt(Destinataire destinataire, int socket_desc, STATUS Status, struct sockaddr_in * dest_addr){ //status -> structure enregistr�e
	socklen_t dest_struct_length = sizeof(*dest_addr);
	
	switch(destinataire){
		case ROULIS : ;	
			trame_recep_roulis trame_recep_roulis_;
			int nr = 7;
			char bufferr[7];
			// Receive client's message:
			if (recvfrom(socket_desc, (void *) bufferr, nr, 0,
				(struct sockaddr*)dest_addr, &dest_struct_length) < 0){
				printf("OBC - Couldn't receive roulis\n");

			}
			printf("OBC - receive roulis\n");
			
			//affiche en binaire le buffer
			printBits(7, (void *)bufferr); 
			printf("buffer : %s\n", bufferr);

			memcpy((void *) &(trame_recep_roulis_.StartID),(void *)(bufferr), 1);
			memcpy((void *) &(trame_recep_roulis_.PacketCounter),(void *)(bufferr + 1), 4);
			memcpy((void *) &(trame_recep_roulis_.Status_Roulis),(void *)(bufferr + 5), 1);
			memcpy((void *) &(trame_recep_roulis_.EndID),(void *)(bufferr + 6), 1);

			Status.Status_Roulis = (trame_recep_roulis_.Status_Roulis);
      		printf("status roulis : %d\n",	(Status.Status_Roulis));
										
			break;
			
		case PARAFOIL : ;	
			trame_recep_parafoil trame_recep_parafoil_;
			int np = 9;
			char bufferp[9];
			// Receive client's message:
			if (recvfrom(socket_desc, (void *) bufferp, np, 0,
				(struct sockaddr*)dest_addr, &dest_struct_length) < 0){
				printf("OBC - Couldn't receive parafoil\n");

			}
			printf("OBC - receive parafoil\n");
			//printBits(7, (void *)buffer); //affiche en binaire le buffer
			printf("buffer : %s\n", bufferp);

			memcpy((void *) &(trame_recep_parafoil_.StartID),(void *)(bufferp), 1);
			memcpy((void *) &(trame_recep_parafoil_.PacketCounter),(void *)(bufferp + 1), 4);
			memcpy((void *) &(trame_recep_parafoil_.Status_Parafoil),(void *)(bufferp + 5), 1);
			memcpy((void *) &(trame_recep_parafoil_.Status_Servo_1),(void *)(bufferp + 6), 1);
			memcpy((void *) &(trame_recep_parafoil_.Status_Servo_2),(void *)(bufferp + 7), 1);
			memcpy((void *) &(trame_recep_parafoil_.EndID),(void *)(bufferp + 8), 1);

			Status.Status_Parafoil = (trame_recep_parafoil_.Status_Parafoil);
      		printf("status parafoil : %d\n",(Status.Status_Parafoil));
			Status.Status_Servo_1 = (trame_recep_parafoil_.Status_Servo_1);
      		printf("status servo 1 : %d\n",(Status.Status_Servo_1));
			Status.Status_Servo_2 = (trame_recep_parafoil_.Status_Servo_2);
      		printf("status servo 2 : %d\n",(Status.Status_Servo_2));
					
			break;
		
		case SEQUENCEUR : ;
			trame_recep_sequenceur trame_recep_sequenceur_;
			int ns = 7;
			char buffers[7];
			// Receive client's message:
			if (recvfrom(socket_desc, (void *) buffers, ns, 0,
				(struct sockaddr*)dest_addr, &dest_struct_length) < 0){
				printf("OBC - Couldn't receive sequenceur\n");

			}
			printf("OBC - receive sequenceur\n");
			//printBits(7, (void *)buffer); //affiche en binaire le buffer
			printf("buffer : %s\n", buffers);

			memcpy((void *) &(trame_recep_sequenceur_.StartID),(void *)(buffers), 1);
			memcpy((void *) &(trame_recep_sequenceur_.PacketCounter),(void *)(buffers + 1), 4);
			memcpy((void *) &(trame_recep_sequenceur_.Status_Sequenceur),(void *)(buffers + 5), 1);
			memcpy((void *) &(trame_recep_sequenceur_.EndID),(void *)(buffers + 6), 1);

			Status.Status_Sequenceur = (trame_recep_sequenceur_.Status_Sequenceur);
      		printf("status sequenceur : %d\n",	(Status.Status_Sequenceur));
		
			break;
			
		default :;
			break;
		
	}
}

void sendt(Message * data, Destinataire destinataire, trame Trame, int socket_desc, struct sockaddr_in dest_addr){
	//socklen_t dest_struct_length = sizeof(*dest_addr);
	printf("je vais envoyer des données\n");
	switch(destinataire){
		case ROULIS : ;
			switch(data->type){
				case EKF_NAV_TYPE : ;
					EKF_nav ekf_nav = data->data.ekf_nav;
					Trame.Trame_roulis.TIME_STAMP_NAV = ekf_nav.timeStamp;
					Trame.Trame_roulis.VELOCITY_N = ekf_nav.velocity[0];
					Trame.Trame_roulis.VELOCITY_E = ekf_nav.velocity[1];
					Trame.Trame_roulis.VELOCITY_D = ekf_nav.velocity[2];
					break;
				case EKF_TYPE : ;
					EKF ekf = data->data.ekf;
					Trame.Trame_roulis.TIME_STAMP_EULER = ekf.timeStamp;
					Trame.Trame_roulis.YAW = ekf.euler[2];
					break;				
				case IMU_TYPE : ;
					IMU imu = data->data.imu;
					Trame.Trame_roulis.TIME_STAMP_IMU = imu.timeStamp;
					Trame.Trame_roulis.GYRO_Z = imu.gyroscopes[2];
					break;
				default : ;
					printf("pas le bon\n");
					break;
			}
			int nr = 38;
			//char bufferr[38];
			//memset(&bufferr, 0, nr);

			Trame.Trame_roulis.StartID = 170; //0xAA
			Trame.Trame_roulis.PacketCounter = 892613426;//808464432;
			Trame.Trame_roulis.EndID = 255; //0xFF

			//Trame.Trame_roulis.TIME_STAMP_NAV = 892613426;
			//Trame.Trame_roulis.VELOCITY_N = 55.3; //
			//Trame.Trame_roulis.VELOCITY_E = 54.8; //
			//Trame.Trame_roulis.VELOCITY_D = 51.5; //
			/*Trame.Trame_roulis.TIME_STAMP_EULER = 892613426;
			Trame.Trame_roulis.YAW = 52.3; //
			Trame.Trame_roulis.TIME_STAMP_IMU = 892613426;
			Trame.Trame_roulis.GYRO_Z = 56.4; //*/
			
			//printf("velocity : %f\n", Trame.Trame_roulis.VELOCITY_N);

			//printBits(38, (void *)&Trame.Trame_roulis);

			/*memcpy((void *)bufferr, (void *) &Trame.Trame_roulis.StartID, 1);
			memcpy((void *)(bufferr + 1), (void *) &Trame.Trame_roulis.PacketCounter, 4);
			memcpy((void *)(bufferr + 5), (void *) &Trame.Trame_roulis.TIME_STAMP_NAV, 4);
			memcpy((void *)(bufferr + 9), (void *) &Trame.Trame_roulis.VELOCITY_N, 4);
			memcpy((void *)(bufferr + 13), (void *) &Trame.Trame_roulis.VELOCITY_E, 4);
			memcpy((void *)(bufferr + 17), (void *) &Trame.Trame_roulis.VELOCITY_D, 4);
			memcpy((void *)(bufferr + 21), (void *) &Trame.Trame_roulis.TIME_STAMP_EULER, 4);
			memcpy((void *)(bufferr + 25), (void *) &Trame.Trame_roulis.YAW, 4);
			memcpy((void *)(bufferr + 29), (void *) &Trame.Trame_roulis.TIME_STAMP_IMU, 4);
			memcpy((void *)(bufferr + 33), (void *) &Trame.Trame_roulis.GYRO_Z, 4);
			memcpy((void *)(bufferr + 37), (void *) &Trame.Trame_roulis.EndID, 1);*/

			if (sendto(socket_desc, (void *)&Trame.Trame_roulis, nr, 0,
			(struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0){
				printf("OBC - Can't send to roulis\n");
				
			}
			printf("OBC - send to roulis\n");

			//printBits(38, (void *)bufferr);

			printf("buffer envoi : %s\n", &Trame.Trame_roulis);

			break;
		
		case PARAFOIL : ;

			switch(data->type){
				case EKF_NAV_TYPE : ;
					EKF_nav ekf_nav = data->data.ekf_nav;
					Trame.Trame_parafoil.TIME_STAMP_NAV = ekf_nav.timeStamp;
					Trame.Trame_parafoil.VELOCITY_N = ekf_nav.velocity[0];
					Trame.Trame_parafoil.VELOCITY_E = ekf_nav.velocity[1];
					Trame.Trame_parafoil.VELOCITY_D = ekf_nav.velocity[2];
					Trame.Trame_parafoil.LATITUDE = ekf_nav.position[0];
					Trame.Trame_parafoil.LONGITUDE = ekf_nav.position[1];
					Trame.Trame_parafoil.ALTITUDE = ekf_nav.position[2];
					break;
				case EKF_TYPE : ;
					EKF ekf = data->data.ekf;
					Trame.Trame_parafoil.TIME_STAMP_EULER = ekf.timeStamp;
					Trame.Trame_parafoil.YAW = ekf.euler[2];
					break;				
				default : ;
					printf("pas le bon\n");
					break;
			}
			int np = 54;

			Trame.Trame_parafoil.StartID = 170; //0xAA
			Trame.Trame_parafoil.PacketCounter = 892613426;//808464432;
			Trame.Trame_parafoil.EndID = 255; //0xFF

			if (sendto(socket_desc, (void *)&Trame.Trame_parafoil, np, 0,
			(struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0){
				printf("OBC - Can't send to parafoil\n");
				
			}
			printf("OBC - send to parafoil\n");

			printf("buffer envoi : %s\n", &Trame.Trame_parafoil);

			break;

		case SEQUENCEUR : ;

			switch(data->type){
				case EKF_NAV_TYPE : ;
					EKF_nav ekf_nav = data->data.ekf_nav;
					Trame.Trame_sequenceur.TIME_STAMP_NAV = ekf_nav.timeStamp;
					Trame.Trame_sequenceur.VELOCITY_N = ekf_nav.velocity[0];
					Trame.Trame_sequenceur.VELOCITY_E = ekf_nav.velocity[1];
					Trame.Trame_sequenceur.VELOCITY_D = ekf_nav.velocity[2];
					Trame.Trame_sequenceur.LATITUDE = ekf_nav.position[0];
					Trame.Trame_sequenceur.LONGITUDE = ekf_nav.position[1];
					Trame.Trame_sequenceur.ALTITUDE = ekf_nav.position[2];
					break;
				case EKF_TYPE : ;
					EKF ekf = data->data.ekf;
					Trame.Trame_sequenceur.TIME_STAMP_EULER = ekf.timeStamp;
					Trame.Trame_sequenceur.ROLL = ekf.euler[0];
					Trame.Trame_sequenceur.PITCH = ekf.euler[1];
					Trame.Trame_sequenceur.YAW = ekf.euler[2];
					break;				
				default : ;
					printf("pas le bon\n");
					break;
			}
			int ns = 62;

			Trame.Trame_sequenceur.StartID = 170; //0xAA
			Trame.Trame_sequenceur.PacketCounter = 892613426;//808464432;
			Trame.Trame_sequenceur.EndID = 255; //0xFF

			if (sendto(socket_desc, (void *)&Trame.Trame_sequenceur, ns, 0,
			(struct sockaddr*)&dest_addr, sizeof(dest_addr)) < 0){
				printf("OBC - Can't send to sequenceur\n");
				
			}
			printf("OBC - send to sequenceur\n");

			printf("buffer envoi : %s\n", &Trame.Trame_sequenceur);

			break;

		default : ;
			break;

	}
}


	
void printBits(size_t const size, void const * const ptr)
{
    unsigned char * b = (unsigned char *) ptr;
    unsigned char byte;
    int i, j;
	printf("tramebit : \n");
    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
		printf("\n");
    }
    puts("");
	printf("\n");
}
