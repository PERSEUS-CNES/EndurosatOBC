#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

int init_socket(uint16_t ,char * ,struct sockaddr_in * );
int init_socket_bind(uint16_t , char * );

int main(void){
	int socket_desc;
	struct sockaddr_in client_addr;
	char server_message[100], client_message[100];
	socklen_t client_struct_length = sizeof(client_addr);
	uint16_t port = 3000;
	char * adresse = "192.168.1.2";
    
   socket_desc = init_socket_bind(port,adresse);
    
	while(true){
   
	// Clean buffers:
	memset(server_message, '\0', sizeof(server_message));
	memset(client_message, '\0', sizeof(client_message));
    
    
    
    //message
    server_message[0] = 's';
    server_message[1] = 'e';
    
     // Receive client's message:
	if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
     	(struct sockaddr*)&client_addr, &client_struct_length) < 0){
    	printf("Couldn't receive\n");
    	return -1;
	}
	printf("Received message from IP: %s and port: %i\n",
       	inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	//switch suivant les clients
	printf("Msg from client: %s\n", client_message);
    
    /*client_addr.sin_family = AF_INET;
   	 client_addr.sin_port = htons(port);
   	 client_addr.sin_addr.s_addr = inet_addr("192.168.1.3");*/
    
    if (sendto(socket_desc, server_message, strlen(server_message), 0,
     	(struct sockaddr*)&client_addr, client_struct_length) < 0){
    	printf("\nCan't send\n");
    	return -1;
	}
    printf("send1\n");
    
    if (sendto(socket_desc, server_message, strlen(server_message), 0,
     	(struct sockaddr*)&client_addr, client_struct_length) < 0){
    	printf("\nCan't send\n");
    	return -1;
	}
    printf("send2\n");
    
    
	// Receive client's message:
	if (recvfrom(socket_desc, client_message, sizeof(client_message), 0,
     	(struct sockaddr*)&client_addr, &client_struct_length) < 0){
    	printf("Couldn't receive\n");
    	return -1;
	}
	printf("Received message from IP: %s and port: %i\n",
       	inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	//switch suivant les clients
	printf("Msg from client: %s\n", client_message);
	}

	// Close the socket:
	close(socket_desc);
    
	return 0;

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

int init_socket_bind(uint16_t port, char * adresse){
    struct sockaddr_in server_addr;
    int socket_desc = init_socket(port,adresse,&server_addr);
    
    if(bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
   		 printf("Couldn't bind to the port\n");
   		 return -1;
   	 }
    printf("Done with binding\n");


    return socket_desc;

}
