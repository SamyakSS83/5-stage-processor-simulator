#include <bits/stdc++.h>
#include<cstdint>
#include"cycle_stages.hpp"
using namespace std;
 
#define DEBUG_MODE 1

#if DEBUG_MODE
#define DEBUG_PRINT(msg) cout << "[ DEBUG] " << msg << endl
#else
#define DEBUG_PRINT(msg)
#endif

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
            uint32_t pc;
            uint32_t alu_result;
            uint32_t mem_data;
            uint32_t rd;
            bool reg_write;
            bool mem_to_reg;
            bool valid;
        };
    
        // Constructor
        RV32I_5Stage(uint32_t mem_size, bool enable):
            instruction_memory(mem_size, 0),
            data_memory(mem_size, 0),
            pc(0),
            cycle_count(0),
            pipeline_stalled(false),
            stall_count(0),
            id_needs_flush(false),
            pending_branch_target(0),
            stall_just_resolved(false),
            enable_forwarding(enable) {
            
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
            mem_wb = MEM_WB_Reg{0, 0, 0, 0, false, false, false};
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
                    imm = (instr >> 20) ;
                   
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
            
            if (!enable_forwarding) {
                // Don't fetch new instruction if pipeline is stalled OR stall was just resolved
                if (pipeline_stalled || stall_just_resolved) {
                    DEBUG_PRINT("  Pipeline stalled or stall just resolved - not fetching new instruction");
                    return;
                }
                
                // Check for branch from ID stage
                if (id_needs_flush) {
                    DEBUG_PRINT("  Branch taken detected - updating PC to 0x" << hex 
                                << pending_branch_target << dec << " and invalidating IF/ID");
                    pc = pending_branch_target;
                    if_id.valid = false;
                    id_needs_flush = false;
                } else {
                    // Fetch instruction from memory
                    uint32_t instr = instruction_memory[pc];
                    DEBUG_PRINT("  Fetched instruction: 0x" << hex << instr 
                                << " from address 0x" << pc << dec);
                    
                    // Update IF/ID register
                    if (pc/4 < cycle_stages.size())
                    cycle_stages[pc/4][cycle_count] = "IF";
                    cout << "pc: " << pc << " stage: " << "IF" << " cycle " << cycle_count << endl;
                    if_id.pc = pc;
                    if_id.instruction = instr;
                    if_id.valid = true;
                    
                    // Increment PC
                    pc += 4;
                    DEBUG_PRINT("  PC incremented to 0x" << hex << pc << dec);
                }
            }
    
            else {
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
            if (pc/4 < cycle_stages.size())
            cycle_stages[pc/4][cycle_count] = "IF";
             cout << "pc: " << pc << " stage: " << "IF" << " cycle " << cycle_count << endl;   
            // Update IF/ID register
            if_id.pc = pc;
            if_id.instruction = instr;
            if_id.valid = true;
            ex_mem.branch_taken = false; // Reset branch taken flag
            
            // Increment PC
            pc += 4;
            DEBUG_PRINT("  PC incremented to 0x" << hex << pc << dec);
            }
        }
    
        void instruction_decode() {
            DEBUG_PRINT("STAGE: Instruction Decode");
            
            if (!enable_forwarding) {       
                cout << "not forwarding " << endl; 
                if (!if_id.valid) {
                    DEBUG_PRINT("  Pipeline stage invalid - skipping");
                    id_ex.valid = false;
                    return;
                }
                
                uint32_t instr = if_id.instruction;
                uint32_t pc_1 = if_id.pc;
                if (pc_1/4 < cycle_stages.size())
                cycle_stages[pc_1/4][cycle_count] = "ID";
                cout << "pc: " << pc_1 << " stage: " << "ID " << "cycle " << cycle_count << endl ;
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
                                if (((imm >> 5) & 0x7F) == 0x0) 
                                    alu_op = ALUOp::SRL;
                                else if (((imm >> 5) & 0x7F) == 0x20)
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
                        use_pc = true;
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
                
                // Check for RAW hazards - we need to stall if:
                // 1. Instruction in EX stage will write to a register we need
                // 2. Instruction in MEM stage will write to a register we need from a load
                bool needs_stall = false;
                cout << "rs1: " << rs1 << " rs2: " << rs2 << endl;
                cout << "rd: " << rd << endl;
                cout << "id_ex.rd: " << id_ex.rd << endl;
                cout << "ex_mem.rd: " << ex_mem.rd << endl;
                cout << "id_ex.valid: " << id_ex.valid << endl;
                // Check if we need values from registers that are being written to by previous instructions
                if ((rs1 != 0) && (
                    // EX stage will write to rs1
                    (id_ex.valid && id_ex.reg_write && id_ex.rd == rs1) ||
                    // MEM stage will load into rs1
                    (ex_mem.valid && ex_mem.mem_read && ex_mem.rd == rs1))) {
                    DEBUG_PRINT("  RAW hazard detected: rs1 (x" << rs1 << ") dependency");
                    needs_stall = true;
                }
                
                if ((rs2 != 0) && (
                    // EX stage will write to rs2
                    (id_ex.valid && id_ex.reg_write && id_ex.rd == rs2) ||
                    // MEM stage will load into rs2
                    (ex_mem.valid && ex_mem.mem_read && ex_mem.rd == rs2))) {
                    DEBUG_PRINT("  RAW hazard detected: rs2 (x" << rs2 << ") dependency");
                    needs_stall = true;
                }
                
                // Branch evaluation in ID stage - only if we're not stalling
                if (!needs_stall && branch_cond != BranchCond::FALSE) {
                    DEBUG_PRINT("  Evaluating branch in ID stage");
                    
                    // Read register values for branch evaluation
                    uint32_t rs1_val = read_register(rs1);
                    uint32_t rs2_val = read_register(rs2);
                    bool cond_met = evaluate_branch(rs1_val, rs2_val, branch_cond);
                    
                    if (cond_met) {
                        DEBUG_PRINT("  Branch condition met");
                        // Calculate branch target
                        if (opcode == OPCODE_JALR) {
                            pending_branch_target = (rs1_val + imm) & ~1; // Clear lowest bit
                        } else {
                            pending_branch_target = if_id.pc + imm;
                        }
                        
                        id_needs_flush = true;
                        DEBUG_PRINT("  Branch target set to 0x" << hex << pending_branch_target << dec);
                    }
                }
                
                // Handle stall logic
                if (needs_stall) {
                    DEBUG_PRINT(" Stalling pipeline");
                    pipeline_stalled = true;
                    stall_just_resolved = false;  // Not resolving, starting a stall
                    stall_count++;
                    
                    // Don't update ID/EX - effectively inserting a bubble
                    id_ex.valid = false;
                } else {
                    // If we were stalled but now it's resolved
                    if (pipeline_stalled) {
                        DEBUG_PRINT("  Pipeline stall resolved after " << stall_count << " cycles");
                        pipeline_stalled = false;
                        stall_just_resolved = true;  // Mark that we just resolved a stall
                        stall_count = 0;
                        
                        // Insert a NOP (addi x0, x0, 0) - flush the potentially incorrect instruction
                        DEBUG_PRINT("  Inserting NOP to flush pipeline");
                        id_ex.valid = true;  // NOP is a valid instruction
                        id_ex.pc = if_id.pc;
                        id_ex.rs1_val = 0;
                        id_ex.rs2_val = 0;
                        id_ex.imm = 0;
                        id_ex.rd = 0;        // x0 register
                        id_ex.rs1 = 0;       // x0 register
                        id_ex.rs2 = 0;       // x0 register
                        id_ex.alu_op = ALUOp::ADD;  // ADD operation
                        id_ex.branch_cond = BranchCond::FALSE;
                        id_ex.mem_op = MemOp::NONE;
                        id_ex.reg_write = false;  // Don't write to any register
                        id_ex.mem_read = false;   // No memory read
                        id_ex.mem_write = false;  // No memory write
                        id_ex.alu_src = true;     // Use immediate (0)
                        id_ex.use_pc = false;     // Don't use PC
                        return;  // Exit early - don't process the instruction in IF/ID yet
                    } else {
                        stall_just_resolved = false;  // Normal operation, not just resolved
                    }
                    
                    // Read register values - now dependency-free
                    uint32_t rs1_val = read_register(rs1);
                    uint32_t rs2_val = read_register(rs2);
                    
                    // Update ID/EX register normally
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
            }
            else {
                if (!if_id.valid) {
                    DEBUG_PRINT("  Pipeline stage invalid - skipping");
                    return;
                }
                
                uint32_t instr = if_id.instruction;
                uint32_t pc_1 = if_id.pc;
                DEBUG_PRINT("  Decoding instruction: 0x" << hex << instr << dec);
                if (pc_1/4 < cycle_stages.size())
                cycle_stages[pc_1/4][cycle_count] = "ID";
                cout << "pc: " << pc_1 << "stage: " << "ID" << " cycle " << cycle_count << endl;
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
                                if (((imm >> 5) & 0x7F) == 0x0) 
                                    alu_op = ALUOp::SRL;
                                else if (((imm >> 5) & 0x7F) == 0x20)
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
                        use_pc = true;
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
        }
    
        void execute() {
            DEBUG_PRINT("STAGE: Execute");
            uint32_t pc_1 = id_ex.pc;
    
            if (!enable_forwarding) {
                
                if (!id_ex.valid) {
                    DEBUG_PRINT("  Pipeline stage invalid - skipping");
                    ex_mem.valid = false;
                    cout << "returning" << endl;
                    return;
                }

                
                // Calculate ALU input
                uint32_t alu_in1 = id_ex.use_pc ? id_ex.pc : id_ex.rs1_val;
                uint32_t alu_in2 = id_ex.alu_src ? id_ex.imm : id_ex.rs2_val;
                if (pc_1/4 < cycle_stages.size())
                cycle_stages[pc_1/4][cycle_count] = "EX";
                cout << "pc: " << pc_1 << " stage: " << "EX" << " cycle: " << cycle_count << endl;
                
                DEBUG_PRINT("  ALU inputs: in1=0x" << hex << alu_in1 << " (from " 
                            << (id_ex.use_pc ? "PC" : "rs1") << "), in2=0x" << alu_in2 
                            << " (from " << (id_ex.alu_src ? "imm" : "rs2") << ")" << dec);
                
                // Execute ALU operation
                uint32_t alu_result = execute_alu(alu_in1, alu_in2, id_ex.alu_op);
                
                // Branch handling is now done in ID stage
                
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
                ex_mem.branch_taken = false; // Branch handling is now in ID stage
                ex_mem.branch_target = 0;
                ex_mem.valid = id_ex.valid;
            }
    
            else {
                if (!id_ex.valid) {
                    DEBUG_PRINT("  Pipeline stage invalid - skipping");
                    cout << "returning" << endl;
                    return;
                }
                
                // Calculate ALU inputs
                uint32_t alu_in1 = id_ex.use_pc ? id_ex.pc : id_ex.rs1_val;
                uint32_t alu_in2 = id_ex.alu_src ? id_ex.imm : id_ex.rs2_val;
                if (pc_1/4 < cycle_stages.size())
                cycle_stages[pc_1/4][cycle_count] = "EX";
                cout << "pc: " << pc_1 << " stage: " << "EX" << " cycle: " << cycle_count << endl;
                
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
        
            
        }
    
        void memory_access() {
            DEBUG_PRINT("STAGE: Memory Access");
            
            if (!ex_mem.valid) {
                DEBUG_PRINT("  Pipeline stage invalid - skipping");
                mem_wb.valid = (enable_forwarding) ? mem_wb.valid : false;
                return;
            }
            
            uint32_t mem_data = 0;

            uint32_t pc_1  = ex_mem.pc;
            if (pc_1/4 < cycle_stages.size())
            cycle_stages[pc_1/4][cycle_count] = "MEM";
            cout << "pc: " << pc_1  << " stage: " << "MEM" << " cycle: " << cycle_count << endl;
            
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
            mem_wb.pc = ex_mem.pc;
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

            uint32_t pc_1  = mem_wb.pc;
            if (pc_1/4 < cycle_stages.size())
            cycle_stages[pc_1/4][cycle_count] = "WB";
            cout << "pc: " << pc_1  << " stage: " << "WB" << " cycle: " << cycle_count<< endl;
            
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
            
            if (!enable_forwarding) {
                // Reset the stall_just_resolved flag at the end of each cycle
                if (stall_just_resolved) {
                    DEBUG_PRINT("  Cleared stall_just_resolved flag");
                    stall_just_resolved = false;
                }
            }
            
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
    
        // Add these fields to the RV32I_5Stage class:
        bool pipeline_stalled;
        uint32_t stall_count;
        bool id_needs_flush;
        uint32_t pending_branch_target;
        bool stall_just_resolved; 
        bool enable_forwarding; // to be used when forwarding is enabled 
    };