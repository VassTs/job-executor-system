killall ./bin/progDelay
./bin/jobCommander issueJob ./bin/progDelay 1000
./bin/jobCommander issueJob ./bin/progDelay 1000
./bin/jobCommander issueJob ./bin/progDelay 1000
./bin/jobCommander issueJob ./bin/progDelay 1000
./bin/jobCommander issueJob ./bin/progDelay 1000
./bin/jobCommander issueJob ./bin/progDelay 1000
./bin/jobCommander setConcurrency 4
./bin/jobCommander poll running
./bin/jobCommander poll queued
./bin/jobCommander stop job_4
./bin/jobCommander stop job_5
./bin/jobCommander poll running
./bin/jobCommander poll queued
./bin/jobCommander exit
