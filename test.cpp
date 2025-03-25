#include<iostream>
#include<vector>
#include<map>
#include<bitset>
#include<string>
#include<sstream>
#include<utility>
#include<iomanip>
using namespace std;

// Debug macro - set to 1 to enable debug prints
#define DEBUG_MODE 1

#if DEBUG_MODE
#define DEBUG_PRINT(msg) cout << "[DEBUG] " << msg << endl
#else
#define DEBUG_PRINT(msg)
#endif

// Simple RISC-V Assembler
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
        
        opcodeMap["addi"] = {0x13, 0, 0x0};
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
                // R-type: add rd, rs1, rs2
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

class RV32I_5Stage {
public:
    // Constants for RISC-V instruction formats and opcodes
    static constexpr uint32_t OPCODE_LOAD      = 0b0000011;
    static constexpr uint32_t OPCODE_STORE     = 0b0100011;
    static constexpr uint32_t OPCODE_BRANCH    = 0b1100011;
    static constexpr uint32_t OPCODE_JAL       = 0b1101111;
    static constexpr uint32_t OPCODE_JALR      = 0b1100111;
    static constexpr uint32_t OPCODE_OP_IMM    = 0b0010011;
    static constexpr uint32_t OPCODE_OP        = 0b0110011;
    static constexpr uint32_t OPCODE_SYSTEM    = 0b1110011;
    static constexpr uint32_t OPCODE_AUIPC     = 0b0010111;
    static constexpr uint32_t OPCODE_LUI       = 0b0110111;

    // ALU operations
    enum class ALUOp {
        ADD, SUB, SLL, SLT, SLTU, XOR, SRL, SRA, OR, AND, NONE
    };

    // Branch comparison types
    enum class BranchCond {
        EQ, NE, LT, GE, LTU, GEU, TRUE, FALSE
    };

    // Memory operations
    enum class MemOp {
        LB, LH, LW, LBU, LHU, SB, SH, SW, NONE
    };

    // Instruction types
    enum class InstrType {
        R, I, S, B, U, J, UNKNOWN
    };

    // Pipeline registers
    struct IF_ID_Reg {
        uint32_t pc;
        uint32_t instruction;
        bool valid;
    };

    struct ID_EX_Reg {
        uint32_t pc;
        uint32_t rs1_val;
        uint32_t rs2_val;
        uint32_t imm;
        uint32_t rd;
        uint32_t rs1;
        uint32_t rs2;
        ALUOp alu_op;
        BranchCond branch_cond;
        MemOp mem_op;
        bool reg_write;
        bool mem_read;
        bool mem_write;
        bool alu_src;
        bool use_pc;
        bool valid;
    };

    struct EX_MEM_Reg {
        uint32_t pc;
        uint32_t alu_result;
        uint32_t rs2_val;
        uint32_t rd;
        MemOp mem_op;
        bool reg_write;
        bool mem_read;
        bool mem_write;
        bool branch_taken;
        uint32_t branch_target;
        bool valid;
    };

    struct MEM_WB_Reg {
        uint32_t alu_result;
        uint32_t mem_data;
        uint32_t rd;
        bool reg_write;
        bool mem_to_reg;
        bool valid;
    };

    // Constructor
    RV32I_5Stage(uint32_t mem_size = 0x10000) : 
        instruction_memory(mem_size, 0),
        data_memory(mem_size, 0),
        pc(0),
        cycle_count(0) {
        
        // Initialize registers to 0
        for (int i = 0; i < 32; i++) {
            registers[i] = 0;
        }
        
        // x0 is hardwired to 0
        registers[0] = 0;
        
        reset_pipeline_regs();
        DEBUG_PRINT("Processor initialized with " << mem_size << " bytes of memory");
    }

    // Reset pipeline registers
    void reset_pipeline_regs() {
        DEBUG_PRINT("Resetting pipeline registers");
        if_id = IF_ID_Reg{0, 0, false};
        id_ex = ID_EX_Reg{0, 0, 0, 0, 0, 0, 0, ALUOp::NONE, BranchCond::FALSE, MemOp::NONE, false, false, false, false, false, false};
        ex_mem = EX_MEM_Reg{0, 0, 0, 0, MemOp::NONE, false, false, false, false, 0, false};
        mem_wb = MEM_WB_Reg{0, 0, 0, false, false, false};
    }

    // Load program into instruction memory
    void load_program(const vector<uint32_t>& program, uint32_t start_addr = 0) {
        DEBUG_PRINT("Loading program of " << program.size() << " instructions at address 0x" 
                     << hex << start_addr << dec);
        for (size_t i = 0; i < program.size(); i++) {
            instruction_memory[start_addr + i * 4] = program[i];
            DEBUG_PRINT("  Instruction " << i << " at 0x" << hex 
                         << (start_addr + i * 4) << ": 0x" << program[i] << dec);
        }
    }

    // Register file operations
    uint32_t read_register(uint32_t index) const {
        if (index == 0) return 0; // x0 is hardwired to 0
        if (index < 32) {
            DEBUG_PRINT("Reading register x" << index << " = 0x" << hex 
                         << registers[index] << dec);
            return registers[index];
        }
        DEBUG_PRINT("INVALID REGISTER READ: " << index);
        return 0;
    }

    void write_register(uint32_t index, uint32_t value) {
        if (index > 0 && index < 32) { // Can't write to x0
            DEBUG_PRINT("Writing register x" << index << " = 0x" << hex 
                         << value << dec);
            registers[index] = value;
        } else if (index == 0) {
            DEBUG_PRINT("Attempted write to x0 (ignored)");
        } else {
            DEBUG_PRINT("INVALID REGISTER WRITE: " << index);
        }
    }

    // Instruction decoder
    InstrType decode_instr_type(uint32_t instr) {
        uint32_t opcode = instr & 0x7F;
        
        DEBUG_PRINT("Decoding instruction type with opcode 0x" << hex 
                     << opcode << dec);
        
        switch (opcode) {
            case OPCODE_OP:
                DEBUG_PRINT("  Instruction type: R-type");
                return InstrType::R;
            case OPCODE_OP_IMM:
            case OPCODE_LOAD:
            case OPCODE_JALR:
                DEBUG_PRINT("  Instruction type: I-type");
                return InstrType::I;
            case OPCODE_STORE:
                DEBUG_PRINT("  Instruction type: S-type");
                return InstrType::S;
            case OPCODE_BRANCH:
                DEBUG_PRINT("  Instruction type: B-type");
                return InstrType::B;
            case OPCODE_LUI:
            case OPCODE_AUIPC:
                DEBUG_PRINT("  Instruction type: U-type");
                return InstrType::U;
            case OPCODE_JAL:
                DEBUG_PRINT("  Instruction type: J-type");
                return InstrType::J;
            default:
                DEBUG_PRINT("  Instruction type: UNKNOWN");
                return InstrType::UNKNOWN;
        }
    }

    // Immediate value extraction based on instruction type
    uint32_t extract_immediate(uint32_t instr, InstrType type) {
        uint32_t imm = 0;
        
        switch (type) {
            case InstrType::I: {
                // I-type: [31:20]
                imm = (instr >> 20) & 0xFFF;
                // Sign extend
                if (imm & 0x800) imm |= 0xFFFFF000;
                DEBUG_PRINT("  I-type immediate: 0x" << hex << imm << dec);
                break;
            }
                
            case InstrType::S: {
                // S-type: [31:25][11:7]
                imm = ((instr >> 25) & 0x7F) << 5;
                imm |= (instr >> 7) & 0x1F;
                // Sign extend
                if (imm & 0x800) imm |= 0xFFFFF000;
                DEBUG_PRINT("  S-type immediate: 0x" << hex << imm << dec);
                break;
            }
                
            case InstrType::B: {
                // B-type: [31][7][30:25][11:8]
                imm = ((instr >> 31) & 0x1) << 12;
                imm |= ((instr >> 7) & 0x1) << 11;
                imm |= ((instr >> 25) & 0x3F) << 5;
                imm |= ((instr >> 8) & 0xF) << 1;
                // Sign extend
                if (imm & 0x1000) imm |= 0xFFFFE000;
                DEBUG_PRINT("  B-type immediate: 0x" << hex << imm << dec);
                break;
            }
                
            case InstrType::U: {
                // U-type: [31:12]
                imm = instr & 0xFFFFF000;
                DEBUG_PRINT("  U-type immediate: 0x" << hex << imm << dec);
                break;
            }
                
            case InstrType::J: {
                // J-type: [31][19:12][20][30:21]
                imm = ((instr >> 31) & 0x1) << 20;
                imm |= ((instr >> 12) & 0xFF) << 12;
                imm |= ((instr >> 20) & 0x1) << 11;
                imm |= ((instr >> 21) & 0x3FF) << 1;
                // Sign extend
                if (imm & 0x100000) imm |= 0xFFF00000;
                DEBUG_PRINT("  J-type immediate: 0x" << hex << imm << dec);
                break;
            }
                
            default:
                DEBUG_PRINT("  Unknown immediate type");
                imm = 0;
        }
        
        return imm;
    }

    // Field extraction
    uint32_t extract_rd(uint32_t instr) {
        uint32_t rd = (instr >> 7) & 0x1F;
        DEBUG_PRINT("  rd = x" << rd);
        return rd;
    }

    uint32_t extract_rs1(uint32_t instr) {
        uint32_t rs1 = (instr >> 15) & 0x1F;
        DEBUG_PRINT("  rs1 = x" << rs1);
        return rs1;
    }

    uint32_t extract_rs2(uint32_t instr) {
        uint32_t rs2 = (instr >> 20) & 0x1F;
        DEBUG_PRINT("  rs2 = x" << rs2);
        return rs2;
    }

    uint32_t extract_funct3(uint32_t instr) {
        uint32_t funct3 = (instr >> 12) & 0x7;
        DEBUG_PRINT("  funct3 = 0x" << hex << funct3 << dec);
        return funct3;
    }

    uint32_t extract_funct7(uint32_t instr) {
        uint32_t funct7 = (instr >> 25) & 0x7F;
        DEBUG_PRINT("  funct7 = 0x" << hex << funct7 << dec);
        return funct7;
    }

    uint32_t extract_opcode(uint32_t instr) {
        uint32_t opcode = instr & 0x7F;
        DEBUG_PRINT("  opcode = 0x" << hex << opcode << dec);
        return opcode;
    }

    // ALU operation
    uint32_t execute_alu(uint32_t a, uint32_t b, ALUOp op) {
        uint32_t result = 0;
        
        DEBUG_PRINT("ALU: a=0x" << hex << a << ", b=0x" << b << dec);
        
        switch (op) {
            case ALUOp::ADD:  
                result = a + b; 
                DEBUG_PRINT("  ADD: result=0x" << hex << result << dec);
                break;
            case ALUOp::SUB:  
                result = a - b; 
                DEBUG_PRINT("  SUB: result=0x" << hex << result << dec);
                break;
            case ALUOp::SLL:  
                result = a << (b & 0x1F); 
                DEBUG_PRINT("  SLL: result=0x" << hex << result << dec);
                break;
            case ALUOp::SLT:  
                result = static_cast<int32_t>(a) < static_cast<int32_t>(b) ? 1 : 0; 
                DEBUG_PRINT("  SLT: result=" << result);
                break;
            case ALUOp::SLTU: 
                result = a < b ? 1 : 0; 
                DEBUG_PRINT("  SLTU: result=" << result);
                break;
            case ALUOp::XOR:  
                result = a ^ b; 
                DEBUG_PRINT("  XOR: result=0x" << hex << result << dec);
                break;
            case ALUOp::SRL:  
                result = a >> (b & 0x1F); 
                DEBUG_PRINT("  SRL: result=0x" << hex << result << dec);
                break;
            case ALUOp::SRA:  
                result = static_cast<int32_t>(a) >> (b & 0x1F); 
                DEBUG_PRINT("  SRA: result=0x" << hex << result << dec);
                break;
            case ALUOp::OR:   
                result = a | b; 
                DEBUG_PRINT("  OR: result=0x" << hex << result << dec);
                break;
            case ALUOp::AND:  
                result = a & b; 
                DEBUG_PRINT("  AND: result=0x" << hex << result << dec);
                break;
            default: 
                DEBUG_PRINT("  UNKNOWN ALU OP");
                result = 0;
        }
        
        return result;
    }

    // Branch condition check
    bool evaluate_branch(uint32_t a, uint32_t b, BranchCond cond) {
        bool result = false;
        
        DEBUG_PRINT("Branch condition check: a=0x" << hex << a << ", b=0x" << b << dec);
        
        switch (cond) {
            case BranchCond::EQ:   
                result = a == b; 
                DEBUG_PRINT("  EQ: result=" << result);
                break;
            case BranchCond::NE:   
                result = a != b; 
                DEBUG_PRINT("  NE: result=" << result);
                break;
            case BranchCond::LT:   
                result = static_cast<int32_t>(a) < static_cast<int32_t>(b); 
                DEBUG_PRINT("  LT: result=" << result);
                break;
            case BranchCond::GE:   
                result = static_cast<int32_t>(a) >= static_cast<int32_t>(b); 
                DEBUG_PRINT("  GE: result=" << result);
                break;
            case BranchCond::LTU:  
                result = a < b; 
                DEBUG_PRINT("  LTU: result=" << result);
                break;
            case BranchCond::GEU:  
                result = a >= b; 
                DEBUG_PRINT("  GEU: result=" << result);
                break;
            case BranchCond::TRUE: 
                result = true; 
                DEBUG_PRINT("  TRUE: result=true");
                break;
            case BranchCond::FALSE:
                result = false; 
                DEBUG_PRINT("  FALSE: result=false");
                break;
            default: 
                DEBUG_PRINT("  UNKNOWN BRANCH CONDITION");
                result = false;
        }
        
        return result;
    }

    // Memory access operations
    uint32_t read_memory(uint32_t addr, MemOp op) {
        uint32_t aligned_addr = addr & ~0x3;  // Word-align the address
        uint32_t word = data_memory[aligned_addr];
        uint32_t offset = addr & 0x3;
        uint32_t result = 0;
        
        DEBUG_PRINT("Memory read: addr=0x" << hex << addr 
                    << ", aligned_addr=0x" << aligned_addr 
                    << ", word=0x" << word << dec);
        
        switch (op) {
            case MemOp::LB: {
                uint8_t byte = (word >> (offset * 8)) & 0xFF;
                result = (byte & 0x80) ? (byte | 0xFFFFFF00) : byte;
                DEBUG_PRINT("  LB: result=0x" << hex << result << dec);
                break;
            }
            case MemOp::LH: {
                if (offset > 2) {
                    DEBUG_PRINT("  LH: MISALIGNED ACCESS");
                    return 0; // Misaligned access
                }
                uint16_t half = (word >> (offset * 8)) & 0xFFFF;
                result = (half & 0x8000) ? (half | 0xFFFF0000) : half;
                DEBUG_PRINT("  LH: result=0x" << hex << result << dec);
                break;
            }
            case MemOp::LW:
                result = word;
                DEBUG_PRINT("  LW: result=0x" << hex << result << dec);
                break;
            case MemOp::LBU: {
                uint8_t byte = (word >> (offset * 8)) & 0xFF;
                result = byte;
                DEBUG_PRINT("  LBU: result=0x" << hex << result << dec);
                break;
            }
            case MemOp::LHU: {
                if (offset > 2) {
                    DEBUG_PRINT("  LHU: MISALIGNED ACCESS");
                    return 0; // Misaligned access
                }
                uint16_t half = (word >> (offset * 8)) & 0xFFFF;
                result = half;
                DEBUG_PRINT("  LHU: result=0x" << hex << result << dec);
                break;
            }
            default:
                DEBUG_PRINT("  UNKNOWN MEMORY READ OP");
                result = 0;
        }
        
        return result;
    }

    void write_memory(uint32_t addr, uint32_t value, MemOp op) {
        uint32_t aligned_addr = addr & ~0x3;  // Word-align the address
        uint32_t word = data_memory[aligned_addr];
        uint32_t offset = addr & 0x3;
        
        DEBUG_PRINT("Memory write: addr=0x" << hex << addr 
                    << ", aligned_addr=0x" << aligned_addr 
                    << ", value=0x" << value << dec);
        
        switch (op) {
            case MemOp::SB: {
                uint32_t mask = ~(0xFF << (offset * 8));
                word = (word & mask) | ((value & 0xFF) << (offset * 8));
                DEBUG_PRINT("  SB: new word=0x" << hex << word << dec);
                break;
            }
            case MemOp::SH: {
                if (offset > 2) {
                    DEBUG_PRINT("  SH: MISALIGNED ACCESS");
                    return; // Misaligned access
                }
                uint32_t mask = ~(0xFFFF << (offset * 8));
                word = (word & mask) | ((value & 0xFFFF) << (offset * 8));
                DEBUG_PRINT("  SH: new word=0x" << hex << word << dec);
                break;
            }
            case MemOp::SW:
                word = value;
                DEBUG_PRINT("  SW: new word=0x" << hex << word << dec);
                break;
            default:
                DEBUG_PRINT("  UNKNOWN MEMORY WRITE OP");
                return;
        }
        
        data_memory[aligned_addr] = word;
    }

    // Pipeline stages implementation
    void instruction_fetch() {
        DEBUG_PRINT("STAGE: Instruction Fetch");
        
        // Check for branch/jump from EX stage
        if (ex_mem.valid && ex_mem.branch_taken) {
            DEBUG_PRINT("  Branch taken detected - updating PC to 0x" << hex 
                        << ex_mem.branch_target << dec << " and invalidating pipeline");
            pc = ex_mem.branch_target;
            // Invalidate earlier pipeline stages
            if_id.valid = false;
            id_ex.valid = false;
        }
        
        // Fetch instruction from memory
        uint32_t instr = instruction_memory[pc];
        DEBUG_PRINT("  Fetched instruction: 0x" << hex << instr 
                    << " from address 0x" << pc << dec);
        
        // Update IF/ID register
        if_id.pc = pc;
        if_id.instruction = instr;
        if_id.valid = true;
        ex_mem.branch_taken = false; // Reset branch taken flag
        
        // Increment PC
        pc += 4;
        DEBUG_PRINT("  PC incremented to 0x" << hex << pc << dec);
    }

    void instruction_decode() {
        DEBUG_PRINT("STAGE: Instruction Decode");
        
        if (!if_id.valid) {
            DEBUG_PRINT("  Pipeline stage invalid - skipping");
            return;
        }
        
        uint32_t instr = if_id.instruction;
        DEBUG_PRINT("  Decoding instruction: 0x" << hex << instr << dec);
        
        uint32_t opcode = extract_opcode(instr);
        uint32_t funct3 = extract_funct3(instr);
        uint32_t funct7 = extract_funct7(instr);
        uint32_t rs1 = extract_rs1(instr);
        uint32_t rs2 = extract_rs2(instr);
        uint32_t rd = extract_rd(instr);
        
        InstrType instr_type = decode_instr_type(instr);
        uint32_t imm = extract_immediate(instr, instr_type);
        
        // Determine control signals
        bool reg_write = false;
        bool mem_read = false;
        bool mem_write = false;
        bool alu_src = false;
        bool use_pc = false;
        ALUOp alu_op = ALUOp::NONE;
        BranchCond branch_cond = BranchCond::FALSE;
        MemOp mem_op = MemOp::NONE;
        
        DEBUG_PRINT("  Setting control signals based on opcode 0x" << hex 
                    << opcode << dec);
        
        switch (opcode) {
            case OPCODE_OP:  // R-type
                DEBUG_PRINT("  R-type instruction");
                reg_write = true;
                alu_src = false;
                
                if (funct7 == 0x00) {
                    switch (funct3) {
                        case 0x0: alu_op = ALUOp::ADD; break;
                        case 0x1: alu_op = ALUOp::SLL; break;
                        case 0x2: alu_op = ALUOp::SLT; break;
                        case 0x3: alu_op = ALUOp::SLTU; break;
                        case 0x4: alu_op = ALUOp::XOR; break;
                        case 0x5: alu_op = ALUOp::SRL; break;
                        case 0x6: alu_op = ALUOp::OR; break;
                        case 0x7: alu_op = ALUOp::AND; break;
                    }
                } else if (funct7 == 0x20) {
                    switch (funct3) {
                        case 0x0: alu_op = ALUOp::SUB; break;
                        case 0x5: alu_op = ALUOp::SRA; break;
                    }
                }
                break;
                
            case OPCODE_OP_IMM:  // I-type ALU
                DEBUG_PRINT("  I-type ALU instruction");
                reg_write = true;
                alu_src = true;
                
                switch (funct3) {
                    case 0x0: alu_op = ALUOp::ADD; break;
                    case 0x1: alu_op = ALUOp::SLL; break;
                    case 0x2: alu_op = ALUOp::SLT; break;
                    case 0x3: alu_op = ALUOp::SLTU; break;
                    case 0x4: alu_op = ALUOp::XOR; break;
                    case 0x5: 
                        if ((imm >> 5) & 0x7F == 0x0) 
                            alu_op = ALUOp::SRL;
                        else if ((imm >> 5) & 0x7F == 0x20)
                            alu_op = ALUOp::SRA;
                        break;
                    case 0x6: alu_op = ALUOp::OR; break;
                    case 0x7: alu_op = ALUOp::AND; break;
                }
                break;
                
            case OPCODE_LOAD:  // Load
                DEBUG_PRINT("  Load instruction");
                reg_write = true;
                mem_read = true;
                alu_src = true;
                alu_op = ALUOp::ADD;
                
                switch (funct3) {
                    case 0x0: mem_op = MemOp::LB; break;
                    case 0x1: mem_op = MemOp::LH; break;
                    case 0x2: mem_op = MemOp::LW; break;
                    case 0x4: mem_op = MemOp::LBU; break;
                    case 0x5: mem_op = MemOp::LHU; break;
                }
                break;
                
            case OPCODE_STORE:  // Store
                DEBUG_PRINT("  Store instruction");
                mem_write = true;
                alu_src = true;
                alu_op = ALUOp::ADD;
                
                switch (funct3) {
                    case 0x0: mem_op = MemOp::SB; break;
                    case 0x1: mem_op = MemOp::SH; break;
                    case 0x2: mem_op = MemOp::SW; break;
                }
                break;
                
            case OPCODE_BRANCH:  // Branch
                DEBUG_PRINT("  Branch instruction");
                alu_op = ALUOp::SUB;
                
                switch (funct3) {
                    case 0x0: branch_cond = BranchCond::EQ; break;
                    case 0x1: branch_cond = BranchCond::NE; break;
                    case 0x4: branch_cond = BranchCond::LT; break;
                    case 0x5: branch_cond = BranchCond::GE; break;
                    case 0x6: branch_cond = BranchCond::LTU; break;
                    case 0x7: branch_cond = BranchCond::GEU; break;
                }
                break;
                
            case OPCODE_JAL:  // Jump and Link
                DEBUG_PRINT("  JAL instruction");
                reg_write = true;
                alu_op = ALUOp::ADD;
                branch_cond = BranchCond::TRUE;
                use_pc = true;
                break;
                
            case OPCODE_JALR:  // Jump and Link Register
                DEBUG_PRINT("  JALR instruction");
                reg_write = true;
                alu_src = true;
                alu_op = ALUOp::ADD;
                branch_cond = BranchCond::TRUE;
                break;
                
            case OPCODE_LUI:  // Load Upper Immediate
                DEBUG_PRINT("  LUI instruction");
                reg_write = true;
                alu_op = ALUOp::ADD;
                alu_src = true;
                break;
                
            case OPCODE_AUIPC:  // Add Upper Immediate to PC
                DEBUG_PRINT("  AUIPC instruction");
                reg_write = true;
                alu_op = ALUOp::ADD;
                use_pc = true;
                break;
                
            default:
                DEBUG_PRINT("  Unknown instruction");
        }
        
        // Read register values
        uint32_t rs1_val = read_register(rs1);
        uint32_t rs2_val = read_register(rs2);
        
        // Forward data if there's a RAW hazard
        // From EX/MEM stage
        if (ex_mem.valid && ex_mem.reg_write && ex_mem.rd != 0) {
            if (ex_mem.rd == rs1) {
                DEBUG_PRINT("  FORWARDING: EX/MEM -> rs1 (x" << rs1 << ")");
                rs1_val = ex_mem.alu_result;
            }
            if (ex_mem.rd == rs2) {
                DEBUG_PRINT("  FORWARDING: EX/MEM -> rs2 (x" << rs2 << ")");
                rs2_val = ex_mem.alu_result;
            }
        }
        
        // From MEM/WB stage
        if (mem_wb.valid && mem_wb.reg_write && mem_wb.rd != 0) {
            uint32_t wb_data = mem_wb.mem_to_reg ? mem_wb.mem_data : mem_wb.alu_result;
            if (mem_wb.rd == rs1 && !(ex_mem.valid && ex_mem.reg_write && ex_mem.rd == rs1)) {
                DEBUG_PRINT("  FORWARDING: MEM/WB -> rs1 (x" << rs1 << ")");
                rs1_val = wb_data;
            }
            if (mem_wb.rd == rs2 && !(ex_mem.valid && ex_mem.reg_write && ex_mem.rd == rs2)) {
                DEBUG_PRINT("  FORWARDING: MEM/WB -> rs2 (x" << rs2 << ")");
                rs2_val = wb_data;
            }
        }
        
        // Update ID/EX register
        DEBUG_PRINT("  Updating ID/EX register");
        id_ex.pc = if_id.pc;
        id_ex.rs1_val = rs1_val;
        id_ex.rs2_val = rs2_val;
        id_ex.imm = imm;
        id_ex.rd = rd;
        id_ex.rs1 = rs1;
        id_ex.rs2 = rs2;
        id_ex.alu_op = alu_op;
        id_ex.branch_cond = branch_cond;
        id_ex.mem_op = mem_op;
        id_ex.reg_write = reg_write;
        id_ex.mem_read = mem_read;
        id_ex.mem_write = mem_write;
        id_ex.alu_src = alu_src;
        id_ex.use_pc = use_pc;
        id_ex.valid = if_id.valid;
    }

    void execute() {
        DEBUG_PRINT("STAGE: Execute");
        
        if (!id_ex.valid) {
            DEBUG_PRINT("  Pipeline stage invalid - skipping");
            return;
        }
        
        // Calculate ALU inputs
        uint32_t alu_in1 = id_ex.use_pc ? id_ex.pc : id_ex.rs1_val;
        uint32_t alu_in2 = id_ex.alu_src ? id_ex.imm : id_ex.rs2_val;
        
        DEBUG_PRINT("  ALU inputs: in1=0x" << hex << alu_in1 << " (from " 
                    << (id_ex.use_pc ? "PC" : "rs1") << "), in2=0x" << alu_in2 
                    << " (from " << (id_ex.alu_src ? "imm" : "rs2") << ")" << dec);
        
        // Execute ALU operation
        uint32_t alu_result = execute_alu(alu_in1, alu_in2, id_ex.alu_op);
        
        // Branch evaluation
        bool branch_taken = false;
        uint32_t branch_target = 0;
        
        if (id_ex.branch_cond != BranchCond::FALSE) {
            DEBUG_PRINT("  Evaluating branch condition");
            bool cond_met = evaluate_branch(id_ex.rs1_val, id_ex.rs2_val, id_ex.branch_cond);
            
            if (cond_met) {
                branch_taken = true;
                
                // Calculate branch target
                if (extract_opcode(if_id.instruction) == OPCODE_JALR) {
                    branch_target = (id_ex.rs1_val + id_ex.imm) & ~1; // Clear lowest bit
                    DEBUG_PRINT("  JALR branch target: 0x" << hex << branch_target << dec);
                } else {
                    branch_target = id_ex.pc + id_ex.imm;
                    DEBUG_PRINT("  Branch target: 0x" << hex << branch_target << dec);
                }
            }
        }
        
        // Update EX/MEM register
        DEBUG_PRINT("  Updating EX/MEM register");
        ex_mem.pc = id_ex.pc;
        ex_mem.alu_result = alu_result;
        ex_mem.rs2_val = id_ex.rs2_val;
        ex_mem.rd = id_ex.rd;
        ex_mem.mem_op = id_ex.mem_op;
        ex_mem.reg_write = id_ex.reg_write;
        ex_mem.mem_read = id_ex.mem_read;
        ex_mem.mem_write = id_ex.mem_write;
        ex_mem.branch_taken = branch_taken;
        ex_mem.branch_target = branch_target;
        ex_mem.valid = id_ex.valid;
    }

    void memory_access() {
        DEBUG_PRINT("STAGE: Memory Access");
        
        if (!ex_mem.valid) {
            DEBUG_PRINT("  Pipeline stage invalid - skipping");
            return;
        }
        
        uint32_t mem_data = 0;
        
        // Memory read
        if (ex_mem.mem_read) {
            DEBUG_PRINT("  Memory read at address 0x" << hex << ex_mem.alu_result << dec);
            mem_data = read_memory(ex_mem.alu_result, ex_mem.mem_op);
        }
        
        // Memory write
        if (ex_mem.mem_write) {
            DEBUG_PRINT("  Memory write at address 0x" << hex << ex_mem.alu_result 
                        << " value 0x" << ex_mem.rs2_val << dec);
            write_memory(ex_mem.alu_result, ex_mem.rs2_val, ex_mem.mem_op);
        }
        
        // Update MEM/WB register
        DEBUG_PRINT("  Updating MEM/WB register");
        mem_wb.alu_result = ex_mem.alu_result;
        mem_wb.mem_data = mem_data;
        mem_wb.rd = ex_mem.rd;
        mem_wb.reg_write = ex_mem.reg_write;
        mem_wb.mem_to_reg = ex_mem.mem_read;
        mem_wb.valid = ex_mem.valid;
    }

    void write_back() {
        DEBUG_PRINT("STAGE: Write Back");
        
        if (!mem_wb.valid) {
            DEBUG_PRINT("  Pipeline stage invalid - skipping");
            return;
        }
        
        // Write back to register file
        if (mem_wb.reg_write && mem_wb.rd != 0) {
            uint32_t write_data = mem_wb.mem_to_reg ? mem_wb.mem_data : mem_wb.alu_result;
            DEBUG_PRINT("  Writing 0x" << hex << write_data << dec 
                        << " to register x" << mem_wb.rd);
            write_register(mem_wb.rd, write_data);
        } else if (mem_wb.reg_write && mem_wb.rd == 0) {
            DEBUG_PRINT("  Attempted write to x0 (ignored)");
        } else {
            DEBUG_PRINT("  No register write");
        }
    }

    // Run single clock cycle
    void clock_cycle() {
        DEBUG_PRINT("======= BEGIN CYCLE " << cycle_count + 1 << " =======");
        
        // Pipeline stages must execute in reverse order to prevent data loss
        write_back();
        memory_access();
        execute();
        instruction_decode();
        instruction_fetch();
        
        cycle_count++;
        
        DEBUG_PRINT("======= END CYCLE " << cycle_count << " =======\n");
    }

    // Run program for specified number of cycles
    void run(int cycles) {
        cout << "Running for " << cycles << " cycles" << endl;
        for (int i = 0; i < cycles; i++) {
            clock_cycle();
        }
        cout << "Execution completed after " << cycles << " cycles" << endl;
    }

    // Dump registers and pipeline state for debugging
    void dump_state() {
        cout << "======= PROCESSOR STATE =======" << endl;
        cout << "Cycle count: " << cycle_count << endl;
        cout << "PC: 0x" << hex << pc << dec << endl;
        
        cout << "\nRegisters:" << endl;
        for (int i = 0; i < 32; i++) {
            cout << "x" << setw(2) << setfill('0') << i << ": 0x" 
                      << hex << setw(8) << setfill('0') << registers[i] << dec;
            if (i % 4 == 3) cout << endl;
            else cout << "\t";
        }
        
        cout << "\nPipeline State:" << endl;
        cout << "IF/ID: valid=" << if_id.valid << " pc=0x" << hex << if_id.pc 
                 << " instr=0x" << if_id.instruction << dec << endl;
        cout << "ID/EX: valid=" << id_ex.valid << " pc=0x" << hex << id_ex.pc 
                 << " rd=" << id_ex.rd << " alu_op=" << static_cast<int>(id_ex.alu_op) << dec << endl;
        cout << "EX/MEM: valid=" << ex_mem.valid << " alu_result=0x" << hex 
                 << ex_mem.alu_result << " rd=" << ex_mem.rd << dec << endl;
        cout << "MEM/WB: valid=" << mem_wb.valid << " alu_result=0x" << hex 
                 << mem_wb.alu_result << " rd=" << mem_wb.rd << dec << endl;
        
        cout << "\nData Memory (non-zero values):" << endl;
        int memEntries = 0;
        for (size_t i = 0; i < data_memory.size(); i += 4) {
            if (data_memory[i] != 0) {
                cout << "0x" << hex << setw(8) << i << ": 0x" 
                          << setw(8) << data_memory[i] << dec << endl;
                memEntries++;
                if (memEntries >= 10) {
                    cout << "... (more non-zero entries exist)" << endl;
                    break;
                }
            }
        }
        cout << "===============================" << endl;
    }

    void store_user_data(int addr, int value) {
        DEBUG_PRINT("Storing user data at addr 0x" << hex << addr 
                   << ": 0x" << value << dec);
        data_memory[addr] = value;
    }

private:
    // Memory
    vector<uint32_t> instruction_memory;
    vector<uint32_t> data_memory;
    
    // Registers
    uint32_t registers[32];
    uint32_t pc;
    
    // Pipeline registers
    IF_ID_Reg if_id;
    ID_EX_Reg id_ex;
    EX_MEM_Reg ex_mem;
    MEM_WB_Reg mem_wb;
    
    // Statistics
    uint64_t cycle_count;
};

// Main function with scanf and assembly input
int main() {
    // Initialize processor
    RV32I_5Stage cpu;
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
    
    // Ask for input data to store in memory
    // 
    
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
