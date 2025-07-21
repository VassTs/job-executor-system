#pragma once // #include το πολύ μία φορά

#include "../include/my_queue.h"

extern int global_JobID;
extern int global_Running;
extern int global_Concurrency;

// Δημιουργεί μία νέα εργασία την οποία και επιστρέφει
struct Job new_job(vector<string>& whole_command);

// Επιστρέφει τριπλέτα <jobID,job,queuePosition> για το job
// που δίνεται ως παράμετρο
string get_triple(my_queue& queue, struct Job& job);

// Λαμβάνει την εντολή + τις παραμέτρους σε ένα vector
// τρέχει την execvp για την παραπάνω εντολή
void my_execvp(vector<string>& args);

// signal handler functions should be TINY
void sigchld_action(int);

// Θέτει το global_Currency = new_concurrency
// Επιστρέφει στον commander μηδέν μηνύματα
void handle_setConcurrency(int pid, int new_concurrency);


void handle_stop(int pid, my_queue& queue, string& job_XX);


void handle_poll_running(int pid);


void handle_poll_queued(int pid, my_queue& queue);


void handle_exit(int pid, vector<struct info_commander>& active_commanders);