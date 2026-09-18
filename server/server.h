#include <semaphore.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <time.h>
#include <string.h>
#include <signal.h>
/* Function prototypes */
void* handle_client(void* arg);

/* Preprocessor directives */
#define SERVER_ADDR "172.233.157.24" // loopback ip address
#define PORT 23757              // port the server will listen on
#define MAX_SIZE 80
#define FALSE 0
#define TRUE !FALSE

#define NUM_CONNECTIONS 5       // number of pending connections in the connection queue

struct arguments{
    sem_t* semaphore;
    int client_socket;
};
