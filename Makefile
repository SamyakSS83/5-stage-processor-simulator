CC = g++
CFLAGS = -Wall -Wextra -g 

all: forwarding non_forwarding

forwarding: forwarding.o
	$(CC) $(CFLAGS) -o forwarding forwarding.o

forwarding.o: src/forwarding.cpp src/risc_v_assembler.cpp src/risc_v_processor.cpp
	$(CC) $(CFLAGS) -c src/forwarding.cpp

non_forwarding: non_forwarding.o
	$(CC) $(CFLAGS) -o non_forwarding non_forwarding.o

non_forwarding.o: src/non_forwarding.cpp src/risc_v_assembler.cpp src/risc_v_processor.cpp
	$(CC) $(CFLAGS) -c src/non_forwarding.cpp

clean:
	rm -f forwarding non_forwarding forwarding.o non_forwarding.o