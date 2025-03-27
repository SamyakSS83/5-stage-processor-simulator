#include<bits/stdc++.h>
#include<cstdint>
#include "risc_v_processor.cpp"
#include "cycle_stages.hpp"
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


int main(int argc, char *argv[]){
if (argc != 3) {
    cout << "Usage: " << argv[0] << " <filename>" << "count" << endl;
    return 0;
}
RV32I_5Stage cpu(0x10000, true);

string file_name = argv[1];
int cycles = stoi(argv[2]);
ifstream file(file_name);

if(!file.is_open()){
    cout << "File not found" << endl;
    return 0;
}

vector<uint32_t> instructions;
string line;

while (getline(file, line)) {
    // Skip empty lines
    if (line.empty()) continue;
    
    // Convert hex string to uint32_t
    uint32_t instruction = stoul(line, nullptr, 16);
    instructions.push_back(instruction);
}

file.close();

cpu.load_program(instructions);
cpu.run(cycles);    
cpu.dump_state();


return 0;

}