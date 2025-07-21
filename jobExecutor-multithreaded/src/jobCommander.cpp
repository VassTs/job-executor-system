#include <stdlib.h>         // atoi
#include <netdb.h>	        // gethostbyaddr
#include <stdio.h>          // printf
#include <string.h>         // memcpy
#include <unistd.h>         // close
#include "../include/my_io.h"
#include <string>
using namespace std;

int main(int argc, char *argv[]) {
// Program runs as: ./bin/jobCommmander [serverName] [portNum] [jobCommanderInputCommand]

/* Find server address */
    struct hostent *rem = gethostbyname(argv[1]);
    if (rem == NULL) {	
        herror("gethostbyname"); 
        exit(1);
    }

/* Convert port number to integer */
    int portNum = atoi(argv[2]);            

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

    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr*)&server;
    server.sin_family = AF_INET;                                /* Internet domain */
    memcpy(&server.sin_addr, rem->h_addr, rem->h_length);
    server.sin_port = htons(portNum);                           // converts the unsigned short integer hostshort from host byte order to network byte order.

/* Initiate connection */
    if (connect(sock, serverptr, sizeof(server)) == -1) {       // A connection is attempted to a listening socket on the server in address
        perror("connect");
        exit(EXIT_FAILURE);
    }

/* Write to socket */
    // Λέμε στον server πόσα <ορίσματα + εντολή + τύπος εντολής> διαβάσματα θα χρειαστεί να κάνει:
    int reps = argc - 3;
    write_number_to_socket(sock, reps);

    // Για κάθε όρισμα + εντολή + τύπο εντολής
    for(int i = 3; i < argc; i++) {
        string str = argv[i]; 
        write_string_to_socket(sock, str);
    }


/* Read from socket */
   // Πόσες προτάσεις θα διαβάσουμε από τον server
   int sentences = read_int_from_socket(sock);

   // Για κάθε πρόταση:
   for(int i = 0; i < sentences; i++) {
      // Πόσα bytes είναι η τρέχουσα πρόταση
      int n = read_int_from_socket(sock);

      // Πάμε να διαβάσουμε την actual πρόταση
      string str(n, '\0');     // Αρχικοποιούμε ένα string μεγέθους <n> με τον χαρακτήρα '\0' (call of string constructor) 

      read_string_from_socket(sock, str, n);

      // Εκτυπώνουμε την actual πρόταση στην οθόνη:
      printf("%s\n", str.c_str());

    }

    if(strcmp(argv[3], "issueJob") == 0) {
        // Πόσους χαρακτήρες θα διαβάσουμε από το socket
        int characters = read_int_from_socket(sock);

        int loops = characters / 200;
        int remaining = characters % 200;

        for(int i = 0; i < loops; i++) {
            // Διαβάζουμε 200 χαρακτήρες από το socket
            string str(200, '\0');     // Αρχικοποιούμε ένα string μεγέθους 200 με τον χαρακτήρα '\0' (call of string constructor) 
            read_string_from_socket(sock, str, 200);
            
            // Εκτυπώνουμε τους χαρακτήρες στην οθόνη:
            printf("%s", str.c_str());
        }

        if(remaining != 0) {
            // Διαβάζουμε remaining χαρακτήρες από το socket
            string str(remaining, '\0');     // Αρχικοποιούμε ένα string μεγέθους remaining με τον χαρακτήρα '\0' (call of string constructor) 
            read_string_from_socket(sock, str, remaining);
                
            // Εκτυπώνουμε τους χαρακτήρες στην οθόνη:
            printf("%s", str.c_str());
        }
    }

    close(sock);
    return 0;
}
