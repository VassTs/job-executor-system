#include "../include/my_queue.h"
#include "assert.h"

// Επιστρέφει το 1ο στοιχείο της ουράς
struct Job my_queue::top(void) {
    return queue.front();
}

int my_queue::size(void) {
    return queue.size();
}

// Αφαιρεί το 1ο στοιχείο της ουράς
void my_queue::pop(void) {
    queue.pop_front();      // *** deconstructor of job if defined is called!
}

// Συνάρτηση που εκτυπώνει τα περιεχόμενα της ουράς
void my_queue::print_queue(void) {
    list<struct Job>::iterator it;
	for (it = queue.begin(); it != queue.end(); it++) {
        struct Job job_temp = *it;
		cout << '\t' << job_temp.id;
    }
	cout << '\n';
}

void my_queue::push(struct Job job_ins) {
    queue.push_back(job_ins);
}

// Συνάρτηση που επιστρέφει σε ποια θέση της ουράς είναι το <id>
// Αν δεν βρέθηκε στην ουρά, τότε επιστρέφει -1
int my_queue::queue_position(int id) {
    int position = 1;       // Το 1ο στοιχείο της ουράς είναι το στοιχείο #1
    list<struct Job>::iterator it;
    for(it = queue.begin(); it != queue.end(); it++) {
        struct Job job_temp = *it;
        if(job_temp.id == id) {       // int x = *it; 
            // Το βρήκαμε!
            return position;
        }
        position++;
    }
    return -1;
}


// Αφαιρεί το <pos>-στο στοιχείο της ουράς
// Θεωρούμε ότι το 1ο στοιχείο της ουράς είναι το στοιχείο #1
void my_queue::erase(int pos) {
    int size = queue.size();    // Μέγεθος λίστας/ουράς
    assert((pos >= 1) && (pos <= size));    

    list<struct Job>::iterator it;
    int temp_pos = 1;
    for(it = queue.begin(); it != queue.end(); it++) {
        if(temp_pos == pos) {
            queue.erase(it);    // erase
            return;
        } else {
            temp_pos++;
        }
    }
}