#include "risc_v_processor.cpp"
#include "risc_v_assembler.cpp"
#include"cycle_stages.h"
#include<fstream>
using namespace std;

/* RISC-V Assembler */
vector <map<int, string>> cycle_stages;


void clean_map(vector <map<int, string>>& cycle_stages){
    // remove leading and trailing "-" from the map :
    for (int i = 0; i < cycle_stages.size(); i++){

        for (auto it = cycle_stages[i].begin(); it != cycle_stages[i].end(); it++){
            auto next =it;
            next++;
            if (next != cycle_stages[i].end() && (it->second == next->second) && (it->second != "-")){
                it->second = "-";
            }
        }

        for (auto it = cycle_stages[i].begin(); it != cycle_stages[i].end(); it++){
            if (it->second == "-"){
                it->second = " ";
            }
            else break;
        }

        for (auto it = cycle_stages[i].rbegin(); it != cycle_stages[i].rend(); it++){
            if (it->second == "-"){
                it->second = " ";
            }
            else break;
        }
    }
}

int main() {
    // Initialize processor
    RV32I_5Stage cpu(0x10000, false);
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

    for (int i; i<machine_code.size(); i++){
        map<int, string> row_i;
        for (int j = 0; j < cycles; j++) {
            row_i[j] = "-";
        }
        cycle_stages.push_back(row_i); //ith row stats
    }
    
    // Run the simulation
    cpu.run(cycles);
    
    // Display final state
    cpu.dump_state();

    ofstream file;
    file.open("output_non_forwarding.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open the file.\n";// Exit the program with an error code
    }

    clean_map(cycle_stages);
     
    // Display cycle stages
    for (int i=0; i< machine_code.size(); i++){
        cout <<  assembly_lines[i] << ";";
        file <<  assembly_lines[i] << ";";
        for (const auto& pair : cycle_stages[i]) {
            cout << pair.second << ";";
            file << pair.second << ";";
        }
        file << "\n";
        cout << endl;
    }
    file.close();
    return 0;
}

