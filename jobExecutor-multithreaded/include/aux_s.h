#pragma once // #include το πολύ μία φορά

#include "../include/my_queue.h"

extern pthread_mutex_t  global_Mtx_Concurrency;
extern pthread_mutex_t  global_Mtx_JobID;
extern pthread_mutex_t  global_Mtx_Queue;
extern pthread_mutex_t  global_Mtx_Running;
extern pthread_mutex_t  global_Mtx_Controller_Count;


extern my_queue global_Queue;
extern int global_Concurrency;
extern int global_JobID;
extern int global_BufferSize;
extern int global_Running; 
extern int exit_server;            // by default is set to zero
extern int global_ControllerCount; // by default is set to zero

extern pthread_cond_t global_Cond_nonempty;
extern pthread_cond_t global_Cond_nonfull;
extern pthread_cond_t global_Cond_nonfull_Running;

void my_atoi(char* argv[], int* portnum, int* bufferSize, int* threadPoolSize);

void read_command_and_handle(int newsock);

// Thread function for controller thread
void *controller_f(void *argp);

// Thread function for worker thread
void *worker_f(void *argp);
