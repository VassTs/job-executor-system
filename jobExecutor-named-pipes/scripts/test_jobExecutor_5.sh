./bin/jobCommander issueJob ./bin/progDelay 1000
./bin/jobCommander issueJob ./bin/progDelay 110
./bin/jobCommander issueJob ./bin/progDelay 115
./bin/jobCommander issueJob ./bin/progDelay 120
./bin/jobCommander issueJob ./bin/progDelay 125
./bin/jobCommander poll running
./bin/jobCommander poll queued
./bin/jobCommander setConcurrency 2
./bin/jobCommander poll running
./bin/jobCommander poll queued
./bin/jobCommander exit
