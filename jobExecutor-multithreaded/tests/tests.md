> test #1
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob touch myFile.txt
ls myFile.txt
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob rm myFile.txt
ls myFile.txt
./bin/jobCommander linux04.di.uoa.gr 2034 exit

> test #2
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob ./bin/progDelay 2
./bin/jobCommander linux04.di.uoa.gr 2034 exit

> test #3
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob ls -l src/
./bin/jobCommander linux04.di.uoa.gr 2034 exit

> test #4
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob ./bin/progDelay 10
./bin/jobCommander linux04.di.uoa.gr 2034 poll
./bin/jobCommander linux04.di.uoa.gr 2034 exit

> test #5
./bin/jobCommander linux04.di.uoa.gr 2034 setConcurrency 2 
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob ./bin/progDelay 5 
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob ./bin/progDelay 5 
./bin/jobCommander linux04.di.uoa.gr 2034 setConcurrency 1 
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob ./bin/progDelay 5 
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob ./bin/progDelay 5 
./bin/jobCommander linux04.di.uoa.gr 2034 poll
./bin/jobCommander linux04.di.uoa.gr 2034 exit

> test #6
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob ./bin/progDelay 10 
./bin/jobCommander linux04.di.uoa.gr 2034 issueJob ./bin/progDelay 10 
./bin/jobCommander linux04.di.uoa.gr 2034 stop job_2       
./bin/jobCommander linux04.di.uoa.gr 2034 exit
