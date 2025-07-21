./scripts/multijob.sh scripts/commands_3.txt
./bin/jobCommander issueJob setConcurrency 4
./scripts/multijob.sh scripts/commands_4.txt
./scripts/allJobsStop.sh
./bin/jobCommander  poll running
./bin/jobCommander  poll queued
ps -aux | grep bin/progDelay
