#include <unistd.h>     // read
#include <stdio.h>      // printf
#include "../include/aux.h"
#include "../include/my_io.h"
#include <cstring>      // strcmp
#include "../include/aux_2.h"
#include <assert.h>
#include <fcntl.h>      // open


/*
Συνάρτηση που:
1. Διαβάζει από το server.pipe (read_sp) αριθμούς μεγέθους int (πρόκειται για τα pids των διαφορετικών commander)
2. Για κάθε pid που διάβασε ανοίγει το pipe pid_r (pid_w για τον commander)
3. Για κάθε pid αποθηκεύουμε σε ένα struct τις πληροφορίες από το βήμα 1 και 2
struct info_commander {    // Αφορά κάθε pid
    int pid;        // pid εκάστοτε jobCommander
    int fd_r;       // file descriptor για το read άκρο του pipe που δημιούργησε ο jobCommander για επικοινωνία με τον server
    int fd_w;       // file descriptor για το write άκρο του pipe που δημιούργησε ο jobCommander για επικοινωνία με τον server
}
4. Τα structs που θα προκύψουν από τα βήματα 1,2 και 3 τα 
αποθηκεύουμε σε ένα vector active_commanders
*/

// Τσεκάρει αν στο fd υπάρχει κάτι για διάβασμα
// Επιστρέφει -1 αν δεν υπάρχει κάτι για διάβασμα
// Αν υπάρχει κάτι για διάβασμα, επιστρέφει τον αριθμό που διάβασε
// Επιστρέφει 0, αν διάβασε EOF
int my_fdisset_read(int fd, fd_set* rfds) {
    if (FD_ISSET(fd, rfds)) {
        int number;
        ssize_t n = read(fd, &number, sizeof(int));
        if(n == 0) {
            // read returned EOF
            return 0;      // there is nothing more to read in pipe
        } else if(n == -1) {
            perror("read");
            exit(1);
        }
        if(n == sizeof(int)) {
            return number;
        } else {
            exit(1);
        }
    } else {   
        return -1;      // Δεν έχει κάτι για διάβασμα
    }
}

// Τσεκάρει αν στο fd υπάρχει κάτι για διάβασμα
// Επιστρέφει -1 αν δεν υπάρχει κάτι για διάβασμα
// Αν υπάρχει κάτι για διάβασμα, το διαβάζει αποθηκεύοντας το στο buffer
// και επιστρέφει μηδέν
int my_fdisset_read_str(int fd, fd_set* rfds, void* buffer, int size) {
    if (FD_ISSET(fd, rfds)) {
        ssize_t n = read(fd, buffer, size);
        if(n == 0) {
            // read returned EOF
            return -1;      // there is nothing more to read in the pipe
        } else if(n == -1) {
            perror("read");
            exit(1);
        }
        if(n != size) {
            perror("fd_isset said i have something to read, but there is no str to read");
            exit(1);
        }
        return 0;
    } else {        
        return -1;      // Δεν έχει κάτι για διάβασμα
    }
}


// Συνάρτηση που λαμβάνει read end του server_pipe και διαβάζει ακεραίους (pid commanders)
// Για κάθε pid commander ανοίγει το read άκρο του pipe που δημιούργησε ο commander
// Οι πληροφορίες των commanders αποθηκεύονται στο vector active_commanders
void read_from_server_pipe(vector<struct info_commander>& active_commanders, fd_set* rfds) {
    int pid;
    ssize_t n;
    // while((n = read(read_sp, &pid, sizeof(int))) == sizeof(int)) {     // Όσο υπάρχει κάτι στο pipe για διάβασμα
    while((pid = my_fdisset_read(read_sp, rfds)) > 0) {     // Όσο υπάρχει κάτι στο pipe για διάβασμα
        // pid

       // pid_r
            int pidreadfd = open_read_end_commander(pid);   // file descriptor read end του pid_r

        // pid_w
            // int pidwritefd = open_write_end_commander(pid); // file descriptor write end του pid_w

        // struct
            struct info_commander info;            // Αποθηκεύουμε τις πληροφορίες του εκάστοτε pid στο vector
            
            info.pid = pid;
            info.fd_r = pidreadfd;
            // info.fd_w = pidwritefd;
        // vector.push_back
            active_commanders.push_back(info);

        // close(fd)
    }

    // If you want select to remain blocked even when an EOF is detected on the file descriptor
    // you need to reopen the pipe when EOF is detected.
    if(pid == 0) {      // read EOF
        close(read_sp);
        if ( (read_sp = open("server.pipe", O_RDONLY | O_NONBLOCK))  < 0)  {      
            perror("server: can't open read server.pipe");
            exit(1);
        }
    }
}

// Δημιουργεί μία νέα εργασία που θα περιέχει την εντολή <whole_command>
// Εισάγει την εργασία στην ουρά
// Επιστρέφουμε στον jobCommander την τριπλέτα που αφορά τη νέα εργασία
void handle_issueJob(int pid, vector<string>& whole_command, my_queue& queue) {
// [1] Εισάγουμε την εργασία στην ουρά

    // whole_command[0] == "issueJob"
    // Νέα εργασία
    struct Job job_ins = new_job(whole_command);     

    // Εισάγουμε την εργασία στην ουρά
    queue.push(job_ins);

    // Δημιουργία τριπλέτας <jobID,job,queuePosition>  (**): Πρώτα δημιουργούμε την τριπλέτα, και μετά αφαιρούμε την εργασία από την ουρά. Διαφορετικά, το queuePosition θα επιστρέψει "Job not found".
    string triple = get_triple(queue, job_ins);

// [2] Επιστροφή τριπλέτας στον jobCommander
    // open pid_w, write at pid_w, close pid_w, return

    // open pid_r του commander (write για εμάς)
    int pidwritefd = open_write_end_commander(pid);

    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    int repeat = 1;
    write_number_to_fd(pidwritefd, repeat);

    // Για κάθε πρόταση:
    for(int i = 0; i < repeat; i++) {
        // Γράφει στο fd το μέγεθος του string, και στη συνέχεια το ίδιο το string 
        write_string_to_fd(pidwritefd, triple);
    }


    // close pid_r
    close(pidwritefd);

    // return
    return;
}

// handle cmd, open pid_w, write at pid_w, close pid_w, return
void handle_command(int pid, vector<string>& whole_command, my_queue& queue, vector<struct info_commander>& active_commanders) {

    // We decode τι τύπου εντολή είναι: -- issuejob, setconcurrency ...?
    if(whole_command.size() == 0) {
        // Δεν δόθηκε τύπος εντολής
        exit(1);
    }

    if(whole_command[0].compare("issueJob") == 0) {

        if(whole_command.size() == 1) {
            exit(1);      
        }
        
        handle_issueJob(pid, whole_command, queue);

    } else if(whole_command[0].compare("setConcurrency") == 0) {

        if(whole_command.size() != 2) {
            exit(1);
        }

        int new_concurrency = stoi(whole_command[1]);
        if(new_concurrency < 1) {
            exit(1);        
        }


        handle_setConcurrency(pid, new_concurrency);    

    } else if(whole_command[0].compare("stop") == 0) {
    


        if(whole_command.size() != 2) {
            exit(1);
        }

        handle_stop(pid, queue, whole_command[1]);


    } else if(whole_command[0].compare("poll") == 0) {

        if(whole_command.size() != 2) {
            exit(1);
        }

        if(whole_command[1].compare("running") == 0) {


            handle_poll_running(pid);


        } else if(whole_command[1].compare("queued") == 0) {



            handle_poll_queued(pid, queue);


        } else {
            exit(1);
        }


    } else if(whole_command[0].compare("exit") == 0) {
    

        if(whole_command.size() != 1) {
            exit(1);
        }

        handle_exit(pid, active_commanders);

    } else {
        exit(1);
    }


}


// Για κάθε commander στο vector active_commanders, διαβάζει από το read άκρο του pipe που δημιούργησε ο 
// o commander
// Ανάλογα με την εντολή που διαβάσαμε, ο server κάνει handle τη συγκεκριμένη εντολή
// Ανοίγει το write άκρο του pipe που δημιούργησε ο commander, και γράφει στο pipe κάποιο μήνυμα για τον commander
// Αφαιρεί από το vector active_commanders τον τρέχοντα commander
void read_from_commanders(vector<struct info_commander>& active_commanders, my_queue& queue, fd_set* rfds) {
    int size = active_commanders.size();

    // Για κάθε commander που βρίσκεται μέσα στο active_commanders
    for(int i = 0; i < size; i++) {
        if(exit_server == 1) {
            // Η τελευταία εντολή που εκτελέσαμε μέσα σε αυτό το loop είναι η exit
            // Έχουν κλείσει όλοι οι file descriptors του vector active_commanders
            break;
        }

        struct info_commander info = active_commanders[i];      // ΤΟΠΙΚΗ ΜΕΤΑΒΛΗΤΗ

        int reps;       // Από πόσες λέξεις αποτελείται η εντολή που θέλει να μάς στείλει ο commander
        ssize_t n;
        // if((n = read(info.fd_r, &reps, sizeof(int))) == sizeof(int)) {     // Υπάρχει κάτι για διάβασμα στο pipe?
        if((reps = my_fdisset_read(info.fd_r, rfds)) != -1) {     // Υπάρχει κάτι για διάβασμα στο pipe?
        
        // Ναι, υπάρχει.
        // reps

            // loop μέχρι να διαβάσουμε και την εντολή
            vector<string> whole_command;

            // Για κάθε όρισμα + εντολή + τύπου εντολής:
            for(int i = 0; i < reps; i++) {
                // Στην τρέχουσα επανάληψη, θα χρειαστεί να διαβάσουμε size bytes

                // we spin around read, till there is something in the pipe
                int size;
                // while((n = read(info.fd_r, &size, sizeof(int))) != sizeof(int)) {
                while((size = my_fdisset_read(info.fd_r, rfds)) != -1) {
                    break;          // Διαβάσαμε τον ακέραιο
                    // continue;
                }

                string str(size, '\0');     // Αρχικοποιούμε ένα string μεγέθους <size> με τον χαρακτήρα '\0' (call of string constructor) 

                // Διαβάζουμε αυτά τα size bytes   

                // we spin around read, till there is something in the pipe
                // while((n = read(info.fd_r, (void*)str.data(), size)) != size) {        // Μπορεί να μην τα διαβάζει σε 1 read?
                while(my_fdisset_read_str(info.fd_r, rfds, (void*)str.data(), size) == -1) {        // Μπορεί να μην τα διαβάζει σε 1 read?
                    continue;
                }

                whole_command.push_back(str);

            }
            close(info.fd_r);       // Διαβάσαμε όλη την εντολή από το pipe (κλείνουμε τα άκρα που δεν χρησιμοποιούμε)
            active_commanders[i].fd_r = -1;   // remove current commander from vector

            handle_command(info.pid, whole_command, queue, active_commanders);
                // ^ handle cmd, open pid_w, write at pid_w, close pid_w, return



        } else {        // Ο commander δεν έχει γράψει ακόμα στο pipe
            continue;   // Πάμε στον επόμενο commander του active_commanders
        }

    }


    // Αφού διασχίσαμε όλους τους active commanders, αφαιρούμε από το vector με όσους τελειώσαμε
    vector<struct info_commander>::iterator iter;     //decleration of vector iterator
    
    for(iter = active_commanders.begin(); iter < active_commanders.end(); iter++) {

        if((*iter).fd_r == -1) {
            // Έχουμε διαβάσει από το pipe, έχουμε κάνει handle το command, και έχουμε
            // επιστρέψει αντίστοιχο μήνυμα στον commander

            // erase
            active_commanders.erase(iter);
        }
    }


}


void exec_jobs(my_queue& queue) {
    // Η τελευταία εντολή που διαβάσαμε στην last call of read_from_commanders ήταν η exit
    // Μην κάνεις exec κάποια νέα εντολή
    if(exit_server == 1) {      
        return;
    }

// check concurrency & do execs

    // Καθώς τρέχουμε τον συγκεκριμένο κώδικα η τιμή του global_Running
    // ενδέχεται να μεταβληθεί.
    int temp_running = global_Running;      
    int queue_size = queue.size();

    assert(temp_running >=0);

    if(temp_running == global_Concurrency) {      // Δεν υπάρχει χώρος για άλλη διεργασία προς εκτέλεση
        return;                                   // max value of global_Running reached (considering concurrency)
    }

    if(queue_size == 0) {       // Δεν υπάρχει κάποια εργασία στην ουρά προς εκτέλεση
        return;        
    }

    int can_run = global_Concurrency - temp_running;      // can_run == 0, εάν temp_running == temp_concurrency
    if(can_run < 0) {       // Το concurrency μειώθηκε
        return;             // Τρέχουν παραπάνω διεργασίες, από το concurrency. Οπότε, return // (**)
    }
    
    if(queue_size < can_run) {      // Οι διαθέσιμες εργασίες είναι λιγότερες από τον available χώρο
        can_run = queue_size;
    }                               // Διαφορετικά: can_run = global_Concurrency - temp_running;


    // Θα κάνουμε <can_run> σε πλήθος fork(s)
    for(int i = 0; i < can_run; i++) {
        // Εργασία προς εκτέλεση
        struct Job job = queue.top();       // Το κάνουμε εδώ, γιατί μέσα στα forks, μπορεί να μπλεχτούν τα pop
        queue.pop();
        global_Running++;                   // Θα εκτελέσουμε μια νέα διεργασία     [όχι! εντός κώδικα child]
        

        pid_t pid = fork();
        if (pid == -1){
            perror("Failed to fork");
            exit(1);
        }
        if (pid == 0) {        // child
            my_execvp(job.cmd_prms);
            perror("my_execvp");       // If execvp returns, it means an error occurred
            exit(1);        // Αν κάνει fail η execvp, δεν θα κάνει exit ο server
        }       
        else {      // parent

            // Προσθήκη εργασίας στο vector με τις διεργασίες που τρέχουν αυτή τη στιγμή
            struct info_running info;           
            info.job_XX = job.jobID; 
            info.pid = pid;
            info.id = job.id;
            info.cmd_prms = job.cmd_prms;

            // Προσθέτουμε το info στην αρχή του vector, σαν προσομοίωση ουράς (needed in poll running)
            global_Running_Jobs.insert(global_Running_Jobs.begin(), info);      

            // global_Running_Jobs.push_back(info);

            continue;
        }

    }       // end for

    // CHECK pro-1/signal-handler/main.cpp

    return;
}


/*
(*):
if(n == -1){
    // perror("server: data read error! \n");
    // ΟΧΙ return 1, γιατί τότε θα κλείσει το pipe
}

(**): Από piazza:
Απο την εκφωνηση:

Αν ο βαθμός παραλληλίας αλλάξει, ο jobExecutor θα
προσαρμοστεί αντίστοιχα, χωρίς να σταματήσει εργασίες που ήδη τρέχουν, σε περίπτωση
μείωσης του βαθμού παραλληλίας.
*/