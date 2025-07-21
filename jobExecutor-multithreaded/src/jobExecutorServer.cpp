#include <stdlib.h>         // atoi
#include <stdio.h>          // printf
#include <assert.h>
#include <sys/types.h>      // socket
#include <sys/socket.h>     // socket
#include <netinet/in.h>      /* internet sockets */
#include <unistd.h>         // read
#include <pthread.h>        /* For threads  */
#include <string.h>         /* For strerror */
#include "../include/aux_s.h"
#include "../include/my_io.h"
#include "../include/my_queue.h"

int global_Concurrency = 1;     // Προκαθορισμένη τιμή
pthread_mutex_t  global_Mtx_Concurrency = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  global_Mtx_JobID = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  global_Mtx_Queue = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  global_Mtx_Running = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t  global_Mtx_Controller_Count = PTHREAD_MUTEX_INITIALIZER;
my_queue global_Queue;         // Δημιουργία ουράς
int global_JobID;          // Υποθέτουμε ότι τα job ids ξεκινούν από το 1
int global_BufferSize;
int global_Running;         // Εργασίες που τρέχουν αυτή τη στιγμή (initialized by defualt to zero)
int exit_server;            // by default is set to zero
int global_ControllerCount; // by default is set to zero

pthread_cond_t global_Cond_nonempty;
pthread_cond_t global_Cond_nonfull;
pthread_cond_t global_Cond_nonfull_Running;

void signal_handler(int sig) {
}

#include <signal.h>     // sigaction

int main(int argc, char *argv[]) {
    static struct sigaction act;
    // set up the signal handler
    act.sa_handler = signal_handler;
    sigfillset(&(act.sa_mask));
    sigaction(SIGUSR1, &act, NULL);

// Program runs as: ./bin/jobExecutorServer [portnum] [bufferSize] [threadPoolSize]
    assert(argc == 4);

    int portnum, bufferSize, threadPoolSize;
    my_atoi(argv, &portnum, &bufferSize, &threadPoolSize);
    global_BufferSize = bufferSize;

// Init conditional variables:
	pthread_cond_init(&global_Cond_nonempty, 0);
	pthread_cond_init(&global_Cond_nonfull, 0);
	pthread_cond_init(&global_Cond_nonfull_Running, 0);

// Δημιουργία worker threads
    vector<pthread_t> threads(threadPoolSize);

    for(int i = 0; i < threadPoolSize; i++) {
        pthread_t thr;
        int err = pthread_create(&threads[i], NULL, worker_f, NULL);
        if (err != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            exit(EXIT_FAILURE);  
        }
    }

/* Create socket */
    int sock = socket(AF_INET, SOCK_STREAM, 0);                 // returns a file descriptor or -1 on error
    if (sock == -1) {
        /* int socket ( int domain , int type , int protocol ) ;
        domain = internet (AD_INET) || Unix
        type = TCP || UDP (SOCK_STREAM || SOCK_DGRAM, αντίστοιχα)
        protocol = 0 (όταν έχουμε μόνο ένα πρωτόκολλο)
        */
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // A socket address is represented by a sockaddr_in struct
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *)&server;

    server.sin_family = AF_INET;                    /* Internet domain */
    server.sin_port = htons(portnum);               // converts the unsigned short integer hostshort from host byte order to network byte order.
    server.sin_addr.s_addr = htonl(INADDR_ANY);     // INADDR_ANY = 0.0.0.0 converts the unsigned integer hostlong from host byte order to network byte order.


// Setting socket options
    const int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt(SO_REUSEADDR) failed");
        exit(1);
    }

/* Bind socket to address */
    if (bind(sock, serverptr, sizeof(server)) == -1) {        // bind requests for an address to be assigned to a socket
        perror("bind");
        exit(EXIT_FAILURE);        
    }

/* Listen for connections */
    if (listen(sock, 128) == -1) {      // 128 = max connections in linux kernel
        perror("listen");
        exit(EXIT_FAILURE);  
    }
    

    while (1) {
        /* accept connection */
        struct sockaddr_in client;
        struct sockaddr *clientptr = (struct sockaddr *)&client;
        socklen_t clientlen = sizeof(client);

        int newsock = accept(sock, clientptr, &clientlen);
        if (newsock == -1) {          // blocks here, μέχρι κάποιος να συνδεθεί
            if (errno == EINTR) {
                break;  // Exit the loop if accept is interrupted
            } else {
                perror("accept");
                exit(EXIT_FAILURE);
            }
        }

// Create controller thread
        pthread_t thr;
        int* socketNum = (int*) malloc(sizeof(int));
        *socketNum = newsock;
        // Φροντίζουμε ο pointer να γίνεται free'd στο τέλος του κώδικα του controller_f
        int err = pthread_create(&thr, NULL, controller_f, socketNum);
        if (err != 0) {
            fprintf(stderr, "pthread_create: %s\n", strerror(err));
            exit(EXIT_FAILURE);  
        }

    }


    // Περιμένουμε κάθε worker thread να τελειώσει
    // Join all threads
    for (int i = 0; i < threadPoolSize; i++) {
        int status;
        int err = pthread_join(threads[i], (void**) &status);
        if(err != 0) {
            perror("thread_join");
            exit(1);
        }
    }


    while(global_ControllerCount != 0);
    // Τα controller threads once "broadcasted" that server will exit,
    // τελειώνουν ακαριαία. Γι' αυτό και βάζουμε while, αντί π.χ. κάποιο condition variable


    int size = global_Queue.size();
    for(int i = 0; i < size; i++) {
        struct Job job = global_Queue.top();
        global_Queue.pop();

        // "Αποτέλεσμα" εργασίας
        string to_return = "SERVER TERMINATED BEFORE EXECUTION";
        write_string_to_socket(job.clientSocket, to_return);
        close(job.clientSocket);
    }

    close(sock);

    /* Destroy mutex and condition variable */
    int err = pthread_mutex_destroy(&global_Mtx_Concurrency);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_destroy global_Mtx_Concurrency: %s\n", strerror(err));
        exit(EXIT_FAILURE);  
    }

    err = pthread_mutex_destroy(&global_Mtx_JobID);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_destroy global_Mtx_JobID: %s\n", strerror(err));
        exit(EXIT_FAILURE);  
    }

    err = pthread_mutex_destroy(&global_Mtx_Queue);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_destroy global_Mtx_Queue: %s\n", strerror(err));
        exit(EXIT_FAILURE);  
    }

    err = pthread_mutex_destroy(&global_Mtx_Running);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_destroy global_Mtx_Running: %s\n", strerror(err));
        exit(EXIT_FAILURE);  
    }

    err = pthread_mutex_destroy(&global_Mtx_Controller_Count);
    if (err != 0) {
        fprintf(stderr, "pthread_mutex_destroy global_Mtx_Controller_Count: %s\n", strerror(err));
        exit(EXIT_FAILURE);  
    }

    err = pthread_cond_destroy(&global_Cond_nonempty);
    if (err != 0) {
        fprintf(stderr, "pthread_cond_destroy global_Cond_nonempty: %s\n", strerror(err));
        exit(EXIT_FAILURE);  
    }

    err = pthread_cond_destroy(&global_Cond_nonfull);
    if (err != 0) {
        fprintf(stderr, "pthread_cond_destroy global_Cond_nonfull: %s\n", strerror(err));
        exit(EXIT_FAILURE);  
    }

    err = pthread_cond_destroy(&global_Cond_nonfull_Running);
    if (err != 0) {
        fprintf(stderr, "pthread_cond_destroy global_Cond_nonfull_Running: %s\n", strerror(err));
        exit(EXIT_FAILURE);  
    }

    return 0;
}
