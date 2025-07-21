#!/bin/bash
if [ "$#" -eq 0 ]
then
  echo "No arguments supplied"
  exit 1
fi

repeat=$#           # Πλήθος παραμέτρων που δόθηκαν στο script = πλήθος αρχείων από τα οποία χρειάζεται να διαβάσουμε τα περιεχόμενα τους

# Αποοθηκεύουμε τις παραμέτρους που δόθηκαν στο script σε έναν πίνακα
args=("$@")

# Για κάθε αρχείο που δόθηκε ως παράμετρο
for (( i = 0; i < repeat; i++ ));
do
    # last line of input(s) file(s), should be an empty line
    # otherwise last line of file, won't be exec'ed
    # (*)
    while IFS= read -r line     # καταλαβαίνει ότι είναι το τέλος της γραμμής, επειδή κάθε γραμμή τελειώνει με $
    do    
      ./bin/jobCommander issueJob $line

    done < "${args[i]}"       # το input της while δίνεται από τις γραμμές του specified file

done

# By default η IFS περιέχει white space χαρακτήρες
# Κάνουμε την IFS (Internal Field Separator) να είναι empty ως εξής: 'IFS= '
# Όταν η IFS είναι empty, τότε η read διαβάζει όλη τη γραμμή χωρίς να σταματά σε πιθανά white spaces
# Η read διαβάσει μέχρι να συναντήσει new line
# Ο λόγος που στην read δίνουμε την παράμετρο -r είναι: ώστε να μην κάνει interpet τα backslashes ως escape χαρακτήρες

# Δεν κάνουμε exit τον server, καθώς στο test_sh_scripts_2.sh έχουμε:
# ./multijob.sh commands_4.txt    # don't exit
# ./allJobsStop.sh                # server should be active before running this script