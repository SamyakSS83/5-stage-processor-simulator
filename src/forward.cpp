#include<bits/stdc++.h>
#include "risc_v_processor.cpp"
#include "risc_v_assembler.cpp"
#include"cycle_stages.hpp"
#include<fstream>
using namespace std;

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


void read_input_file(string file_name, vector<uint32_t>& machine_code, vector<string>& assembly_code) {
    ifstream file;
    file.open(file_name);
    char c;

    if (!file.is_open()) {
        std::cerr << "Failed to open the file.\n";
        exit(1);
    }

    string machine_code_str;
    string assembly_code_str;
    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            continue; // Skip empty lines
        }

        istringstream input(line);
        input >> machine_code_str;

        // Check if the first token is a valid hexadecimal machine code
        try {
            machine_code.push_back(stoul(machine_code_str, nullptr, 16));
        } catch (const std::invalid_argument& e) {
            // If not, treat it as part of the assembly instruction
            if (!assembly_code.empty()) {
                assembly_code.back() += " " + line;
            }
            continue;
        }

        // Extract the rest of the line as the assembly instruction
        getline(input, assembly_code_str);
        assembly_code.push_back(assembly_code_str);
    }

    file.close();
}


void print_stats(vector<map<int, string>>& cycle_stages){
    for (int i = 0; i < cycle_stages.size(); i++){
        cout << "Instruction " << i << " : ";
        for (auto const& x : cycle_stages[i])
        {
            cout << x.first << " : " << x.second << ";";
        }
        cout << endl;
    }
}


int main(int argc, char* argv[]) {

    if (argc < 3) {
        cout << "Usage: " << argv[0] << " <input_file> " << "<cycle_count>" << endl;
        return 1;
    }

    string input_file = argv[1];
    int cycles = atoi(argv[2]);

    // Initialize processor
    RV32I_5Stage cpu(0x10000, true);
    vector<uint32_t> machine_code;
    vector<string> assembly_code;

    // Read input file
    read_input_file(input_file, machine_code, assembly_code);
    //print the machine code and assembly code
    for (const auto& code : machine_code) {
        cout << hex << code << " ";
    }
    cout << endl;
    for (const auto& code : assembly_code) {
        cout << code << endl;
    }
    // Load the program into the CPU


    cpu.load_program(machine_code);
    // int cycles;
    // cout << "\nEnter number of cycles to run: ";
    // cin >> cycles;

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

    // Clean the map
    // clean_map(cycle_stages);
    print_stats(cycle_stages);

    // Output file
    // write_output_file("output1.txt", cycle_stages, assembly_code);
    return 0;

}