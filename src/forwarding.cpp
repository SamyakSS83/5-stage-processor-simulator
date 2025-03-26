#include "risc_v_processor.cpp"
#include "risc_v_assembler.cpp"
using namespace std;

/* RISC-V Assembler */

int main() {
    // Initialize processor
    RV32I_5Stage cpu(0x10000, true);
    RV32Assembler assembler;
    
    // Ask user for program input
    cout << "RISC-V 5-stage Pipeline Simulator" << endl;
    cout << "==================================" << endl;
    cout << "Enter RISC-V assembly code, one instruction per line." << endl;
    cout << "Enter an empty line to finish input." << endl;
    
    vector<string> assembly_lines;
    string line;
    
    cout << "\nEnter program:" << endl;
    while (true) {
        cout << assembly_lines.size() + 1 << "> ";
        getline(cin, line);
        
        // Remove leading and trailing whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);
        
        if (line.empty()) {
            break;
        }
        
        assembly_lines.push_back(line);
    }
    
    if (assembly_lines.empty()) {
        cout << "No instructions entered. Using default program." << endl;
        assembly_lines = {
            "addi x1, x0, 1",      // x1 = 1 (i)
            "addi x2, x0, 0",      // x2 = 0 (sum)
            "addi x3, x0, 10",     // x3 = 10 (limit)
            "add  x4, x1, x2",     // x4 = i + sum
            "addi x1, x1, 1",      // i++
            "add  x2, x0, x4",     // sum = x4
            "blt  x1, x3, -12"     // if (i < limit) goto loop
        };
    }
    
    // Assemble the program
    vector<uint32_t> machine_code = assembler.assembleProgram(assembly_lines);
    
    // Load program into processor memory
    cpu.load_program(machine_code);
        
    // Ask for number of cycles to run
    int cycles;
    cout << "\nEnter number of cycles to run: ";
    cin >> cycles;
    
    // Run the simulation
    cpu.run(cycles);
    
    // Display final state
    cpu.dump_state();
    
    return 0;
}

