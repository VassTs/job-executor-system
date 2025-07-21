#include "../include/aux_2.h"
#include "../include/my_io.h"
#include <unistd.h>         // execvp
#include <sys/types.h>      // waitpid
#include <sys/wait.h>       // waitpid
#include "../include/aux.h"            // struct info_running

// Δημιουργεί μία νέα εργασία την οποία και επιστρέφει
struct Job new_job(vector<string>& whole_command) {
  // Νέα εργασία
    struct Job job_ins;     

    // Άυξοντας αριθμός εργασίας -- jobID
    global_JobID++;
    string number = to_string(global_JobID);
    string job_ = "job_";
    job_ins.jobID = job_ + number;

    // ID:
    job_ins.id = global_JobID;

    // cmd_prms:
    int new_reps = whole_command.size() - 1;   // new_reps = <ορίσματα> + <εντολή>
    for(int i = 0; i < new_reps; i++) {
        string str = whole_command[i+1];    // whole_command[0] = issueJob
        job_ins.cmd_prms.push_back(str);
    }
    return job_ins;
}


// Επιστρέφει τριπλέτα <jobID,job,queuePosition> για το job
// που δίνεται ως παράμετρο
string get_triple(my_queue& queue, struct Job& job) {
    // queuePosition:
    int position = queue.queue_position(job.id);   // func prints σε ποια θέση μπήκε

    // job:
    string triple = "<" + job.jobID + ",";
    for(int i = 0; i < job.cmd_prms.size(); i++) {
        triple = triple + job.cmd_prms[i]; 
        if(i < job.cmd_prms.size() - 1) {     // Μετά από κάθε όρισμα (εκτός από το τελευταίο), προσθέτουμε ένα κενό
        triple = triple + " "; 
        }
    }

    triple = triple + "," + to_string(position) + ">";  

    return triple;
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

// signal handler functions should be TINY
void sigchld_action(int) {
// Μόλις έλαβα ένα σήμα SIGCHLD και γι' αυτό κλήθηκε ο signal handler
// Ελέγχω πόσα παιδιά τερμάτισαν, γιατί μπορεί να έχω λάβει παραπάνω από ένα SIGCHLD που απλά φαίνεται ως 1     // (**)
    int count = 0;          // μετρητής για το πόσα παιδιά στην πραγματικότητα έχουν τερματίσει, παρόλο που έλαβα 1 SIGCHLD
    while(1) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);      // we request any child

        if(pid == 0) {
            break;      // break endless loop
        } else if(pid == -1){
            if(errno == ECHILD) {
                break;
            } else {
                perror("Error at calling waitpid. Check errno!\n");   // 2 more possible errors: EINTR, EINVAL
                exit(1);      // break endless loop
            }
        }

        // case: pid > 0
        /* See line 332 of aux.cpp
        if((status >> 8) != 0) {    // Το παιδί δεν τερμάτισε με success status
            exit(1);                // Να τερματίσει ο server
        }       
        */
        count++;
        // Δεν βάζω να γίνει exec εδώ κάτι. Τα execs τα κάνω σε 1 σημείο, και αυτό είναι στο 3ο μέρος του while
    
        // Αφαίρεση εργασίας από το vector με τα running processes:
        vector<struct info_running>::iterator it;
        for(it = global_Running_Jobs.begin(); it != global_Running_Jobs.end(); it++) {
            struct info_running info = *it;
            if(info.pid == pid) {
                global_Running_Jobs.erase(it);
                break;          // IMPORTANT
            }
        }

    }

    global_Running = global_Running - count;
}

// Θέτει το global_Currency = new_concurrency
// Επιστρέφει στον commander μηδέν μηνύματα
// Αναφορικά με την εφαρμογή του new_concurrecy:
// [piazza] Αν ο βαθμός παραλληλίας αλλάξει, ο jobExecutor θα προσαρμοστεί 
// αντίστοιχα, χωρίς να σταματήσει εργασίες που ήδη τρέχουν, σε περίπτωση
// μείωσης του βαθμού παραλληλίας.
void handle_setConcurrency(int pid, int new_concurrency) {
// [1] Set the concurrency of the server to new_concurrency
    global_Concurrency = new_concurrency;   // (*)

// [2] Επιστροφή μηνήματος στον jobCommander (none message)
    // open pid_w, write at pid_w, close pid_w, return

    // open pid_r του commander (write για εμάς)
    int pidwritefd = open_write_end_commander(pid);

    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    int repeat = 0;         // Δεν επιστρέφουμε κάποιο μήνυμα στον jobCommander
    write_number_to_fd(pidwritefd, repeat);

    // Για κάθε πρόταση:
    for(int i = 0; i < repeat; i++) {
    }

    // close pid_r
    close(pidwritefd);

    // return
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


void handle_stop(int pid, my_queue& queue, string& job_XX) {
    int id = subtract_integer(job_XX);

// Η εργασία δεν βρέθηκε ούτε στην ουρά, ούτε στο vector (not queued, not running)
    string to_return = "Process to stop doesn't exist";

// Ελέγχουμε αν η εργασία υπάρχει στην ουρά
    int pos = queue.queue_position(id);     // returns one-based
    if(pos != -1) {     // Η εργασία είναι στην ουρά

        // Αφαίρεση εργασίας από την ουρά
        queue.erase(pos);

        // TO-DO: Επιστροφή μηνύματος στον jobCommander; job_XX removed
        to_return = job_XX + " removed";
        // cout << to_return;

    } else {            // Η εργασία δεν είναι στην ουρά

// Ελέγχουμε αν η εργασία είναι στο vector με τα currently running processes
        vector<struct info_running>::iterator it;
        for(it = global_Running_Jobs.begin(); it != global_Running_Jobs.end(); it++) {
            struct info_running info = *it;
            if(info.job_XX.compare(job_XX) == 0) {

                // send kill to that job
                if(kill(info.pid, SIGKILL) == 0) {      // SIGTERM?
                    // parent sends sigkill to child
                    // operating system sends sigchld to parent
                    // Άρα, ο handler θα αφαιρέσει το process από το vector με τα running processes
                } else {        // -1 was returned
                    perror("kill in stop");     
                    // errno == ESRCH (the target process doesn't exist), aka
                    // μέχρι να στείλουμε το kill, η εργασία τερμάτισε πριν από εμάς
                    exit(1);
                }

                to_return = job_XX + " terminated";
                break;          // IMPORTANT
            }
        }
    }

// Επιστροφή μηνύματος στον jobCommander
    // open pid_w, write at pid_w, close pid_w, return

    // open pid_r του commander (write για εμάς)
    int pidwritefd = open_write_end_commander(pid);

    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    int repeat = 1;
    write_number_to_fd(pidwritefd, repeat);

    // Για κάθε πρόταση:
    for(int i = 0; i < repeat; i++) {
        // Γράφει στο fd το μέγεθος του string, και στη συνέχεια το ίδιο το string 
        write_string_to_fd(pidwritefd, to_return);
    }

    // close pid_r
    close(pidwritefd);

    // return
    return;
}


void handle_poll_running(int pid) {
    // Όσο βρισκόμαστε μέσα στη συνάρτηση, ενδέχεται να αλλάξουν τα running jobs
    // Γι' αυτό και κρατάμε μια προσωρινή αντιγραφή αυτών
    vector<struct info_running> temp_running_jobs = global_Running_Jobs;
    int size = temp_running_jobs.size();

// Επιστροφή τριπλετών στον jobCommander
    // open pid_w, write at pid_w, close pid_w, return

    // open pid_r του commander (write για εμάς)
    int pidwritefd = open_write_end_commander(pid);

    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    if(size == 0) {         // There are no running processes
        write_number_to_fd(pidwritefd, 1);
        string to_return = "There is no running process";
        write_string_to_fd(pidwritefd, to_return);
        close(pidwritefd);      // close pid_r
        return;

    }
    // Υπάρχουν running processes
    // # προτάσεων = #running jobs = temp_running_jobs.size()
    write_number_to_fd(pidwritefd, size);

    // Για κάθε πρόταση:
    for(int i = 0; i < size; i++) {

        struct info_running info = temp_running_jobs[i];

        string triple = "<" + info.job_XX + ",";
        for(int i = 0; i < info.cmd_prms.size(); i++) {
            triple = triple + info.cmd_prms[i]; 
            if(i < info.cmd_prms.size() - 1) {     // Μετά από κάθε όρισμα (εκτός από το τελευταίο), προσθέτουμε ένα κενό
                triple = triple + " "; 
            }
        }

        // Έχουμε φροντίσει τα running processes να γίνονται inserted στο vector σαν να ήταν ουρά.
        // Συνεπώς, η θέση του running process που προκύπτει από το random access του vector, είναι σωστή 
        // i+1, καθώς το vector position είναι zero based, αλλά εμείς θέλουμε να εμφανίζεται στον χρήστη ως one based 
        triple = triple + "," + to_string(i+1) + ">";       

        // Γράφει στο fd το μέγεθος του string, και στη συνέχεια το ίδιο το string 
        write_string_to_fd(pidwritefd, triple);
    }

    // close pid_r
    close(pidwritefd);

    // return
    return;
}

void handle_poll_queued(int pid, my_queue& queue) {
    int size = queue.size();

// Επιστροφή τριπλετών στον jobCommander
    // open pid_w, write at pid_w, close pid_w, return

    // open pid_r του commander (write για εμάς)
    int pidwritefd = open_write_end_commander(pid);

    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    if(size == 0) {         // There are no queued processes
        write_number_to_fd(pidwritefd, 1);
        string to_return = "There is no queued process";
        write_string_to_fd(pidwritefd, to_return);
        close(pidwritefd);      // close pid_r
        return;

    }
    // Υπάρχουν queued processes
    // # προτάσεων = #queued jobs = queue.size()
    write_number_to_fd(pidwritefd, size);

    // Για κάθε πρόταση:
    // Δεν μπορούμε να κάνουμε διάσχιση μέσα σε μία ουρά.
    // Γι' αυτό και δημιουργούμε ένα αντίγραφο της ουράς, από την οποία με top & pop
    // θα διασχίσουμε σιγά-σιγά την ουρά
    my_queue temp_queue = queue;        // default copy constructor of my_queue will be called (since no copy constructor for my_queue is defined) (aka, copy constructor of std::list)

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

        int pos = queue.queue_position(job.id);     // Από την ουρά που λάβαμε ως παράμετρο βρίσκουμε το pos της εργασίας μέσα στην ουρά
        triple = triple + "," + to_string(pos) + ">";       

        // Γράφει στο fd το μέγεθος του string, και στη συνέχεια το ίδιο το string 
        write_string_to_fd(pidwritefd, triple);
    }

    // close pid_r
    close(pidwritefd);

    // return
    return;
}


void handle_exit(int pid, vector<struct info_commander>& active_commanders) {
// Πριν τερματίσει ο server, επιστρέφει μήνυμα στον jobCommander
    // open pid_w, write at pid_w, close pid_w, return

    // open pid_r του commander (write για εμάς)
    int pidwritefd = open_write_end_commander(pid);

    // Λέμε στον commander πόσες προτάσεις θα χρειαστεί να διαβάσει
    int repeat = 1;
    write_number_to_fd(pidwritefd, repeat);

    // Για κάθε πρόταση:
    for(int i = 0; i < repeat; i++) {
        // Γράφει στο fd το μέγεθος του string, και στη συνέχεια το ίδιο το string
        string to_return = "jobExecutorServer terminated."; 
        write_string_to_fd(pidwritefd, to_return);
    }

    // close pid_r
    close(pidwritefd);


// Τερματισμός server

    // [1] Κάθε running διεργασία την κάνουμε kill  [δεν έχω κάνει temporal copy]
    int size  = global_Running_Jobs.size();
    for(int i = 0; i < size; i++) {
        struct info_running info = global_Running_Jobs[i];

        // send kill to that job
        if(kill(info.pid, SIGKILL) == 0) {      // SIGTERM?
            // parent sends sigkill to child
            // operating system sends sigchld to parent
            // Άρα, ο handler θα αφαιρέσει το process από το vector με τα running processes
        } else {        // -1 was returned
            perror("kill in exit");     
            // errno == ESRCH (the target process doesn't exist), aka
            // μέχρι να στείλουμε το kill, η εργασία τερμάτισε πριν από εμάς
            exit(1);
        }
    }

    // [2] Για κάθε commander που υπάρχει μέσα στο vector active_commanders 
    // κλείνουμε το fd_r (aka, κλείνουμε το read άκρο του server του pipe που δημιούργησε ο commander)
    // Μέσα στο vector active_commanders, βρίσκονται όλοι οι commanders για τους οποίους το read άκρο του
    // server του pipe είναι ανοικτό, αλλά ο commander μέχρι πρότινος δεν είχε γράψει κάτι στο pipe
    size = active_commanders.size();
    for(int i = 0; i < size; i++) {
        struct info_commander info = active_commanders[i];
        if(info.fd_r != -1) {
            close(info.fd_r);
        }
    }
    active_commanders.clear();              // Αδειάζουμε το vector active_commanders

    // [3] flag exit_server = 1
    exit_server = 1;

    // [4] return;
    return;
}


/*
(*): Από piazza:
Απο την εκφωνηση:

Αν ο βαθμός παραλληλίας αλλάξει, ο jobExecutor θα
προσαρμοστεί αντίστοιχα, χωρίς να σταματήσει εργασίες που ήδη τρέχουν, σε περίπτωση
μείωσης του βαθμού παραλληλίας.
*/
