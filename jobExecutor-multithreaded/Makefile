all:			# default target
	mkdir -p build bin
	make jobCommander
	make jobExecutorServer
	make progDelay

## jobCommander:
build/my_io.o: src/my_io.cpp
	g++ -c -g src/my_io.cpp
	mv my_io.o build/

build/jobCommander.o: src/jobCommander.cpp
	g++ -c -g src/jobCommander.cpp
	mv jobCommander.o build/

jobCommander: build/jobCommander.o build/my_io.o
	g++ -g -o bin/jobCommander build/jobCommander.o build/my_io.o


## jobExecutorServer:
build/jobExecutorServer.o: src/jobExecutorServer.cpp
	g++ -c -g src/jobExecutorServer.cpp
	mv jobExecutorServer.o build/

build/aux_s.o: src/aux_s.cpp
	g++ -c -g src/aux_s.cpp
	mv aux_s.o build/

build/my_queue.o: src/my_queue.cpp
	g++ -c -g src/my_queue.cpp
	mv my_queue.o build/

jobExecutorServer: build/jobExecutorServer.o build/my_io.o build/aux_s.o build/my_queue.o
	g++ -g -o bin/jobExecutorServer build/jobExecutorServer.o build/my_io.o build/aux_s.o build/my_queue.o -lpthread


## progDelay:
build/progDelay.o: tests/progDelay.cpp
	g++ -g -c tests/progDelay.cpp
	mv progDelay.o build/

progDelay: build/progDelay.o
	g++ -g -o bin/progDelay build/progDelay.o


clean:
	rm -f build/*.o
	rm -f bin/*
	rm -f *.output


#	./bin/jobExecutorServer 2034 8 5
#	./bin/jobCommander linux04.di.uoa.gr 2034 setConcurrency 2
