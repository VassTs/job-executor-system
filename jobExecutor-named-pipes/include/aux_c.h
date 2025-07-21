#pragma once

// Δημιουγεί και ανοίγει ένα pipe για writing με όνομα pid_w
// Επιστρέφει το file descriptor του νέου pipe
int create_open_write_pipe(int pid);

// Δημιουγεί και ανοίγει ένα pipe για reading με όνομα pid_r
// Επιστρέφει το file descriptor του νέου pipe
int create_open_read_pipe(int pid);