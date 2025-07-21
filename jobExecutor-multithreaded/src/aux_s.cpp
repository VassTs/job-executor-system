#include <pthread.h>          // For threads
#include <sys/types.h>        // socket
#include <sys/socket.h>       // socket
#include <netinet/in.h>       // internet sockets
#include <stdlib.h>           // exit
#include <stdio.h>            // perror
#include <unistd.h>           // sleep
#include <assert.h>
#include <string.h>           // For strerror
#include "../include/aux_s.h"
#include "../include/my_io.h"
#include "../include/my_queue.h"
#include <sys/wait.h>

#include <fcntl.h>      // open
#define PERMS   0666

#include <vector>
using namespace std;

void my_atoi(char* argv[], int* portnum, int* bufferSize, int* threadPoolSize) {
 /* Convert port number to integer */
    *portnum = atoi(argv[1]);            

/* Convert bufferSize to integer */
    *bufferSize = atoi(argv[2]);  
    assert(*bufferSize > 0);          

/* Convert threadPoolSize to integer */
    *threadPoolSize = atoi(argv[3]); 
    assert(*threadPoolSize > 0);           

    return;
}


// Δημιουργεί μία νέα εργασία την οποία και επιστρέφει
struct Job new_job(int newsock, vector<string>& whole_command) {
  // Νέα εργασία
    struct Job job_ins;     

    // Άυξοντας αριθμός εργασίας -- jobID
    pthread_mutex_lock(&global_Mtx_JobID);       // returns immediately if mutex is unlocked, otherwise it blocks
    global_JobID++;
    job_ins.id = global_JobID;
    pthread_mutex_unlock(&global_Mtx_JobID);     // returns if mutex gets unlocked successfully


    string number = to_string(job_ins.id);
    string job_ = "job_";
    job_ins.jobID = job_ + number;


    // cmd_prms:
    int new_reps = whole_command.size() - 1;   // new_reps = <ορίσματα> + <εντολή>
    for(int i = 0; i < new_reps; i++) {
        string str = whole_command[i+1];    // whole_command[0] = issueJob
        job_ins.cmd_prms.push_back(str);
    }

    // clientSocket
    job_ins.clientSocket = newsock;

    return job_ins;
}


// Επιστρέφει τριπλέτα <jobID,job> για το job
// που δίνεται ως παράμετρο
string get_triple(struct Job& job) {
    // job:
    string triple = "<" + job.jobID + ",";
    for(int i = 0; i < job.cmd_prms.size(); i++) {
        triple = triple + job.cmd_prms[i]; 
        if(i < job.cmd_prms.size() - 1) {     // Μετά από κάθε όρισμα (εκτός από το τελευταίο), προσθέτουμε ένα κενό
        triple = triple + " "; 
        }
    }

    triple = triple + ">";  

    return triple;
}


void handle_issueJob(int newsock, vector<string>& whole_command) {
// [1] Εισάγουμε την εργασία στην ουρά
    // whole_command[0] == "issueJob"
    // Νέα εργασία
    struct Job job_ins = new_job(newsock, whole_command);     

// PRODUCER CODE
    // Εισάγουμε την εργασία στην ουρά
    pthread_mutex_lock(&global_Mtx_Queue);         // acquire the lock για πρόσβαση στην ουρά
    while (global_Queue.size() == global_BufferSize) {       // check if the buffer is full
        if(exit_server == 1) {
            pthread_mutex_unlock(&global_Mtx_Queue);        // τελικά δεν θα προσθέσω στην ουρά
            string to_return = "SERVER TERMINATED BEFORE INSERTING TO BUFFER + EXECUTION";
            write_number_to_socket(newsock, 1);     // Ο commander θα διαβάσει 1 πρόταση
            write_string_to_socket(newsock, to_return);

            write_number_to_socket(newsock, 0);     // Ο commander θα διαβάσει 0 χαρακτήρες από αποτελέσματα εργασίας
            close(newsock);
            return;     // thread controller about to exit
        }
        // wait for buffer to have at least 1 empty slot
        pthread_cond_wait(&global_Cond_nonfull, &global_Mtx_Queue);     // automatically unlocks mutex and blocks      
        // once signaled, mutex is automatically locked 
    }
    global_Queue.push(job_ins);                         // produce!
    pthread_cond_signal(&global_Cond_nonempty);             // signal that the buffer has at least 1 full slot
    pthread_mutex_unlock(&global_Mtx_Queue);           // release the lock
// END OF PRODUCER CODE

    // Δημιουργία τριπλέτας
    string triple = get_triple(job_ins);


// [2] Επιστροφή τριπλέτας στον jobCommander
    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    int repeat = 1;   
    write_number_to_socket(newsock, repeat);

    string to_return = "JOB " + triple + " SUBMITTED";

    // Για κάθε πρόταση:
    for(int i = 0; i < repeat; i++) {
        write_string_to_socket(newsock, to_return);
    }
    return;
}


void handle_setConcurrency(int newsock, int new_concurrency) {
// [1] Set the concurrency of the server to new_concurrency
    pthread_mutex_lock(&global_Mtx_Concurrency);       // returns immediately if mutex is unlocked, otherwise it blocks
    global_Concurrency = new_concurrency;
    pthread_cond_broadcast(&global_Cond_nonfull_Running);
    /* ^ Όποιος κάνει wait στο cond_var: global_Cond_nonfull_Running, τον ξυπνάω 
    για να τον ενημερώσω ότι άλλαξε το concurrency, οπότε να τσεκάρει ξανά σε συνδυασμό
    και με το global_Running, αν μπορεί να τρέξει. */
    pthread_mutex_unlock(&global_Mtx_Concurrency);     // returns if mutex gets unlocked successfully

// [2] Επιστροφή μηνύματος στον jobCommander
    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    int repeat = 1;   
    write_number_to_socket(newsock, repeat);

    string to_return = "CONCURRENCY SET AT " + to_string(new_concurrency);      // Όχι χρήση του global_Concurrency γιατί τότε θα πρέπει να ξανακατεβάσουμε τον mutex!

    // Για κάθε πρόταση:
    for(int i = 0; i < repeat; i++) {
        write_string_to_socket(newsock, to_return);
    }
    close(newsock);

    return;
}


// Λαμβάνει ως παράμετρο ένα string της μορφής job_XX,
// και επιστρέφει το ΧΧ ως ακέραια μεταβλητή
int subtract_integer(string& job_XX) {
    size_t pos_ = job_XX.find('_');       // Searches the string for the first occurrence of the sequence specified by its arguments.
    
    // Αφαιρούμε το substring μετά το _ και το επιστρέφουμε
    string chars_id = job_XX.substr(pos_ + 1);      // The substring is the portion of the object that starts at character position pos and spans len characters (or until the end of the string, whichever comes first).
    
    // Μετατρέπουμε το id από χαρακτήρες σε ακέραιο
    int number = stoi(chars_id);
    return number;
}

void handle_stop(int newsock, string& job_XX) {
    int id = subtract_integer(job_XX);
    string to_return;

// [1] Ελέγχουμε αν η εργασία υπάρχει στην ουρά
    pthread_mutex_lock(&global_Mtx_Queue);       // returns immediately if mutex is unlocked, otherwise it blocks
    int job_socket;
    int pos = global_Queue.queue_position(id, &job_socket);     // returns one-based

    if(pos != -1) {     // Η εργασία είναι στην ουρά
        // Αφαίρεση εργασίας από την ουρά
        global_Queue.erase(pos);
        to_return = "JOB <" + job_XX + "> REMOVED";

        // Αποστολή στον commander του job που μόλις κάναμε remove το εξής μήνυμα:
        string to_return = "Your job was removed from buffer before execution by a different commander";
        write_string_to_socket(job_socket, to_return);
        close(job_socket);

    } else {            // Η εργασία δεν είναι στην ουρά
        to_return = "JOB <" + job_XX + "> NOTFOUND";
    }
    pthread_mutex_unlock(&global_Mtx_Queue);     // returns if mutex gets unlocked successfully


// [2] Επιστροφή μηνύματος στον commander
// Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    int repeat = 1;   
    write_number_to_socket(newsock, repeat);

    // Για κάθε πρόταση:
    for(int i = 0; i < repeat; i++) {
        write_string_to_socket(newsock, to_return);
    }
    close(newsock);
    return;
}


void handle_poll(int newsock) {
    pthread_mutex_lock(&global_Mtx_Queue);       // returns immediately if mutex is unlocked, otherwise it blocks
    int size = global_Queue.size();

// Επιστροφή τριπλετών στον jobCommander
    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    if(size == 0) {         // There are no queued processes
        write_number_to_socket(newsock, 1);
        string to_return = "There is no queued process";
        write_string_to_socket(newsock, to_return);
        
        pthread_mutex_unlock(&global_Mtx_Queue);     // returns if mutex gets unlocked successfully
        close(newsock);
        return;

    }
    // Υπάρχουν queued processes
    // # προτάσεων = #queued jobs = queue.size()
    write_number_to_socket(newsock, size);

    // Για κάθε πρόταση:
    // Δεν μπορούμε να κάνουμε διάσχιση μέσα σε μία ουρά.
    // Γι' αυτό και δημιουργούμε ένα αντίγραφο της ουράς, από την οποία με top & pop
    // θα διασχίσουμε σιγά-σιγά την ουρά
    my_queue temp_queue = global_Queue;        // default copy constructor of my_queue will be called (since no copy constructor for my_queue is defined) (aka, copy constructor of std::list)

    for(int i = 0; i < size; i++) {
        struct Job job = temp_queue.top();
        temp_queue.pop();

        string triple = "<" + job.jobID + ",";
        for(int i = 0; i < job.cmd_prms.size(); i++) {
            triple = triple + job.cmd_prms[i]; 
            if(i < job.cmd_prms.size() - 1) {     // Μετά από κάθε όρισμα (εκτός από το τελευταίο), προσθέτουμε ένα κενό
                triple = triple + " "; 
            }
        }

        triple = triple + ">";       

        // Γράφει στο fd το μέγεθος του string, και στη συνέχεια το ίδιο το string 
        write_string_to_socket(newsock, triple);
    }

    pthread_mutex_unlock(&global_Mtx_Queue);     // returns if mutex gets unlocked successfully
    close(newsock);

    // return
    return;
}


void handle_exit(int newsock) {
    exit_server = 1;
    // Ξυπνάμε όσους workers περιμένουν να γίνουν approved από το concurrency, global_Running
    pthread_cond_broadcast(&global_Cond_nonfull_Running);

    // Ξυπνάμε όσους workers περιμένουν μέχρι η ουρά να έχει τουλάχιστον 1 item
    pthread_cond_broadcast(&global_Cond_nonempty);

    // Ξυπνάμε όσους controllers περιμένουν μέχρι η ουρά να έχει τουλάχιστον 1 empty slot
    pthread_cond_broadcast(&global_Cond_nonfull);

// Όλους τους παραπάνω τους ξυπνάμε, προκειμένω να μην μείνουν μπλοκαρισμένοι για πάντα.
// Μόλις ξεμπλοκαριστούν, είναι ο κώδικας έτσι, που αμέσως θα ελέγξουν αν το exit_server == 1,
// οδηγώντας τελικά στην ακαριαία ολοκλήρωση του εκάστοτε thread.

// [1] Πριν τερματίσει ο server, επιστρέφει μήνυμα στον jobCommander
    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    int repeat = 1;
    write_number_to_socket(newsock, repeat);

    // Για κάθε πρόταση:
    for(int i = 0; i < repeat; i++) {
        // Γράφει στο socket το μέγεθος του string, και στη συνέχεια το ίδιο το string
        string to_return = "SERVER TERMINATED"; 
        write_string_to_socket(newsock, to_return);
    }

    close(newsock);    

// [2] Απελευθέρωση πόρων
    // We want to unblock accept
    kill(getpid(), SIGUSR1);

    return;
}



void handle_command(int newsock, vector<string>& whole_command) {
// We decode τι τύπου εντολή είναι: -- issuejob, setconcurrency ...?
    if(whole_command.size() == 0) {
        // Δεν δόθηκε τύπος εντολής
        exit(1);
    }

    if(whole_command[0].compare("issueJob") == 0) {

        if(whole_command.size() == 1) {
            exit(1);      
        }
        
        handle_issueJob(newsock, whole_command);

    } else if(whole_command[0].compare("setConcurrency") == 0) {

        if(whole_command.size() != 2) {
            exit(1);
        }

        int new_concurrency = stoi(whole_command[1]);
        if(new_concurrency < 1) {
            exit(1);        
        }

        handle_setConcurrency(newsock, new_concurrency); 

    } else if(whole_command[0].compare("stop") == 0) {
    

        if(whole_command.size() != 2) {
            exit(1);
        }

        handle_stop(newsock, whole_command[1]);


    } else if(whole_command[0].compare("poll") == 0) {

        if(whole_command.size() != 1) {
            exit(1);
        }

        handle_poll(newsock);


    } else if(whole_command[0].compare("exit") == 0) {
    

        if(whole_command.size() != 1) {
            exit(1);
        }

        handle_exit(newsock);

    } else {
        exit(1);
    }


}

void read_command_and_handle(int newsock) {
    int reps = read_int_from_socket(newsock);       // Από πόσες λέξεις αποτελείται η εντολή που θέλει να μάς στείλει ο commander   

    vector<string> whole_command;

    // Για κάθε όρισμα + εντολή + τύπου εντολής:
    for(int i = 0; i < reps; i++) {
        // Στην τρέχουσα επανάληψη, θα χρειαστεί να διαβάσουμε size bytes
        int size = read_int_from_socket(newsock);

        string str(size, '\0');     // Αρχικοποιούμε ένα string μεγέθους <size> με τον χαρακτήρα '\0' (call of string constructor) 

        // Διαβάζουμε αυτά τα size bytes   
        read_string_from_socket(newsock, str, size);

        whole_command.push_back(str);
    }

    handle_command(newsock, whole_command);

    return;
}


// Thread function for controller thread
void *controller_f(void *argp){ /* Thread function */
    pthread_mutex_lock(&global_Mtx_Controller_Count);
    global_ControllerCount++;
    pthread_mutex_unlock(&global_Mtx_Controller_Count);

    int* newsock_ptr = (int*)argp;
    int newsock = *newsock_ptr;    

    read_command_and_handle(newsock);

    int err = pthread_detach(pthread_self());
    if (err != 0) {
        fprintf(stderr, "pthread_detach: %s\n", strerror(err));
        exit(EXIT_FAILURE);  
    }

    /* The pthread_detach() function marks the thread identified by
    thread as detached.  When a detached thread terminates, its
    resources are automatically released back to the system without
    the need for another thread to join with the terminated thread.

    Attempting to detach an already detached thread results in
    unspecified behavior. */

    pthread_mutex_lock(&global_Mtx_Controller_Count);
    global_ControllerCount--;
    pthread_mutex_unlock(&global_Mtx_Controller_Count);

    free(newsock_ptr);

    return NULL;        // equivalent to pthread_exit(NULL), where
    // NULL = the thread should exit without returning any specific value to the joining thread. 
}

// Λαμβάνει την εντολή + τις παραμέτρους σε ένα vector
// τρέχει την execvp για την παραπάνω εντολή
void my_execvp(vector<string>& args) {
    // Convert vector of strings to array of C-style strings
    vector<char*> c_args;         // TO-DO: change name
    for(int i = 0; i < args.size(); i++) {
        // c_str() returns a const char*, and push_back() requires a non-const pointer.
        // therefore, we use const_cast to remove the const qualifier.
        c_args.push_back(const_cast<char*>(args[i].c_str()));   
    }

    c_args.push_back(NULL);

    // Reminder: int execvp(const char *file , char * const argv[]); 
    // execvp require the passing of both executable and its arguments in an array: 
    // argv[0] = executable
    // argv[1] - argv[n] = arguments
    // argv[n+1] = NULL

    // They all replace the calling process (including text, data, bss, stack) with the
    // executable designated by either the path or file.
    // Άρα, file = executable

    // std::vector::data: returns a pointer to the first element in the array which is used internally by the vector.
    // elements in the vector are guaranteed to be stored in contiguous storage locations
    execvp(c_args[0], c_args.data());  
}

void child_code(struct Job& job) {
// Δημιουργία αρχείου
    int pid = getpid();
    string str_pid = to_string(pid);
    string filename = str_pid + ".output";

    // O_CREAT = if file doesn't exist, open creats it
    // O_RDWR = because server wants to write in the file
    // open returns a file descriptor
    int fd = open(filename.c_str(), O_CREAT | O_RDWR, PERMS);    
    if(fd == -1) {
        perror("can't open file");
        exit(1);
    }   
// Ανακατεύθυνση εξόδου
    int err = dup2(fd, 1);    // 1 = stdout (αντί να γράφει στο stdout, θα γράφει στο αρχείο)
    // the fd and 1 may be used interchangeably
    if(err == -1) {
        perror("dup2");
        exit(1);
    }
    close(fd);      // stdout is still open and points to fd

// exec
    my_execvp(job.cmd_prms);
    perror("my_execvp");       // If execvp returns, it means an error occurred
    exit(1);        // Αν κάνει fail η execvp, δεν θα κάνει exit ο server
}

void parent_code(int child_pid, struct Job& job) {
// Άνοιγμα αρχείου

    string str_child_pid = to_string(child_pid);
    string filename = str_child_pid + ".output";

    int fd = open(filename.c_str(), O_RDWR);      // We open the file in read and write mode
    if (fd == -1) {
        perror("Can't open file in parent");
        exit(EXIT_FAILURE);
    }

    if(lseek(fd, 0, SEEK_END) == -1) {     // move file pointer to end of file
        perror("lseek to end of file");
        exit(1);
    }

    // We can't pre-append a file, therefore we only write at the end of file
    string end = "-----" + job.jobID + " output end------\n";
    if (write(fd, end.c_str(), end.size()) == -1) {
        perror("write to end of file");
        exit(1);
    }

// Πόσους χαρακτήρες περιέχει το αρχείο;
    lseek(fd, 0, SEEK_SET);     // Reset file pointer to beginning of file
    char buffer[200];
    ssize_t n;
    int count = 0;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        count = count + n;
    }
    if (n == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
    // if (n == 0)  // EOF reached!

    string start = "-----" + job.jobID + " output start------\n";
    count = count + start.size();

// Λέμε στον commander πόσους χαρακτήρες θα χρειαστεί να διαβάσει
    write_number_to_socket(job.clientSocket, count);

// // Στέλνουμε τους χαρακτήρες σε chunks των 200 bytes
    lseek(fd, 0, SEEK_SET);     // Reset file pointer to beginning of file

    int loops = count / 200;
    int remaining = count % 200;

    for(int i = 0; i < loops; i++) {
        // Διαβάζουμε 200 χαρακτήρες από το αρχείο
        if(i == 0) {
            strncpy(buffer, start.c_str(), start.size());
            int err = read(fd, buffer + start.size(), 200 - start.size());
            if(err == -1) {
                perror("read");
                exit(1);
            }
        } else {
            int err = read(fd, buffer, sizeof(buffer));
            if(err == -1) {
                perror("read");
                exit(1);
            }
        }

        // Γράφουμε τους 200 χαρακτήρες στο socket
        if(n = write(job.clientSocket, buffer, sizeof(buffer)) != 200) {
            perror("data write error");
            exit(EXIT_FAILURE);
        }
    }

    if(loops == 0) {
        strncpy(buffer, start.c_str(), start.size());
        int err = read(fd, buffer + start.size(), remaining - start.size());
        if(err == -1) {
            perror("read");
            exit(1);
        } 
    } else {
        // Διαβάζουμε <remaining> χαρακτήρες σε πλήθος από το αρχείο
        int err = read(fd, buffer, remaining);
        if(err == -1) {
            perror("read");
            exit(1);
        }
    }

    // Γράφουμε <remaining> χαρακτήρες στο socket
    if(n = write(job.clientSocket, buffer, remaining) != remaining) {
        perror("data write error");
        exit(EXIT_FAILURE);
    }


// Κλείσιμο socket
    close(job.clientSocket);

// Κλείσιμο αρχείου
    close(fd);     

// Διαγραφή αρχείου
    if (unlink(filename.c_str()) == -1) {
        perror("unlink output file");
        exit(1);
    }

// return
    return;
}


// Thread function for worker thread
void *worker_f(void *argp) {

    while(exit_server == 0) {
        pthread_mutex_lock(&global_Mtx_Running);             // acquire the lock of RUNNING
        pthread_mutex_lock(&global_Mtx_Concurrency);
        while (global_Running >= global_Concurrency) {   // check if the running is full
            pthread_mutex_unlock(&global_Mtx_Concurrency);
            pthread_cond_wait(&global_Cond_nonfull_Running, &global_Mtx_Running);   // wait for running to have at least 1 empty slot
            //
            if(exit_server == 1) { // broadcast στο global_Cond_nonfull_Running στην exit
                pthread_mutex_unlock(&global_Mtx_Running); 
                return NULL;       // thread of worker about to exit
            }
            //
            pthread_mutex_lock(&global_Mtx_Concurrency);
        }
        pthread_mutex_unlock(&global_Mtx_Concurrency);
        global_Running++;                       // produce! (αύξηση  running κατά 1) Έχουμε δεσμεύσει τη θέση μας
        pthread_mutex_unlock(&global_Mtx_Running);           // release the lock


    // CONSUMER CODE
        pthread_mutex_lock(&global_Mtx_Queue);           // acquire the lock για πρόσβαση στην ουρά
        while (global_Queue.size() == 0) {           // check if the buffer is empty
            // wait for the buffer to have at least 1 full slot
            pthread_cond_wait(&global_Cond_nonempty, &global_Mtx_Queue);      // automatically unlocks mutex and blocks
            // once signaled, mutex is automatically locked 

            // S_EDIT, με **broadcast στο exit** για το Cond_nonempty [workers που περιμένουν σε άδεια ουρά]
            if(exit_server == 1) {
                // Έχω δεσμεύσει 1 running slot
                pthread_mutex_unlock(&global_Mtx_Queue);    // δεν θα αφαιρέσω από την ουρά τελικά     
                return NULL;   // // thread of worker about to exit
            }
            // E_EDIT

        }
        struct Job job = global_Queue.top(); // consume!
        global_Queue.pop();
        pthread_cond_signal(&global_Cond_nonfull); // signal that the buffer has at least 1 empty slot
        pthread_mutex_unlock(&global_Mtx_Queue); // release the lock
    // END OF CONSUMER CODE

    // Fork & exec of job
        pid_t pid = fork();
        if (pid == -1){
            perror("Failed to fork");
            exit(1);
        }
        if (pid == 0) {        // child
            child_code(job);
        }       
        else {      // parent
            int status;
            int err = waitpid(pid, &status, 0);
            if (err == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            } 

            parent_code(pid, job);


            pthread_mutex_lock(&global_Mtx_Running);         // acquire the lock
            global_Running--;                  // consume! (μείωση counter κατά 1)
            pthread_cond_signal(&global_Cond_nonfull_Running);        // signal that the buffer has at least 1 empty slot
            pthread_mutex_unlock(&global_Mtx_Running);       // release the lock
        }


    }        // end of while(1)


    return NULL;        // equivalent to pthread_exit(NULL), where
    // NULL = the thread should exit without returning any specific value to the joining thread. 
}
