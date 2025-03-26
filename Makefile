CC = g++
CFLAGS = -Wall -Wextra -g -std=c++17

all: forwarding non_forwarding forward noforward

forward: forward.o
	$(CC) $(CFLAGS) -o forward forward.o

forward.o: src/forward.cpp src/risc_v_assembler.cpp src/risc_v_processor.cpp
	$(CC) $(CFLAGS) -c src/forward.cpp

noforward: noforward.o
	$(CC) $(CFLAGS) -o noforward noforward.o

noforward.o: src/noforward.cpp src/risc_v_assembler.cpp src/risc_v_processor.cpp
	$(CC) $(CFLAGS) -c src/noforward.cpp

forwarding: forwarding.o
	$(CC) $(CFLAGS) -o assembly-forwarding forwarding.o

forwarding.o: src/forwarding.cpp src/risc_v_assembler.cpp src/risc_v_processor.cpp
	$(CC) $(CFLAGS) -c src/forwarding.cpp

non_forwarding: non_forwarding.o
	$(CC) $(CFLAGS) -o assembly-non_forwarding non_forwarding.o

non_forwarding.o: src/non_forwarding.cpp src/risc_v_assembler.cpp src/risc_v_processor.cpp
	$(CC) $(CFLAGS) -c src/non_forwarding.cpp

clean:
	rm -f forwarding non_forwarding forwarding.o non_forwarding.o