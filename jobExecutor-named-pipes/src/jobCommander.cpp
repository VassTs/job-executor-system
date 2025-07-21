#include <sys/types.h>  // mkfifo
#include <sys/stat.h>   // mkfifo
#include <stdio.h>      // perror
#include <fcntl.h>      // open
#include <unistd.h>     // close
#include "../include/my_io.h"      // write_number_to_fd
#include "../include/aux_c.h"      // create_open_write_pipe
#include <string>
using namespace std;

#define SERVER_PIPE "server.pipe"
#define PERMS   0666

int main(int argc, char *argv[]) {
   if (access("jobExecutorServer.txt", F_OK) != 0) {         // F_OK tests for the existence of the file.

        pid_t pid = fork();
        if (pid == -1){
            perror("Failed to fork");
            return 1;
        }

        if (pid == 0) {        // child

            execl("./bin/jobExecutorServer", "./bin/jobExecutorServer", NULL);
            perror("execl");
            exit(1);

        } else {                // parent

            while(1) {      // loop-άρουμε μέχρι ο server να δημιουργήσει το αρχείο
                if (access("jobExecutorServer.txt", F_OK) == 0) {
                    break;      // file exists, server created requested file!
                }
            }

        }

    } 
    // .txt exists
    // ** if commander wants to read the txt file, should first open it, read, and then close it


    pid_t pid_c = getpid();     // pid_c = pid_commander (typedef int pid_t)

    // Δημιουργία (χωρίς άνοιγμα yet) του pid_w
    // Όνομα του pipe
        string number = to_string(pid_c);
        string w = "_w";
        string pid_w = number + w;

    // Δημιουργία του pipe
        if ( (mkfifo(pid_w.c_str(), PERMS) < 0) && (errno != EEXIST) ) {    
            perror("can't creat pid_w");
            exit(1);                          
        }    


    // Δημιουργία (χωρίς άνοιγμα yet) του pid_r
    // Όνομα του pipe
        string r = "_r";
        string pid_r = number + r;

    // Δημιουργία του pipe
        if ( (mkfifo(pid_r.c_str(), PERMS) < 0) && (errno != EEXIST) ) {    
            perror("can't creat pid_r");
            exit(1);                           
        }    


    // Ανοίγουμε το non blocking named pipe για writing που έφτιαξε
    // ο server με όνομα server.pipe
    int write_sp;   // write_server_pipe

    // Περιμένουμε μέχρι ο server να δημιουργήσει το server_pipe
    while((write_sp = open(SERVER_PIPE, O_WRONLY))  < 0) {
    }

    // Γράφουμε μέσα στο server.pipe το pid του τρέχον jobCommander
    write_number_to_fd(write_sp, pid_c);

    close(write_sp);
    // (*)

    // Άνοιγμα του pid_w
    int writefd; 
    // the open call will block till the other end is open
    if ( (writefd = open(pid_w.c_str(), O_WRONLY))  < 0)  {              
        perror("commander: can't open write pid_w");
        exit(1);
    }
 
    // Γράφουμε στο pid_w

   // Το πρόγραμμα τρέχει ως: ./jobCommander <εντολή>
   // Στον server περνάμε τα πάντα εκτός του: ./jobCommander
   // ./jobCommander = argv[0]

   // Λέμε στον server πόσα <ορίσματα + εντολή + τύπος εντολής> διαβάσματα θα χρειαστεί να κάνει:
   int reps = argc - 1;
   write_number_to_fd(writefd, reps);

   // Για κάθε όρισμα + εντολή + τύπο εντολής
   for(int i = 1; i <= reps; i++) {
      string str = argv[i]; 

      write_string_to_fd(writefd, str);
   }

    // Άνοιγμα του pid_r, ώστε να μπλοκάρει ο commander μέχρι ο server να 
    // έχει καταναλώσει το pid_w και ανοίξει το pid_r
    int readfd; 
    // the open call will block till the other end is open
    if ( (readfd = open(pid_r.c_str(), O_RDONLY))  < 0)  {              
        perror("commander: can't open read pid_r");
        exit(1);
    }



// Διάβασε από το pid_r
   // Πόσες προτάσεις θα διαβάσουμε από τον server
   int sentences = read_int_from_fd(readfd);        // blocks?


   // Για κάθε πρόταση:
   for(int i = 0; i < sentences; i++) {
      // Πόσα bytes είναι η τρέχουσα πρόταση
      int n = read_int_from_fd(readfd);


      // Πάμε να διαβάσουμε την actual πρόταση
      string str(n, '\0');     // Αρχικοποιούμε ένα string μεγέθους <n> με τον χαρακτήρα '\0' (call of string constructor) 

      // Διαβάζουμε αυτά τα n bytes   
      if(read(readfd, (void*)str.data(), n) != n) {        // Μπορεί να μην τα διαβάζει σε 1 read?
        perror("data read error \n"); 
        exit(1);
      }

      // Εκτυπώνουμε την actual πρόταση στην οθόνη:
      printf("%s\n", str.c_str());

   }

    close(writefd);
    close(readfd);

    // unlink pid_w
    if ( unlink(pid_w.c_str()) < 0) {     // No future processes will be able to open the *same* named pipe again.
        perror("commander: can't unlink \n");
        exit(1);
    }   

    // unlink pid_r
    if ( unlink(pid_r.c_str()) < 0) {     // No future processes will be able to open the *same* named pipe again.
        perror("commander: can't unlink \n");
        exit(1);
    }   

    return 0;
}

/*
(*)
if ( unlink(SERVER_PIPE) < 0) {
    perror("commander: can't unlink \n");
    return 1;
}
Ο λόγος που *δεν* βάζουμε unlink pipe σε αυτό το σημείο είναι επειδή:
[stackoverflow https://stackoverflow.com/questions/47521850/what-happens-if-i-unlink-an-opened-fifo-pipe]: 
with unlink the name is removed from the file system 
so no future processes can open the *same* named pipe.
Οπότε όταν θα πάει να κληθεί ένας νέος commander, δεν θα μπορεί
να ανοίξει το pipe
*/