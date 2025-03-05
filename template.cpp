#include<bits/stdc++.h>
using namespace std;


int reg[32];
int mem[10000];


struct IF{
    int PC;
    string instruction;
};

struct ID{
    int reg1_name; //to be used for addr in mem wale kaam
    int reg2_name; 
    int reg_future_name;
    string future_opn_ex;
    bool do_i_mem;
    bool in_mem_read;
    bool do_i_wb;
    int imm;
    bool is_I_type;
    bool is_branch_or_jump;

};

struct EX{
    int input_reg1;
    int input_reg2;
    int output_future_reg;
    string operation;
    int imm;
    bool is_I_type;
    bool do_i_mem;
    bool do_i_wb;
    bool in_mem_read;
};

struct MEM{
    int addr_reg;
    int data_reg;
    int data;
    bool in_mem_read;
    bool do_i_wb;
    bool do_i_mem;
};

struct WB{
    int data;
    int reg_name;
    bool do_i_wb;
};



IF actual_IF, temp_IF;
ID actual_ID, temp_ID;
EX actual_EX, temp_EX;
MEM actual_MEM, temp_MEM;
WB actual_WB, temp_WB;

void IFtoID (){


}

void IDtoEX (){
    actual_EX.input_reg1 = temp_ID.reg1_name;
    actual_EX.input_reg2 = temp_ID.reg2_name;
    actual_EX.output_future_reg = temp_ID.reg_future_name;
    actual_EX.operation = temp_ID.future_opn_ex;
    actual_EX.imm = temp_ID.imm;
    actual_EX.is_I_type = temp_ID.is_I_type;
    actual_EX.do_i_mem = temp_ID.do_i_mem;
    actual_EX.do_i_wb = temp_ID.do_i_wb;
    actual_EX.in_mem_read = temp_ID.in_mem_read;
}

void EXtoMEM() {
    actual_MEM.addr_reg = temp_EX.output_future_reg;
    actual_MEM.data_reg = temp_EX.input_reg2;
    actual_MEM.data = temp_EX.input_reg1;
    actual_MEM.in_mem_read = temp_EX.in_mem_read;
    actual_MEM.do_i_wb = temp_EX.do_i_wb;
    actual_MEM.do_i_mem = temp_EX.do_i_mem;
}

void MEMtoWB() {
    actual_WB.data = temp_MEM.data;
    actual_WB.reg_name = temp_MEM.data_reg;
    actual_WB.do_i_wb = temp_MEM.do_i_wb;
}


void actual_work(){}
