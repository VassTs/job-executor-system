#include <sys/types.h>  // mkfifo
#include <sys/stat.h>   // mkfifo
#include <fcntl.h>      // open
#include <string>
using namespace std;

#define PERMS   0666

// Δημιουγεί και ανοίγει ένα pipe για writing με όνομα pid_w
// Επιστρέφει το file descriptor του νέου pipe
int create_open_write_pipe(int pid) {
// Όνομα του pipe
    string number = to_string(pid);
    string w = "_w";
    string pid_w = number + w;

// Δημιουργία του pipe
    if ( (mkfifo(pid_w.c_str(), PERMS) < 0) && (errno != EEXIST) ) {    
        perror("can't creat pid_w");
        exit(1);                          
    }    


// Άνοιγμα του pipe
    int writefd; 
    if ( (writefd = open(pid_w.c_str(), O_WRONLY))  < 0)  {              
        perror("commander: can't open write pid_w");
        exit(1);
    }

    return writefd;
}


// Δημιουγεί και ανοίγει ένα pipe για reading με όνομα pid_r
// Επιστρέφει το file descriptor του νέου pipe
int create_open_read_pipe(int pid) {
// Όνομα του pipe
    string number = to_string(pid);
    string r = "_r";
    string pid_r = number + r;

// Δημιουργία του pipe
    if ( (mkfifo(pid_r.c_str(), PERMS) < 0) && (errno != EEXIST) ) {    
        perror("can't creat pid_r");
        exit(1);                           
    }    

// Άνοιγμα του pipe
    int readfd;
    if ( (readfd = open(pid_r.c_str(), O_RDONLY))  < 0)  {              
        perror("commander: can't open read pid_r");
    }

    return readfd;
}