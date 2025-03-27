#include<bits/stdc++.h>
#include<cstdint>
using namespace std;

#define DEBUG_MODE 0

#if DEBUG_MODE
#define DEBUG_PRINT(msg) cout << "[DEBUG] " << msg << endl
#else
#define DEBUG_PRINT(msg)
#endif

class RV32Assembler {
    public:
        RV32Assembler() {
            // Initialize opcode map
            opcodeMap["add"] = {0x33, 0, 0x0};
            opcodeMap["sub"] = {0x33, 0, 0x20};
            opcodeMap["sll"] = {0x33, 1, 0x0};
            opcodeMap["slt"] = {0x33, 2, 0x0};
            opcodeMap["sltu"] = {0x33, 3, 0x0};
            opcodeMap["xor"] = {0x33, 4, 0x0};
            opcodeMap["srl"] = {0x33, 5, 0x0};
            opcodeMap["sra"] = {0x33, 5, 0x20};
            opcodeMap["or"] = {0x33, 6, 0x0};
            opcodeMap["and"] = {0x33, 7, 0x0};
            
            opcodeMap["addi"] = {0x13, 0, 0x0}; opcodeMap["mv"] = {0x13, 0, 0x0};
            opcodeMap["slti"] = {0x13, 2, 0x0};
            opcodeMap["sltiu"] = {0x13, 3, 0x0};
            opcodeMap["xori"] = {0x13, 4, 0x0};
            opcodeMap["ori"] = {0x13, 6, 0x0};
            opcodeMap["andi"] = {0x13, 7, 0x0};
            opcodeMap["slli"] = {0x13, 1, 0x0};
            opcodeMap["srli"] = {0x13, 5, 0x0};
            opcodeMap["srai"] = {0x13, 5, 0x20};
            
            opcodeMap["lb"] = {0x03, 0, 0x0};
            opcodeMap["lh"] = {0x03, 1, 0x0};
            opcodeMap["lw"] = {0x03, 2, 0x0};
            opcodeMap["lbu"] = {0x03, 4, 0x0};
            opcodeMap["lhu"] = {0x03, 5, 0x0};
            
            opcodeMap["sb"] = {0x23, 0, 0x0};
            opcodeMap["sh"] = {0x23, 1, 0x0};
            opcodeMap["sw"] = {0x23, 2, 0x0};
            
            opcodeMap["beq"] = {0x63, 0, 0x0};
            opcodeMap["bne"] = {0x63, 1, 0x0};
            opcodeMap["blt"] = {0x63, 4, 0x0};
            opcodeMap["bge"] = {0x63, 5, 0x0};
            opcodeMap["bltu"] = {0x63, 6, 0x0};
            opcodeMap["bgeu"] = {0x63, 7, 0x0};
            
            opcodeMap["jal"] = {0x6f, 0, 0x0};
            opcodeMap["jalr"] = {0x67, 0, 0x0};
            
            opcodeMap["lui"] = {0x37, 0, 0x0};
            opcodeMap["auipc"] = {0x17, 0, 0x0};
            
            // Initialize register map
            regMap["x0"] = 0; regMap["zero"] = 0;
            regMap["x1"] = 1; regMap["ra"] = 1;
            regMap["x2"] = 2; regMap["sp"] = 2;
            regMap["x3"] = 3; regMap["gp"] = 3;
            regMap["x4"] = 4; regMap["tp"] = 4;
            regMap["x5"] = 5; regMap["t0"] = 5;
            regMap["x6"] = 6; regMap["t1"] = 6;
            regMap["x7"] = 7; regMap["t2"] = 7;
            regMap["x8"] = 8; regMap["s0"] = 8; regMap["fp"] = 8;
            regMap["x9"] = 9; regMap["s1"] = 9;
            regMap["x10"] = 10; regMap["a0"] = 10;
            regMap["x11"] = 11; regMap["a1"] = 11;
            regMap["x12"] = 12; regMap["a2"] = 12;
            regMap["x13"] = 13; regMap["a3"] = 13;
            regMap["x14"] = 14; regMap["a4"] = 14;
            regMap["x15"] = 15; regMap["a5"] = 15;
            regMap["x16"] = 16; regMap["a6"] = 16;
            regMap["x17"] = 17; regMap["a7"] = 17;
            regMap["x18"] = 18; regMap["s2"] = 18;
            regMap["x19"] = 19; regMap["s3"] = 19;
            regMap["x20"] = 20; regMap["s4"] = 20;
            regMap["x21"] = 21; regMap["s5"] = 21;
            regMap["x22"] = 22; regMap["s6"] = 22;
            regMap["x23"] = 23; regMap["s7"] = 23;
            regMap["x24"] = 24; regMap["s8"] = 24;
            regMap["x25"] = 25; regMap["s9"] = 25;
            regMap["x26"] = 26; regMap["s10"] = 26;
            regMap["x27"] = 27; regMap["s11"] = 27;
            regMap["x28"] = 28; regMap["t3"] = 28;
            regMap["x29"] = 29; regMap["t4"] = 29;
            regMap["x30"] = 30; regMap["t5"] = 30;
            regMap["x31"] = 31; regMap["t6"] = 31;
        }
        
        // Parse a register name to register number
        int parseRegister(const string& reg) {
            // Remove any commas
            string cleanReg = reg;
            size_t commaPos = cleanReg.find(',');
            if (commaPos != string::npos) {
                cleanReg = cleanReg.substr(0, commaPos);
            }
            
            if (regMap.find(cleanReg) != regMap.end()) {
                return regMap[cleanReg];
            } else {
                cerr << "Unknown register: " << cleanReg << endl;
                return 0;
            }
        }
        
        // Parse an immediate value
        int parseImmediate(const string& imm) {
            // Remove any commas and parentheses
            string cleanImm = imm;
            size_t commaPos = cleanImm.find(',');
            if (commaPos != string::npos) {
                cleanImm = cleanImm.substr(0, commaPos);
            }
            
            try {
                // Check if hex
                if (cleanImm.substr(0, 2) == "0x") {
                    return stoi(cleanImm, nullptr, 16);
                }
                // Otherwise decimal
                return stoi(cleanImm);
            } catch (exception& e) {
                cerr << "Invalid immediate value: " << cleanImm << endl;
                return 0;
            }
        }
        
        // Parse memory operand like 8(x1)
        pair<int, int> parseMemOperand(const string& operand) {
            size_t openParen = operand.find('(');
            size_t closeParen = operand.find(')');
            
            if (openParen == string::npos || closeParen == string::npos) {
                cerr << "Invalid memory operand: " << operand << endl;
                return {0, 0};
            }
            
            string offsetStr = operand.substr(0, openParen);
            string regStr = operand.substr(openParen + 1, closeParen - openParen - 1);
            
            int offset = parseImmediate(offsetStr);
            int reg = parseRegister(regStr);
            
            return {offset, reg};
        }
        
        // Assemble an instruction
        uint32_t assemble(const string& line) {
            DEBUG_PRINT("Assembling: " << line);
            
            istringstream iss(line);
            string opcode, op1, op2, op3;
            
            iss >> opcode;
            cout << "received opcode: " << opcode << endl;
            
            // Convert to lowercase
            for (auto& c : opcode) c = tolower(c);
            
            // If opcode not found
            if (opcodeMap.find(opcode) == opcodeMap.end()) {
                cerr << "Unknown opcode: " << opcode << endl;
                return 0;
            }
            
            auto& opcodeInfo = opcodeMap[opcode];
    
            uint32_t baseOpcode = opcodeInfo.first;
            uint32_t funct3 = opcodeInfo.second;
            uint32_t funct7 = opcodeInfo.third;
            
            uint32_t instruction = baseOpcode; // Set opcode bits
            if (opcode == "mv") { 
                opcode = "addi"; //mv is just an alias of addi with imm value 0
                op3 = "0";
            }
            
            // Parse operands based on instruction type
            if (opcode == "lui" || opcode == "auipc") {
                // U-type: lui rd, imm
                iss >> op1 >> op2;
                int rd = parseRegister(op1);
                int imm = parseImmediate(op2);
                
                instruction |= (rd << 7);
                instruction |= (imm & 0xFFFFF000);
            }
            else if (opcode == "jal") {
                // J-type: jal rd, offset
                iss >> op1 >> op2;
                int rd = parseRegister(op1);
                int imm = parseImmediate(op2);
                
                instruction |= (rd << 7);
                // Handle J-type immediate encoding
                uint32_t j_imm = 0;
                j_imm |= ((imm >> 20) & 0x1) << 31;    // imm[20]
                j_imm |= ((imm >> 1) & 0x3FF) << 21;   // imm[10:1]
                j_imm |= ((imm >> 11) & 0x1) << 20;    // imm[11]
                j_imm |= ((imm >> 12) & 0xFF) << 12;   // imm[19:12]
                instruction |= j_imm;
            }
            else if (opcode == "jalr") {
                // I-type: jalr rd, offset(rs1)
                iss >> op1 >> op2;
                int rd = parseRegister(op1);
                auto [offset, rs1] = parseMemOperand(op2);
                
                instruction |= (rd << 7);
                instruction |= (funct3 << 12);
                instruction |= (rs1 << 15);
                instruction |= ((offset & 0xFFF) << 20);
            }
            else if (opcode.substr(0, 1) == "b") {
                // B-type: beq rs1, rs2, offset
                iss >> op1 >> op2 >> op3;
                int rs1 = parseRegister(op1);
                int rs2 = parseRegister(op2);
                int imm = parseImmediate(op3);
                
                instruction |= (funct3 << 12);
                instruction |= (rs1 << 15);
                instruction |= (rs2 << 20);
                
                // Handle B-type immediate encoding
                uint32_t b_imm = 0;
                b_imm |= ((imm >> 12) & 0x1) << 31;    // imm[12]
                b_imm |= ((imm >> 5) & 0x3F) << 25;    // imm[10:5]
                b_imm |= ((imm >> 1) & 0xF) << 8;      // imm[4:1]
                b_imm |= ((imm >> 11) & 0x1) << 7;     // imm[11]
                instruction |= b_imm;
            }
            else if (opcode.substr(0, 1) == "s") {
                if (opcode == "slli" || opcode == "srli" || opcode == "srai") {
                    // I-type shift: slli rd, rs1, shamt
                    iss >> op1 >> op2 >> op3;
                    int rd = parseRegister(op1);
                    int rs1 = parseRegister(op2);
                    int shamt = parseImmediate(op3);
                    
                    instruction |= (rd << 7);
                    instruction |= (funct3 << 12);
                    instruction |= (rs1 << 15);
                    instruction |= ((shamt & 0x1F) << 20);
                    instruction |= (funct7 << 25);
                }
                else if (opcode == "sb" || opcode == "sh" || opcode == "sw") {
                    // S-type: sw rs2, offset(rs1)
                    iss >> op1 >> op2;
                    int rs2 = parseRegister(op1);
                    auto [offset, rs1] = parseMemOperand(op2);
                    
                    instruction |= (funct3 << 12);
                    instruction |= (rs1 << 15);
                    instruction |= (rs2 << 20);
                    
                    // Handle S-type immediate encoding
                    instruction |= ((offset & 0x1F) << 7);        // imm[4:0]
                    instruction |= (((offset >> 5) & 0x7F) << 25); // imm[11:5]
                } 
                else {
                    // R-type: sub rd, rs1, rs2
                    iss >> op1 >> op2 >> op3;
                    int rd = parseRegister(op1);
                    int rs1 = parseRegister(op2);
                    int rs2 = parseRegister(op3);
                    
                    instruction |= (rd << 7);
                    instruction |= (funct3 << 12);
                    instruction |= (rs1 << 15);
                    instruction |= (rs2 << 20);
                    instruction |= (funct7 << 25);
                }
            }
            else if (opcode.substr(0, 1) == "l") {
                // I-type load: lw rd, offset(rs1)
                iss >> op1 >> op2;
                int rd = parseRegister(op1);
                auto [offset, rs1] = parseMemOperand(op2);
                
                instruction |= (rd << 7);
                instruction |= (funct3 << 12);
                instruction |= (rs1 << 15);
                instruction |= ((offset & 0xFFF) << 20);
            }
            else {
                // R-type or I-type arithmetic
                iss >> op1 >> op2 >> op3;
                int rd = parseRegister(op1);
                int rs1 = parseRegister(op2);
                
                instruction |= (rd << 7);
                instruction |= (funct3 << 12);
                instruction |= (rs1 << 15);
                
                if (opcode == "add" || opcode == "sub" || opcode == "sll" || opcode == "slt" || 
                    opcode == "sltu" || opcode == "xor" || opcode == "srl" || opcode == "sra" || 
                    opcode == "or" || opcode == "and") {
                    // R-type
                    int rs2 = parseRegister(op3);
                    instruction |= (rs2 << 20);
                    instruction |= (funct7 << 25);
                } else {
                    // I-type
                    int imm = parseImmediate(op3);
                    instruction |= ((imm & 0xFFF) << 20);
                }
            }
            
            DEBUG_PRINT("Assembled instruction: 0x" << hex << instruction << dec);
            return instruction;
        }
        
        // Assemble a program (multiple lines)
        vector<uint32_t> assembleProgram(const vector<string>& program) {
            vector<uint32_t> machineCode;
            
            for (const auto& line : program) {
                // Skip empty lines and comments
                if (line.empty() || line[0] == '#' || line[0] == '/') {
                    continue;
                }
                
                uint32_t instruction = assemble(line);
                if (instruction != 0) {
                    machineCode.push_back(instruction);
                }
            }
            
            return machineCode;
        }
        
    private:
        struct OpcodeInfo {
            uint32_t first;   // opcode
            uint32_t second;  // funct3
            uint32_t third;   // funct7
        };
        map<string, OpcodeInfo> opcodeMap;
        map<string, int> regMap;
    };