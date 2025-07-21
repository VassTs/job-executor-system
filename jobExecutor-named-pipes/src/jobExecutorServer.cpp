#include <sys/types.h>  // mkfifo
#include <sys/stat.h>   // mkfifo
#include <sys/errno.h>  // errno
#include <stdio.h>      // perror
#include <fcntl.h>      // open
#include <unistd.h>     // close
#include "../include/aux.h"        // read_from_server_pipe
#include <stdlib.h>     // exit
#include "../include/aux_2.h"      // signal handler
#include <signal.h>     // sigaction

#define SERVER_PIPE "server.pipe"
#define PERMS   0666

int global_JobID;          // Υποθέτουμε ότι τα job ids ξεκινούν από το 1
int global_Running;         // Εργασίες που τρέχουν αυτή τη στιγμή (initialized by defualt to zero)
int global_Concurrency = 1;     // Προκαθορισμένη τιμή σύμφωνα με piazza
int exit_server;            // by default is set to zero
int read_sp;

vector<struct info_running> global_Running_Jobs;        // vector με {pid, job_XX} που τρέχουν αυτή τη στιγμή

int main(void) {

    // O_CREAT = if file doesn't exist, open creats it
    // O_RDWR = because server wants to write in the file
    // open returns a file descriptor
    int txt_fd = open("jobExecutorServer.txt", O_CREAT | O_RDWR, PERMS);    
    if(txt_fd == -1) {
        perror("server: can't open file");
        exit(1);
    }    


    // write in .txt
    int pid_server = getpid();
    string str = to_string(pid_server);     // Γράφουμε το pid σε μορφή χαρακτήρων στο txt, για να είναι readable
    int size = str.size();
    if(size = write(txt_fd, str.c_str(), size) != size) {
        perror("data write error");
        exit(1);
    }


    // Δημιουργούμε ένα non blocking named pipe για reading με όνομα
    // server.pipe
    if ( (mkfifo(SERVER_PIPE, PERMS) < 0) && (errno != EEXIST) ) {    
        perror("can't creat server.pipe");
        exit(1);                           
    }
    // Ανοίγουμε το pipe που μόλις δημιουργήσαμε
    // int read_sp;    // read_server_pipe

    if ( (read_sp = open(SERVER_PIPE, O_RDONLY))  < 0)  {      
        perror("server: can't open read server.pipe");
        exit(1);
    }

    my_queue queue;         // Δημιουργία ουράς
    vector<struct info_commander> active_commanders;  

    static struct sigaction act;
    // set up the signal handler
    act.sa_handler = sigchld_action;
    sigfillset(&(act.sa_mask));
    sigaction(SIGCHLD, &act, NULL);

    while(exit_server == 0) {
        fd_set rfds;
        FD_ZERO(&rfds);                 // Initialize the set of file descriptors

        // Βρίσκουμε το nfds:
        // Η select κάνει watch το server_pipe (read_sp) και τα fd_r που
        // είναι μέσα στο vector active_commanders
        int nfds = read_sp;
        int size = active_commanders.size();
        for(int i = 0; i < size; i++) {
            struct info_commander info = active_commanders[i];
            if(info.fd_r > nfds) {
                nfds = info.fd_r;
            }
        }

        FD_SET(read_sp, &rfds);         // Add fd of server_pipe to the set
        for(int i = 0; i < size; i++) {
            struct info_commander info = active_commanders[i];
            FD_SET(active_commanders[i].fd_r, &rfds);
        }

        int retval = select(nfds + 1, &rfds, NULL, NULL, NULL);
        if (retval == -1) {
            if(errno == EINTR) {        // sigchld stopped select
                exec_jobs(queue);
                continue;               // restart select
            }
            else {
                perror("select()");
                exit(1);
            }
        } 
        if (retval == 0) {
            printf("The timeout expired before any file descriptors became ready.");
            exit(1);
            // Δεν θα μπούμε ποτέ σε αυτό το if, καθώς timeval = NULL (block indefinitely)
        }
        // retval > 0

        // Διαβάζει από το server.pipe, 
        // ανοίγει το άκρο pid_r (pid_w για τον commander),
        // αποθηκεύει το pid_r στο vector
        read_from_server_pipe(active_commanders, &rfds);

        // 2o step while: για κάθε commander στο vector active_commanders
        // διάβασε από το read άκρο του pipe
        read_from_commanders(active_commanders, queue, &rfds);

        // 3ο step while: check concurrency & do execs
        exec_jobs(queue);
    }


    close(read_sp);     // read_server_pipe
    
    if ( unlink(SERVER_PIPE) < 0) {     // No future processes will be able to open the *same* named pipe again.
        perror("server: can't unlink \n");
        exit(1);
    }       


    // close and delete .txt
    close(txt_fd);

    if (unlink("jobExecutorServer.txt") == -1) {
        perror("server: unlink txt");
        exit(1);
    }

    printf("About to exit server. Wait a bit please\n");
    return 0;
}
