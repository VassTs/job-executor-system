#include <stdio.h>      // perror
#include <unistd.h>     // write
#include <stdlib.h>     // exit
#include "../include/my_io.h"
#include <fcntl.h>      // open

// Συνάρτηση που λαμβάνει έναν file descriptor και έναν αριθμό.
// Στέλνει στον file descriptor τον αριθμό. 
void write_number_to_fd(int fd, int number) {
    ssize_t n = write(fd, &number, sizeof(int));
    if(n != sizeof(int)) {
        perror("data write error");
        exit(1);
    }
    return;
}

// Συνάρτηση που λαμβάνει έναν file descriptor και ένα string.
// Αρχικά στέλνει στον file descriptor το μέγεθος του string,
// ενώ στη συνέχεια στέλνει στον file descriptor το ίδιο το string.
void write_string_to_fd(int fd, string& str) {
    int size = str.size();

    // Στέλνουμε το μέγεθος του string
    write_number_to_fd(fd, size);

    // Στέλνουμε το ίδιο το string
    if(size = write(fd, str.c_str(), size) != size) {
        perror("data write error");
        exit(1);
    }
    return;
}

// Συνάρτηση που διαβάζει από τον file descriptor έναν αριθμό.
// Επιστρέφει τον αριθμό που διάβασε.
int read_int_from_fd(int fd) {
    int number;
    if(read(fd, &number, sizeof(int)) != sizeof(int)) {
        perror("server: data read error \n");
        exit(1);
    }
    return number;
}

// Η συνάρτηση διαβάζει από το fd <size> χαρακτήρες και τους αποθηκεύει
// στον buffer ο οποίος έχει μέγεθος <size>
// Πριν την κλήση της συνάρτησης θα πρέπει να έχει δεσμευτεί κατάλληλος 
// χώρος για τον buffer.
void read_chars_from_fd(int fd, char* buffer, int size) {
    if(read(fd, buffer, size) != size) {        // Μπορεί να μην τα διαβάζει σε 1 read?
        perror("data read error \n"); 
        exit(1);
    }
    return;
}


// Συνάρτηση που ανοίγει το read end του pipe με όνομα pid_w
// Επιστρέφει το file descriptor που επέστρεψε η open
int open_read_end_commander(int pid) {
    // Όνομα του pipe
    // Για τον server είναι read, αλλά για τον commander είναι write.
    // Ο commander το δημιούργησε ως: pid_w
    string number = to_string(pid);
    string w = "_w";
    string pid_w = number + w;      

    // Ο commander σίγουρα έχει δημιουργήσει το pipe, καθώς η δημιουργία
    // του pipe είναι η 1η γραμμή του commander (και πρώτα τρέχει ο server)
    int pidreadfd;
    if ( (pidreadfd = open(pid_w.c_str(), O_RDONLY))  < 0)  {             
        perror("server: can't open read pid_w");
        exit(1);
    }

    return pidreadfd;
}


// Συνάρτηση που ανοίγει το write end του pipe με όνομα pid_r
// Επιστρέφει το file descriptor που επέστρεψε η open
int open_write_end_commander(int pid) {
    // Όνομα του pipe
    // Για τον server είναι write, αλλά για τον commander είναι read.
    // Ο commander το δημιούργησε ως: pid_r
    string number = to_string(pid);
    string r = "_r";
    string pid_r = number + r;      


    int pidwritefd;
    // We spin on the open call until it succeeds.
    // Ο λόγος που κάνει fail είναι επειδή ο commander δεν έχει δημιουργήσει ακόμη το pipe

    // Ο commander σίγουρα έχει δημιουργήσει το pipe, καθώς η δημιουργία
    // του pipe είναι η 1η γραμμή του commander (και πρώτα τρέχει ο server)
    if ( (pidwritefd = open(pid_r.c_str(), O_WRONLY))  < 0)  {             
        perror("server: can't open write pid_r");
        exit(1);
    }

    return pidwritefd;
}