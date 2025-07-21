#include <unistd.h>     // write
#include <stdio.h>      // perror
#include <stdlib.h>     // exit
#include "../include/my_io.h"


// Συνάρτηση που λαμβάνει ένα socket (aka, file descriptor) και έναν αριθμό.
// Γράφει στο socket τον αριθμό. 
void write_number_to_socket(int sock, int number) {
    ssize_t n = write(sock, &number, sizeof(int));
    if(n != sizeof(int)) {
        perror("data write error");
        exit(EXIT_FAILURE);
    }
    return;
}

// Συνάρτηση που λαμβάνει ένα socket (aka, file descriptor) και ένα string.
// Αρχικά γράφει στο socket το μέγεθος του string,
// ενώ στη συνέχεια γράφει στο socket το ίδιο το string.
void write_string_to_socket(int sock, string& str) {
    int size = str.size();

    // Στέλνουμε το μέγεθος του string
    write_number_to_socket(sock, size);

    // Στέλνουμε το ίδιο το string
    if(size = write(sock, str.c_str(), size) != size) {
        perror("data write error");
        exit(EXIT_FAILURE);
    }
    return;
}

// Συνάρτηση που διαβάζει από ένα socket (aka, έναν file descriptor) έναν αριθμό.
// Επιστρέφει τον αριθμό που διάβασε.
int read_int_from_socket(int sock) {
    int number;
    if(read(sock, &number, sizeof(int)) != sizeof(int)) {
        perror("data read error \n");
        exit(EXIT_FAILURE);
    }
    return number;
}

// Συνάρτηση που διαβάζει από το socket ένα string μεγέθους n
// αποθηκεύοντας το στο str
void read_string_from_socket(int sock, string& str, int n) {
    // Διαβάζουμε αυτά τα n bytes   
    if(read(sock, (void*)str.data(), n) != n) {
        perror("data read error \n"); 
        exit(1);
    }    
    return;
}
