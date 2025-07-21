#pragma once // #include το πολύ μία φορά

#include <string>
using namespace std;

// Συνάρτηση που λαμβάνει έναν file descriptor και έναν αριθμό.
// Στέλνει στον file descriptor τον αριθμό. 
void write_number_to_fd(int fd, int number);

// Συνάρτηση που λαμβάνει έναν file descriptor και ένα string.
// Αρχικά στέλνει στον file descriptor το μέγεθος του string,
// ενώ στη συνέχεια στέλνει στον file descriptor το ίδιο το string.
void write_string_to_fd(int fd, string& str);

// Συνάρτηση που διαβάσει από τον file descriptor έναν αριθμό.
// Επιστρέφει τον αριθμό που διάβασε.
int read_int_from_fd(int fd);

// Η συνάρτηση διαβάζει από το fd <size> χαρακτήρες και τους αποθηκεύει
// στον buffer ο οποίος έχει μέγεθος <size>
// Πριν την κλήση της συνάρτησης θα πρέπει να έχει δεσμευτεί κατάλληλος 
// χώρος για τον buffer.
void read_chars_from_fd(int fd, char* buffer, int size);


// Συνάρτηση που ανοίγει το read end του pipe με όνομα pid_w
// Επιστρέφει το file descriptor που επέστρεψε η open
int open_read_end_commander(int pid);


// Συνάρτηση που ανοίγει το write end του pipe με όνομα pid_r
// Επιστρέφει το file descriptor που επέστρεψε η open
int open_write_end_commander(int pid);