#include<iostream>
#include<vector>
#include<map>
#include<bitset>
#include<utility>
using namespace std;

typedef struct reg {
    int value;
} reg;

typedef struct instruction_memory {
    vector<bitset<32>> data;
} instruction_memory;

//TODO - make pipeline registers for storing intermediate values
class PipelineProcessor {
public:
    PipelineProcessor() {
        // make classes for each component
        // initialize them
        // TODO
    }
};

class InstructionFetch { // update PC increment logic
public:

    bitset<32> instruction; //output instruction

    void fetch_instruction(reg& PC, instruction_memory& instruction_memory) {
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

class RegisterFile { //improve control further by introducing a control signal for write enable
private:
    vector<reg> gp_registers;
    //input ports

    RegisterFile() { // initialising with all zero values
        for (int i = 0; i<32; i++) {
            reg r;
            r.value = 0;
            gp_registers.push_back(r);
        }
    }

    int _read_reg(int index){
        return gp_registers.at(index).value;
    }

    
public: 
    pair<int, int> read_register(int index1, int index2) {
        pair<int, int> result;
        result.first = _read_reg(index1);
        result.second = _read_reg(index2);
        return result; // return both read register value
    } 

    void write_register(int index, int value) {
        gp_registers.at(index).value = value;
    }

};

class InstructionDecode { //TODO
    // OPCODE will be provided in the processor class
    //TODO - input is a 32 bit word as instruction
    //TODO - outputs will be different depending on instruction type
    //use opcodes to figure out instruction type and then decode the register values, etc required
    //pass on register numbers as outputs and in case of non arithmetic operations,  pass on the immediate field
};

class Execute {
    //TODO - inputs will be two values 
    // output will be the result of alu operation and a "zero" signal
};

class MemoryOp {
    //TODO - there will be a data memory interface in the processor
    //TODO - inputs will be the address and the value to be written
    // output should be the value read and in case of a write type instruction,
    // it should pass on the to_be_written value back to register fle
};

class WriteBack {
    //TODO - use the data from  MemoryOp to write back to the register file(if required to write back)
};
