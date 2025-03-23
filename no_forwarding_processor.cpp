#include<iostream>
#include<vector>
#include<map>
#include<bitset>
using namespace std;

typedef struct reg {
    int value;
} reg;

typedef struct memory {
    vector<bitset<32>> data;
} memory;

class PipelineProcessor {
public:
    PipelineProcessor() {
        // make classes for each component
        // initialize them
        // TODO
    }
};

class InstructionFetch {
public:

    bitset<32> instruction; //output instruction

    void fetch_instruction(reg& PC, memory& instruction_memory) {
        // fetch instruction from memory
        // increment PC
        int address = PC.value;
        if (address < 0 || address > instruction_memory.data.size()) {
            cout << "address out of bounds of instruction memory size: " << address << endl;
            cout << "Exiting..." << endl;
            exit(1);
        }
        instruction = instruction_memory.data[address]; //new instruction based on PC value
        PC.value += 4; // for now increment by 4, later add based on branch change 
    }
};



