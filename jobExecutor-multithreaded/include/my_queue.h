#pragma once // #include το πολύ μία φορά

#include <iostream>
#include <string>
#include <list>
#include <vector>
using namespace std;

struct Job {    // everything is public by default
    string jobID;      
    int id;
    vector<string> cmd_prms;   // <εντολή> + <παράμετρος_1> + ... + <παράμετρος_n>
    int clientSocket;       // socket που επιστρέφει η accept() για επικοινωνία με τον συγκεκριμένο πελάτη
};

class my_queue {
    private:
        std::list<struct Job> queue;
    public:
        void push(struct Job);
        int queue_position(int id, int *sock);
        void print_queue(void);     // Βοηθητική συνάρτηση για debugging
        void pop(void);   
        int size(void);             // Όταν κάνω exit, πόσα pop να κάνω
        struct Job top(void);       // Επιστρέφει το 1ο στοιχείο της ουράς
        void erase(int pos);        // Αφαιρεί το <pos>-στο στοιχείο της ουράς
};
