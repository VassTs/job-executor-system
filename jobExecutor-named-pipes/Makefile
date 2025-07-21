# compile with gcc
CC = g++

# Set compiler options here.
#
# -pg is used for profiling (gprof)
CXXFLAGS = -g -std=c++11					# -Wall -Wextra -Wpedantic

# set the name of the executable file to compile here
PROGRAM_1 = bin/jobExecutorServer
PROGRAM_2 = bin/jobCommander

OBJS_1 = build/jobExecutorServer.o build/aux.o   # src/object.o lib/object.o
OBJS_2 = build/jobCommander.o build/my_io.o build/aux_c.o

$(PROGRAM_1): $(OBJS_1)
	$(CC) $(OBJS_1) -o $(PROGRAM_1)

$(PROGRAM_2): $(OBJS_2)
	$(CC) $(OBJS_2) -o $(PROGRAM_2)

clean:
	rm -f $(PROGRAM_1) $(OBJS_1)
	rm -f $(PROGRAM_2) $(OBJS_2)
	rm *_w
	rm *_r
	rm server.pipe

run: $(PROGRAM_1)
	./$(PROGRAM_1)

valgrind: $(PROGRAM_1)
	# valgrind --leak-check=full --track-origins=yes ./$(PROGRAM_1)
	valgrind --leak-check=full --show-leak-kinds=all ./$(PROGRAM_1)

quick:
	clear
	rm -f $(PROGRAM_1) $(OBJS_1)
	rm -f $(PROGRAM_2) $(OBJS_2)
	rm -f *_w
	rm -f *_r
	rm -f server.pipe
	rm -f jobExecutorServer.txt

	mkdir -p build bin
	g++ -c scripts/progDelay.cpp -o build/progDelay.o
	g++ build/progDelay.o -o bin/progDelay


	g++ -c src/my_io.cpp -o build/my_io.o
	g++ -c src/aux_c.cpp -o build/aux_c.o
	g++ -c src/aux.cpp -o build/aux.o
	g++ -c src/aux_2.cpp -o build/aux_2.o
	g++ -c src/my_queue.cpp -o build/my_queue.o
	g++ -c src/jobExecutorServer.cpp -o build/jobExecutorServer.o
	g++ -c src/jobCommander.cpp -o build/jobCommander.o

	g++ -o bin/jobExecutorServer build/jobExecutorServer.o build/aux.o build/my_io.o build/my_queue.o build/aux_2.o
	g++ -o bin/jobCommander build/jobCommander.o build/my_io.o build/aux_c.o
	# valgrind ./$(PROGRAM_1)

fast:
	clear
	./$(PROGRAM_2)