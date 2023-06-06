#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "./Librairies/FilsSS.h"

#define PORT_PARAFOIL 3000
#define PORT_ROULIS 3001
#define PORT_SEQUENCEUR 3002

#define ADDR_PARAFOIL "192.168.1.2"
#define ADDR_ROULIS "192.168.1.3"
#define ADDR_SEQUENCEUR "192.168.1.4"

#define MAX_MESSAGE_SIZE 1024

typedef struct {
    float parafoilValue;
} ParafoilData;

typedef struct {
    float roulisValue;
} RoulisData;

typedef struct {
    int sequenceurValue;
} SequenceurData;

int parafoilSocket, roulisSocket, sequenceurSocket;
pthread_mutex_t obcMutex;

void* obcSendData(void* arg) {
    struct sockaddr_in parafoilAddr, roulisAddr, sequenceurAddr;
    int parafoilConnected = 0, roulisConnected = 0, sequenceurConnected = 0;

    // Création des sockets UDP pour envoyer les données aux sous-systèmes
    parafoilSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (parafoilSocket < 0) {
        perror("Erreur lors de la création du socket pour le Parafoil");
        exit(1);
    }

    roulisSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (roulisSocket < 0) {
        perror("Erreur lors de la création du socket pour le Roulis");
        exit(1);
    }

    sequenceurSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (sequenceurSocket < 0) {
        perror("Erreur lors de la création du socket pour le Séquenceur");
        exit(1);
    }

    memset((char *)&parafoilAddr, 0, sizeof(parafoilAddr));
    parafoilAddr.sin_family = AF_INET;
    parafoilAddr.sin_addr.s_addr = inet_addr(ADDR_PARAFOIL);
    parafoilAddr.sin_port = htons(PORT_PARAFOIL);

    memset((char *)&roulisAddr, 0, sizeof(roulisAddr));
    roulisAddr.sin_family = AF_INET;
    roulisAddr.sin_addr.s_addr = inet_addr(ADDR_ROULIS);
    roulisAddr.sin_port = htons(PORT_ROULIS);

    memset((char *)&sequenceurAddr, 0, sizeof(sequenceurAddr));
    sequenceurAddr.sin_family = AF_INET;
    sequenceurAddr.sin_addr.s_addr = inet_addr(ADDR_SEQUENCEUR);
    sequenceurAddr.sin_port = htons(PORT_SEQUENCEUR);

    while (1) {
        pthread_mutex_lock(&obcMutex);

        // Exemple de génération des données à envoyer aux sous-systèmes
        float parafoilValueToSend = 1.23;
        float roulisValueToSend = 4.56;
        int sequenceurValueToSend = 7;

        // Envoi des données au Parafoil
        ssize_t sentBytes = sendto(parafoilSocket, &parafoilValueToSend, sizeof(parafoilValueToSend), 0, (struct sockaddr *)&parafoilAddr, sizeof(parafoilAddr));
        if (sentBytes < 0) {
            perror("Erreur lors de l'envoi des données au Parafoil");
            exit(1);
        }

        // Envoi des données au Roulis
        sentBytes = sendto(roulisSocket, &roulisValueToSend, sizeof(roulisValueToSend), 0, (struct sockaddr *)&roulisAddr, sizeof(roulisAddr));
        if (sentBytes < 0) {
            perror("Erreur lors de l'envoi des données au Roulis");
            exit(1);
        }

        // Envoi des données au Séquenceur
        sentBytes = sendto(sequenceurSocket, &sequenceurValueToSend, sizeof(sequenceurValueToSend), 0, (struct sockaddr *)&sequenceurAddr, sizeof(sequenceurAddr));
        if (sentBytes < 0) {
            perror("Erreur lors de l'envoi des données au Séquenceur");
            exit(1);
        }

        pthread_mutex_unlock(&obcMutex);

        usleep(1000000); // Attente d'une seconde avant d'envoyer les prochaines données
    }

    close(parafoilSocket);
    close(roulisSocket);
    close(sequenceurSocket);
    pthread_exit(NULL);
}

void* obcReceiveData(void* arg) {
    struct sockaddr_in parafoilAddr, roulisAddr, sequenceurAddr;
    socklen_t addrLen = sizeof(parafoilAddr);

    // Création des sockets UDP pour recevoir les données des sous-systèmes
    parafoilSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (parafoilSocket < 0) {
        perror("Erreur lors de la création du socket pour recevoir les données du Parafoil");
        exit(1);
    }

    roulisSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (roulisSocket < 0) {
        perror("Erreur lors de la création du socket pour recevoir les données du Roulis");
        exit(1);
    }

    sequenceurSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (sequenceurSocket < 0) {
        perror("Erreur lors de la création du socket pour recevoir les données du Séquenceur");
        exit(1);
    }

    memset((char *)&parafoilAddr, 0, sizeof(parafoilAddr));
    parafoilAddr.sin_family = AF_INET;
    parafoilAddr.sin_addr.s_addr = INADDR_ANY;
    parafoilAddr.sin_port = htons(PORT_PARAFOIL);

    memset((char *)&roulisAddr, 0, sizeof(roulisAddr));
    roulisAddr.sin_family = AF_INET;
    roulisAddr.sin_addr.s_addr = INADDR_ANY;
    roulisAddr.sin_port = htons(PORT_ROULIS);

    memset((char *)&sequenceurAddr, 0, sizeof(sequenceurAddr));
    sequenceurAddr.sin_family = AF_INET;
    sequenceurAddr.sin_addr.s_addr = INADDR_ANY;
    sequenceurAddr.sin_port = htons(PORT_SEQUENCEUR);

    // Lier les sockets aux adresses des sous-systèmes
    if (bind(parafoilSocket, (struct sockaddr *)&parafoilAddr, sizeof(parafoilAddr)) < 0) {
        perror("Erreur lors du bind du socket du Parafoil");
        exit(1);
    }

    if (bind(roulisSocket, (struct sockaddr *)&roulisAddr, sizeof(roulisAddr)) < 0) {
        perror("Erreur lors du bind du socket du Roulis");
        exit(1);
    }

    if (bind(sequenceurSocket, (struct sockaddr *)&sequenceurAddr, sizeof(sequenceurAddr)) < 0) {
        perror("Erreur lors du bind du socket du Séquenceur");
        exit(1);
    }

    printf("En attente de données...\n");
    


    while (1) {
        ParafoilData parafoilData;
        RoulisData roulisData;
        SequenceurData sequenceurData;

        memset(&parafoilData, 0, sizeof(parafoilData));
        memset(&roulisData, 0, sizeof(roulisData));
        memset(&sequenceurData, 0, sizeof(sequenceurData));

        // Réception des données du Parafoil
        ssize_t receivedBytes = recvfrom(parafoilSocket, &parafoilData, sizeof(parafoilData), 0, (struct sockaddr *)&parafoilAddr, &addrLen);
        if (receivedBytes < 0) {
            perror("Erreur lors de la réception des données du Parafoil");
            exit(1);
        }

        // Réception des données du Roulis
        receivedBytes = recvfrom(roulisSocket, &roulisData, sizeof(roulisData), 0, (struct sockaddr *)&roulisAddr, &addrLen);
        if (receivedBytes < 0) {
            perror("Erreur lors de la réception des données du Roulis");
            exit(1);
        }

        // Réception des données du Séquenceur
        receivedBytes = recvfrom(sequenceurSocket, &sequenceurData, sizeof(sequenceurData), 0, (struct sockaddr *)&sequenceurAddr, &addrLen);
        if (receivedBytes < 0) {
            perror("Erreur lors de la réception des données du Séquenceur");
            exit(1);
        }

        // Traitement des données reçues
        if (receivedBytes > 0) {
            printf("Données reçues : parafoilValue = %.2f, roulisValue = %.2f, sequenceurValue = %d\n",
                   parafoilData.parafoilValue, roulisData.roulisValue, sequenceurData.sequenceurValue);

            // Utilisation des données reçues
            pthread_mutex_lock(&obcMutex);
            // Traiter les données reçues
            pthread_mutex_unlock(&obcMutex);
        }
    }

    close(parafoilSocket);
    close(roulisSocket);
    close(sequenceurSocket);
    pthread_exit(NULL);
}

int main() {
    pthread_t sendDataThread, receiveDataThread;

    // Création du thread pour envoyer les données aux sous-systèmes
    if (pthread_create(&sendDataThread, NULL, obcSendData, NULL) != 0) {
        perror("Erreur lors de la création du thread pour l'envoi des données");
        exit(1);
    }

    // Création du thread pour recevoir les données des sous-systèmes
    if (pthread_create(&receiveDataThread, NULL, obcReceiveData, NULL) != 0) {
        perror("Erreur lors de la création du thread pour la réception des données");
        exit(1);
    }

    // Initialisation du mutex pour protéger l'accès aux données de l'OBC
    if (pthread_mutex_init(&obcMutex, NULL) != 0) {
        perror("Erreur lors de l'initialisation du mutex");
        exit(1);
    }

    // Attendre la fin des threads
    pthread_join(sendDataThread, NULL);
    pthread_join(receiveDataThread, NULL);

    // Destruction du mutex
    pthread_mutex_destroy(&obcMutex);

    return 0;
}
