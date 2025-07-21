#!/bin/bash

if ! [ -e jobExecutorServer.txt ]       # Ελέγχουμε αν ο server είναι ενεργός
then
    echo "Server isn't active. Can't stop any jobs"
    exit 1
fi

input=$(./bin/jobCommander poll queued)  

if [ "$input" = "There is no queued process" ]; then
    echo "There is no queued process. No job to stop"
else
    echo "$input" | while IFS= read -r line; do
        to_stop=$(echo "$line" | cut -d ',' -f 1 | cut -c 2-)
        ./bin/jobCommander stop $to_stop 
    done
fi


input=$(./bin/jobCommander poll running)  

if [ "$input" = "There is no running process" ]; then
    echo "There is no running process. No job to stop"
else

    # Περνάμε το αποτέλεσμα της: echo $input στην while μέσω pipe (|)
    # Θέτουμε την IFS να είναι empty, ώστε η read να διαβάζει όλη τη γραμμή χωρίς να σταματά σε πιθανά white spaces
    # Η read διαβάσει μέχρι να συναντήσει new line
    # Περνάμε στην read την παράμετρο -r, ώστε να μην κάνει interpet τα backslashes ως escape χαρακτήρες 
    echo "$input" | while IFS= read -r line; do
        # cut -delimeter ',' -field 1 (αφαίρεσε το 1ο πεδίο της γραμμής χρησιμοποιώντας ως delimeter το κόμμα)
        # cut -c (πρόκειται να αφαιρέσουμε χαρατήρα) 2- (στο τελικό αποτέλεσμα, συμπεριέλαβε το string από τον 2ο χαρακτήρα μέχρι το τέλος της γραμμής)
        to_stop=$(echo "$line" | cut -d ',' -f 1 | cut -c 2-)       
        ./bin/jobCommander stop $to_stop 
    done
fi


# Δεν κάνουμε τον exit server, γιατί τότε θα σκοτωθούν όλες οι διεργασίες 
# Στην τελευταία γραμμή του test_sh_scripts_2.sh κάνει: ps -aux | grep progDelay
# ώστε να ελέγξει ότι πράγματι το τρέχον script (και όχι ο exit server) έκανε stop όλες τις διεργασίες