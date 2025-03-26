#include<bits/stdc++.h>
#include<cstdint>
#include "risc_v_processor.cpp"
using namespace std;

int main(int argc, char *argv[]){
if (argc != 3) {
    cout << "Usage: " << argv[0] << " <filename>" << "count" << endl;
    return 0;
}
RV32I_5Stage cpu(0x10000, false);

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