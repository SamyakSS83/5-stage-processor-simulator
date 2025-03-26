CC = g++
CFLAGS = -Wall

all: forwarding non_forwarding

forwarding: forwarding.o
	$(CC) $(CFLAGS) -o src/forwarding forwarding.o

forwarding.o: src/forwarding.cpp src/risc_v_assembler.hpp src/risc_v_processor.hpp
	$(CC) $(CFLAGS) -c src/forwarding.cpp

non_forwarding: non_forwarding.o
	$(CC) $(CFLAGS) -o non_forwarding non_forwarding.o

non_forwarding.o: src/non_forwarding.cpp src/risc_v_assembler.hpp src/risc_v_processor.hpp
	$(CC) $(CFLAGS) -c src/non_forwarding.cpp

clean:
	rm -f forwarding non_forwarding forwarding.o non_forwarding.o