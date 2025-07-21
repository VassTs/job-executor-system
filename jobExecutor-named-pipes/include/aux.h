#pragma once

#include "../include/my_queue.h"
#include <vector>
using namespace std;

extern int global_Running;
extern int global_Concurrency;
extern vector<struct info_running> global_Running_Jobs; 
extern int exit_server;
extern int read_sp;

struct info_commander {    // Αφορά κάθε pid
    int pid;        // pid εκάστοτε jobCommander
    int fd_r;       // file descriptor για το read άκρο του pipe που δημιούργησε ο jobCommander για επικοινωνία με τον server
    int fd_w;       // file descriptor για το write άκρο του pipe που δημιούργησε ο jobCommander για επικοινωνία με τον server
};

struct info_running {
    string job_XX;      // Το job_XX,
    int pid;            // όταν έγινε exec, είχε pid = <pid>
    int id;
    vector<string> cmd_prms;   // <εντολή> + <παράμετρος_1> + ... + <παράμετρος_n>
};

// Συνάρτηση που λαμβάνει read end του server_pipe και διαβάζει ακεραίους (pid commanders)
// Για κάθε pid commander ανοίγει το read άκρο του pipe που δημιούργησε ο commander
// Οι πληροφορίες των commanders αποθηκεύονται στο vector active_commanders
void read_from_server_pipe(vector<struct info_commander>& active_commanders, fd_set* rfds);

// Για κάθε commander στο vector active_commanders, διαβάζει από το read άκρο του pipe που δημιούργησε ο 
// o commander
// Ανάλογα με την εντολή που διαβάσαμε, ο server κάνει handle τη συγκεκριμένη εντολή
// Ανοίγει το write άκρο του pipe που δημιούργησε ο commander, και γράφει στο pipe κάποιο μήνυμα για τον commander
// Αφαιρεί από το vector active_commanders τον τρέχοντα commander
void read_from_commanders(vector<struct info_commander>& active_commanders, my_queue& queue, fd_set* rfds);

// add description of func
void exec_jobs(my_queue& queue);


// Τσεκάρει αν στο fd υπάρχει κάτι για διάβασμα
// Επιστρέφει -1 αν δεν υπάρχει κάτι για διάβασμα
// Αν υπάρχει κάτι για διάβασμα, επιστρέφει τον αριθμό που διάβασε
int my_fdisset_read(int fd, fd_set* rfds);


// Τσεκάρει αν στο fd υπάρχει κάτι για διάβασμα
// Επιστρέφει -1 αν δεν υπάρχει κάτι για διάβασμα
// Αν υπάρχει κάτι για διάβασμα, το διαβάζει αποθηκεύοντας το στο buffer
// και επιστρέφει μηδέν
int my_fdisset_read_str(int fd, fd_set* rfds, void* buffer, int size);