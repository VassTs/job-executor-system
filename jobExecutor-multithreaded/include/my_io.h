#pragma once // #include το πολύ μία φορά

#include <string>
using namespace std;

// Συνάρτηση που λαμβάνει ένα socket (aka, file descriptor) και έναν αριθμό.
// Γράφει στο socket τον αριθμό. 
void write_number_to_socket(int sock, int number);

// Συνάρτηση που λαμβάνει ένα socket (aka, file descriptor) και ένα string.
// Αρχικά γράφει στο socket το μέγεθος του string,
// ενώ στη συνέχεια γράφει στο socket το ίδιο το string.
void write_string_to_socket(int sock, string& str);

// Συνάρτηση που διαβάζει από ένα socket (aka, έναν file descriptor) έναν αριθμό.
// Επιστρέφει τον αριθμό που διάβασε.
int read_int_from_socket(int sock);

// Συνάρτηση που διαβάζει από το socket ένα string μεγέθους n
// αποθηκεύοντας το στο str
void read_string_from_socket(int sock, string& str, int n);
