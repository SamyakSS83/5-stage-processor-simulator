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

void read_input_file(string file_name, vector<uint32_t>& machine_code, vector<string>& assembly_code){
    ifstream file;
    file.open(file_name);
    char c;

    if (!file.is_open()) {
        std::cerr << "Failed to open the file.\n";// Exit the program with an error code
        exit(1);
    }

    //extract machine code and assembly code from the file
    string machine_code_str;
    string assembly_code_str;
    string line;
    while (getline(file, line)) {
        if (line.empty()) {
            break;
        }
        istringstream input(line);
        
        input >> machine_code_str;
        machine_code.push_back(stoul(machine_code_str, nullptr, 16));
        getline(input, assembly_code_str);
        do {
            assembly_code_str = assembly_code_str.substr(1);
            c = assembly_code_str[0];
        } while (!(c >= 'a' && c <= 'z') && !(c >= 'A' && c <= 'Z'));

        assembly_code.push_back(assembly_code_str);
        cout << assembly_code_str << endl;
    }
    file.close();
}

void write_output_file(string file_name, vector<map<int, string>>& cycle_stages, vector<string>& assembly_code){
    ofstream file1;
    file1.open(file_name);
    if (!file1.is_open()) {
        cerr << "Failed to open the file.\n";// Exit the program with an error code
        exit(1);
    }

    // clean_map(cycle_stages);
    // for (int i = 0; i < cycle_stages.size(); i++){
    //     file1 << setw(8) << "Cycle " << i ;
    // }
    // file1 << endl;

    // Display cycle stages
    for (int i = 0; i < cycle_stages.size(); i++){
        file1 << assembly_code[i] << ";";
        for (auto const& x : cycle_stages[i])
        {
            file1 << x.second << ";";
        }
        file1 << endl;
    }
    file1.close();
    
}

int main() {
    // Initialize processor
    RV32I_5Stage cpu(0x10000, false);
    vector<uint32_t> machine_code;
    vector<string> assembly_code;

    // Read input file
    read_input_file("input.txt", machine_code, assembly_code);
    
    cpu.load_program(machine_code);
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

    // Output file
    write_output_file("output1.txt", cycle_stages, assembly_code);
    return 0;

}