#include "server.h"


/* ************************************************************************* */
/* MAIN                                                                      */
/* ************************************************************************* */




int main(int argc, char** argv)
{
    int server_socket;                 // descriptor of server socket
    struct sockaddr_in server_address; // for naming the server's listening socket
    int yes = 1;
    


    // ----------------------------------------------------------
    // ignore SIGPIPE, sent when client disconnected
    // ----------------------------------------------------------
    signal(SIGPIPE, SIG_IGN);
    
    // ----------------------------------------------------------
    // create unnamed network socket for server to listen on
    // ----------------------------------------------------------
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }
    
    // lose the pesky "Address already in use" error message
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // ----------------------------------------------------------
    // bind the socket
    // ----------------------------------------------------------
    server_address.sin_family      = AF_INET;           // accept IP addresses
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // accept clients on any interface
    server_address.sin_port        = htons(PORT);       // port to listen on
    
    // binding unnamed socket to a particular port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) != 0) 
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }
    
    // ----------------------------------------------------------
    // listen on the socket
    // ----------------------------------------------------------
    if (listen(server_socket, NUM_CONNECTIONS) != 0)
    {
        perror("Error listening on socket");
        exit(EXIT_FAILURE);
    }
    sem_t mutex;
    sem_t mutex2;
    //Using a binary semaphore to stop race conditions
    sem_init(&mutex, 0, 1);
    sem_init(&mutex2, 0, 1);
    // ----------------------------------------------------------
    // server loop
    // ----------------------------------------------------------
    
    while (TRUE)
    {
        //sem wait so we don't overwrite the client socket before the thread is finished.
        sem_wait(&mutex);
        
        // accept connection to client
        int client_socket = accept(server_socket, NULL, NULL);
        printf("\nServer with PID %d: accepted client\n", getpid());
 	         
        //initializing the arguments for the thread in a struct
	struct arguments* argument = (struct arguments*)malloc(sizeof(struct arguments));
	argument->semaphore = &mutex2;
        argument->client_socket = client_socket;
	
        //crtical section over.
        sem_post(&mutex);
        pthread_t thread;
        if (pthread_create(&thread, NULL, handle_client, (void*)argument) != 0)
        {
            perror("Error creating thread");
            exit(EXIT_FAILURE);
        }
        
        
        // detach the thread so that we don't have to wait (join) with it to reclaim memory.
        // memory will be reclaimed when the thread finishes.
        if (pthread_detach(thread) != 0)
        {
            perror("Error detaching thread");
            exit(EXIT_FAILURE);
        }
	
    }
    
    sem_destroy(&mutex);
    sem_destroy(&mutex2);
}


/* ************************************************************************* */
/* handle client                                                             */
/* ************************************************************************* */

void* handle_client(void* arg) 
{
    //unpacking arguments from the struct
    struct arguments* argument = (struct arguments*)arg;       
    sem_t* mutex2 = argument->semaphore; 
 
    //wait while we handle the thread.
    sem_wait(mutex2);
    
    //unpack client socket
    int client_socket = argument->client_socket;
    
    //creates a buffer to store the time info with a max size of 80 bytes
    char buffer[MAX_SIZE];

    //stores the number of seconds since the unix epoch
    time_t *seconds;
    
    //creates a pointer to a time structure 
    struct tm* UTC_time;

    //gets the number of seconds since the unix epoch
    *seconds = time(NULL);

    //converts that number of seconds from the local time zone to UTC
    UTC_time = gmtime(seconds);

    //Converts the number of seconds into a custom time format, and stores that string in the buffer
    strftime(buffer, sizeof(buffer), "\n%y-%m-%d %H:%M:%S UTC \n", UTC_time);

     
    
    //writes the time string to the client socket.
    write(client_socket, &buffer, strlen(buffer));
    

    if (close(client_socket) == -1) 
    {
        perror("Error closing socket");
        exit(EXIT_FAILURE);
    } 
    else
    {
        printf("Closed socket to client, exit");
    }
        
    //frees the arguments
    free(argument);
    //unlocks the mutex
    sem_post(mutex2);
    pthread_exit(NULL);
}

