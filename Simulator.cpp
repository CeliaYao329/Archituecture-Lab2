//
//  Simulator.cpp
//  RISCV-Simulator
//
//  Created by CMouse on 2017/10/27.
//  Copyright © 2017年 Peking University. All rights reserved.
//

#include "Simulator.hpp"
#include "PipeReg.h" // Definition of pipeline registers
#include "Instructions.h" // Instruction codes
using namespace std;

// .text segment
extern unsigned int tadr;
extern unsigned int tsize;
extern unsigned int tvadr;

// .data segment
extern unsigned int dadr;
extern unsigned int dsize;
extern unsigned int dvadr;

extern unsigned long entry_main;
extern unsigned long size_main;
extern unsigned long endpc_main;
extern FILE *elf;

extern unsigned long global_pointer;
extern unsigned long result;

extern bool open_file(const char * filename);
extern void read_elf(const char * filename);

bool step_mode = false;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: Simulator [options] <filename>\n");
        printf("Options:\n");
        printf("-s\tRun by step.\n");
        return 1;
    }
    
    else if (argc == 2) {
        read_elf(argv[1]);
        load_memory(argv[1]);
    }
    
    // TODO: Remove hardcode
    else if (argc == 3) {
        if (strncmp(argv[1], "-s", 2) == 0) {
            step_mode = true;
        }
        read_elf(argv[argc - 1]);
        load_memory(argv[argc - 1]);
    }
    
    // PC Initialization
    PC = entry_main;
    endpc_main = entry_main + size_main - 3;
    
    // Register File Initialization
    for (int i = 0; i < 32; i++) {
        reg[i] = 0;
    }
    reg[2] = MAX / 2; // Stack pointer
    
    reg[3] = (REG)global_pointer; // Global pointer
    printf("\n");
    printf("Global pointer = %llx\n", reg[3]);
    
    simulate();
    
    printf("\n");
    printf("Simulation completed. Result located at 0x%lx\n", result);
    printf("Please choose:\n");
    printf("[1] Check register\n");
    printf("[2] Check memory\n");
    printf("[3] Check CPI\n");
    printf("[4] Exit\n");
    
    char choice;
    cin >> choice;
    while (choice == '1' || choice == '2' || choice == '3' ) {
        if (choice == '1') {
            print_reg();
        }
        else if (choice == '2') {
            print_mem();
        }
        else if(choice == '3'){
            print_CPI();
        }
        printf("\n");
        printf("Please choose:\n");
        printf("[1] Check register\n");
        printf("[2] Check memory\n");
        printf("[3] Exit\n");
        cin >> choice;
    }
    
    return 0;
}

void load_memory(const char * filename) {
    open_file(filename);
    // Load .text segment
    fseek(elf, tadr, SEEK_SET);
    fread(&memory[tvadr], 1, tsize, elf);
    // Load .data segment
    fseek(elf, dadr, SEEK_SET);
    fread(&memory[dvadr], 1, dsize, elf);
    fclose(elf);
}

void simulate() {
    unsigned long end = endpc_main;
    while (PC != end) {
        //every iteration simulates one time cycle
        cycle_cnt ++;

        //printf("\n");
        //printf("Executing the %lld instruction at 0x%lx\n", instruction_cnt, PC);
        IF();
        ID();
        EX();
        MEM();
        WB();

        
        if(IF_ID_stall) IF_ID_reset();
        else if(!IF_ID_stall) IF_ID_update();
        IF_ID_bubble = 0;
        IF_ID_stall = 0;

        if(ID_EX_bubble) ID_EX_reset();
        else ID_EX_update();
        ID_EX_bubble = 0;
        EX_MEM_update();
        MEM_WB_update();

        if (step_mode == true) {
            printf("\n");
            printf("Please choose:\n");
            printf("[1] Check register\n");
            printf("[2] Check memory\n");
            printf("[3] Continue\n");
            
            char choice;
            cin >> choice;
            while (choice == '1' || choice == '2') {
                if (choice == '1') {
                    print_reg();
                }
                else {
                    print_mem();
                }
                printf("\n");
                printf("Please choose:\n");
                printf("[1] Check register\n");
                printf("[2] Check memory\n");
                printf("[3] Continue\n");
                cin >> choice;
            }
        }
    }
}

void print_reg() {
    printf("Status of Register File:\n");
    for (int i = 0; i < 32; i+=2) {
        printf("x%i\t", i);
        printf("%017llx\t", reg[i]);
        printf("x%i\t", i + 1);
        printf("%017llx\n", reg[i + 1]);
    }
}

void print_mem() {
    printf("Enter Memory Address (hexadecimal):\n");
    unsigned int addr = 0;
    scanf("%x", &addr);
    if (addr >= MAX) {
        printf("Error: Out of range.\n");
        return;
    }
    printf("Status of Memory:\n");
    unsigned int print_range = 16;
    unsigned int addr_start = addr & (~(print_range - 1));
    for (unsigned int i = addr_start; i < addr_start + print_range; i++) {
        printf("%x\t", i);
        printf("%2x\n", memory[i]);
    }
}

void print_CPI(){
    printf("Cycle number: %lld \n",cycle_cnt);
    printf("Instruction number: %lld \n",instruction_cnt);
    printf("CPI: %.2f\n",double(cycle_cnt/instruction_cnt));
}

void IF() {
    iif.inst = *((unsigned int *) &memory[PC]);
    iif.PC = PC;
}

void IF_ID_update(){
    // Write IF_ID
    IF_ID.inst = iif.inst;
    // PC = PC + 4;
    IF_ID.PC = iif.PC;
}

void IF_ID_reset(){
    IF_ID.inst = 0;
    IF_ID.PC = 0;
}


void ID() {
    // Read IF_ID
    unsigned int inst = IF_ID.inst;
    unsigned long PC = IF_ID.PC;
    
    // Initialize signals
    unsigned long ValA = 0; // ALU Src A
    unsigned long ValB = 0; // ALU Src B
    unsigned char ALUOp = ALU_NONE;
    
    unsigned char Branch = 0;
    unsigned char MemRead = MEM_NONE;
    unsigned char MemWrite = MEM_NONE;
    unsigned char AlignPC = 0;
    
    unsigned char RegWrite = 0;
    unsigned char MemtoReg = 0;
    unsigned char RegDst = 0;
    
    //instruction decoding
    unsigned int OP = getbit(inst, 0, 7);
    unsigned int rd = getbit(inst, 7, 5);
    unsigned int rs1 = getbit(inst, 15, 5);
    unsigned int rs2 = getbit(inst, 20, 5);
    unsigned int funct3 = getbit(inst, 12, 3);
    unsigned int funct7 = getbit(inst, 25, 7);
    
    unsigned int i_imm = getbit(inst, 20, 12);
    
    unsigned int s_imm = getbit(inst, 25, 7) << 5;
    s_imm += getbit(inst, 7, 5);

    // {offset, 1b'0}
    unsigned int sb_imm = getbit(inst, 31, 1) << 12;
    sb_imm += getbit(inst, 7, 1) << 11;
    sb_imm += getbit(inst, 25, 6) << 5;
    sb_imm += getbit(inst, 8, 4) << 1;
    
    // {offset, 12b'0}
    unsigned int u_imm = getbit(inst, 12, 20) << 12;
    
    // {offset, 1b'0}
    unsigned int uj_imm = getbit(inst, 31, 1) << 20;
    uj_imm += getbit(inst, 12, 8) << 12;
    uj_imm += getbit(inst, 20, 1) << 11;
    uj_imm += getbit(inst, 21, 10) << 1;
    
    // Register Arithmetic
    if (OP == OP_R) {
        
        //data hazard
        // TODO
        if(rs1 == ID_EX.Ctrl_EX_RegDst || rs1 == EX_MEM.Ctrl_EX_RegDst || rs1 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }
        else if (rs2 == ID_EX.Ctrl_EX_RegDst || rs2 == EX_MEM.Ctrl_EX_RegDst || rs2 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }

        cout << "DEBUG: Register Arith" << endl;
        ValA = reg[rs1];
        ValB = reg[rs2];
        RegWrite = 1;
        RegDst = rd;
        if(funct3 == F3_ADD && funct7 == F7_ADD) {
            // add rd, rs1, rs2
            // R[rd] <- R[rs1] + R[rs2]
            ALUOp = ALU_ADD;
        }
        else if (funct3 == F3_MUL && funct7 == F7_MUL) {
            // mul rd, rs1, rs2
            // R[rd] <- (R[rs1] * R[rs2])[63:0]
            ALUOp = ALU_MUL;
        }
        else if (funct3 == F3_SUB && funct7 == F7_SUB) {
            // sub rd, rs1, rs2
            // R[rd] <- R[rs1] - R[rs2]
            ALUOp = ALU_SUB;
        }
        else if (funct3 == F3_SLL && funct7 == F7_SLL)
        {
            // sll rd, rs1, rs2
            // R[rd] <- R[rs1] << R[rs2]
            ALUOp = ALU_SLL;
        }
        else if (funct3 == F3_MULH && funct7 == F7_MULH)
        {
            // mulh rd, rsi, rs2
            // R[rd] <- (R[rs1] * R[rs2])[127:64]
            ALUOp = ALU_MULH;
        }
        else if (funct3 == F3_SLT && funct7 == F7_SLT)
        {
            // slt rd, rsi, rs2
            // R[rd] <- (R[rs1] < R[rs2])? 1:0
            ALUOp = ALU_SLT;
        }
        else if (funct3 == F3_XOR && funct7 == F7_XOR)
        {
            // xor rd, rsi, rs2
            // R[rd] <- R[rs1] ^ R[rs2]
            ALUOp = ALU_XOR;
        }
        else if (funct3 == F3_DIV && funct7 == F7_DIV)
        {
            // div rd, rsi, rs2
            // R[rd] <- R[rs1] / R[rs2]
            ALUOp = ALU_DIV;
        }
        else if (funct3 == F3_SRL && funct7 == F7_SRL)
        {
            // srl rd, rsi, rs2
            // R[rd] <- R[rs1] >> R[rs2]
            ALUOp = ALU_SRL;
        }
        else if (funct3 == F3_SRA && funct7 == F7_SRA)
        {
            // sra rd, rsi, rs2
            // R[rd] <- R[rs1] << R[rs2]
            ALUOp = ALU_SRA;
        }
        else if (funct3 == F3_OR && funct7 == F7_OR)
        {
            // or rd, rsi, rs2
            // R[rd] <- R[rs1] | R[rs2]
            ALUOp = ALU_OR;
        }
        else if (funct3 == F3_REM && funct7 == F7_REM)
        {
            // rem rd, rsi, rs2
            // R[rd] <- R[rs1] % R[rs2]
            ALUOp = ALU_REM;
        }
        else if (funct3 == F3_AND && funct7 == F7_AND)
        {
            // and rd, rsi, rs2
            // R[rd] <- R[rs1] & R[rs2]
            ALUOp = ALU_AND;
        }
    }
    
    // Load from memory
    else if (OP == OP_L) {
        cout << "DEBUG: Load" << endl;

        if(rs1 == ID_EX.Ctrl_EX_RegDst || rs1 == EX_MEM.Ctrl_EX_RegDst || rs1 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }

        // lx rd, offset(rs1)
        // R[rd] <- SignExt(Mem(R[rs1] + offset, size))
        ValA = reg[rs1];
        ValB = ext_signed(i_imm, 12);
        ALUOp = ALU_ADD;
        RegWrite = 1;
        MemtoReg = 1;
        RegDst = rd;
        if (funct3 == F3_LB) {
            MemRead = MEM_BYTE;
        }
        else if (funct3 == F3_LH) {
            MemRead = MEM_HALF;
        }
        else if (funct3 == F3_LW) {
            MemRead = MEM_WORD;
        }
        else if (funct3 == F3_LD) {
            MemRead = MEM_DOUBLE;
        }
    }
    
    else if (OP == 0x3b) {
        cout << "DEBUG: NEW INSTR" << endl;
        ValA = reg[rs1];
        ValB = reg[rs2];
        if(rs1 == ID_EX.Ctrl_EX_RegDst || rs1 == EX_MEM.Ctrl_EX_RegDst || rs1 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }
        if(rs2 == ID_EX.Ctrl_EX_RegDst || rs2 == EX_MEM.Ctrl_EX_RegDst || rs2 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }
        RegWrite = 1;
        RegDst = rd;
        ALUOp = ALU_ADD;
    }
    
    // Immediate Arithmetic
    else if (OP == OP_I) {
        cout << "DEBUG: Immediate Arith" << endl;
        if(rs1 == ID_EX.Ctrl_EX_RegDst || rs1 == EX_MEM.Ctrl_EX_RegDst || rs1 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }

        ValA = reg[rs1];
        RegWrite = 1;
        RegDst = rd;
        if (funct3 == F3_ADDI)
        {
            // addi rd, rsi, imm
            // R[rd] <- R[rs1] + imm
            ValB = ext_signed(i_imm, 12);
            ALUOp = ALU_ADD;
        }
        else if (funct3 == F3_SLLI && funct7 == F7_SLLI)
        {
            // slli rd, rsi, imm
            // R[rd] <- R[rs1] << imm[5:0]
            ValB = getbit(i_imm, 0, 6);
            ALUOp = ALU_SLL;
        }
        else if (funct3 == F3_SLTI) {
            // slti rd, rsi, imm
            // R[rd] <- (R[rs1] < imm)? 1:0
            ValB = ext_signed(i_imm, 12);
            ALUOp = ALU_SLT;
        }
        else if (funct3 == F3_XORI) {
            // xori rd, rsi, imm
            // R[rd] <- R[rs1] ^ imm
            ValB = ext_signed(i_imm, 12);
            ALUOp = ALU_XOR;
        }
        else if (funct3 == F3_SRLI && funct7 == F7_SRLI)
        {
            // srli rd, rsi, imm
            // R[rd] <- R[rs1] >> imm[5:0]
            ValB = getbit(i_imm, 0, 6);
            ALUOp = ALU_SRL;
        }
        else if (funct3 == F3_SRAI && funct7 == F7_SRAI)
        {
            // srai rd, rsi, imm
            // R[rd] <- R[rs1] >> imm[5:0]
            ValB = getbit(i_imm, 0, 6);
            ALUOp = ALU_SRA;
        }
        else if (funct3 == F3_ORI) {
            // ori rd, rsi, imm
            // R[rd] <- R[rs1] | imm
            ValB = ext_signed(i_imm, 12);
            ALUOp = ALU_OR;
        }
        else if (funct3 == F3_ANDI) {
            // andi rd, rsi, imm
            // R[rd] <- R[rs1] & imm
            ValB = ext_signed(i_imm, 12);
            ALUOp = ALU_AND;
        }
    }
    
    // Wide Immediate Arithmetic
    else if (OP == OP_IW) {

        if(rs1 == ID_EX.Ctrl_EX_RegDst || rs1 == EX_MEM.Ctrl_EX_RegDst || rs1 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }

        cout << "DEBUG: Wide Immediate Arith" << endl;
        if (funct3 == F3_ADDIW) {
            // addiw rd, rs1, imm
            // R[rd] <- SignExt((R[rs1](63:0) + SignExt(imm))[31:0])
            ValA = reg[rs1];
            ValB = ext_signed(i_imm, 12);
            ALUOp = ALU_ADD;
            RegWrite = 1;
            RegDst = rd;
        }
        else if (funct3 == 0x1 && funct7 == 0x0) // 0027979b
        {
            // slliw rd, rsi, imm
            // R[rd] <- R[rs1] << imm[4:0]
            ValA = reg[rs1];
            ValB = getbit(i_imm, 0, 5);
            ALUOp = ALU_SLL;
            RegWrite = 1;
            RegDst = rd;
        }
        else if (funct3 == 0x5 && funct7 == 0x0) // 01f7d71b
        {
            // srliw rd, rsi, imm
            // R[rd] <- R[rs1] >> imm[4:0]
            ValA = reg[rs1];
            ValB = getbit(i_imm, 0, 5);
            ALUOp = ALU_SRL;
            RegWrite = 1;
            RegDst = rd;
        }
        else if (funct3 == 0x5 && funct7 == 0x20) // 4017d79b
        {
            // srai rd, rsi, imm
            // R[rd] <- R[rs1] >> imm[5:0]
            ValA = reg[rs1];
            ValB = getbit(i_imm, 0, 5);
            ALUOp = ALU_SRA;
            RegWrite = 1;
            RegDst = rd;
        }
    }
    
    else if (OP == OP_JALR) {
        if(rs1 == ID_EX.Ctrl_EX_RegDst || rs1 == EX_MEM.Ctrl_EX_RegDst || rs1 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }

        cout << "DEBUG: Jump and link register" << endl;
        // jalr rd, rs1, imm
        // R[rd] <- PC + 4
        // PC <- R[rs1] + imm (PC[0] = 0)
        // TODO: Move to suitable place
        reg[rd] = PC + 4;
        ValA = reg[rs1];
        ValB = ext_signed(i_imm, 12);
        ALUOp = ALU_ADD;
        Branch = 1;
        AlignPC = 1;
    }
    
    // TODO: Call procedure
    else if (OP == OP_ECALL) {
        cout << "DEBUG: Call" << endl;
        if (funct3 == F3_ECALL && funct7 == F7_ECALL) {
            // ???
        }
    }
    
    else if (OP == OP_S) {
        if(rs1 == ID_EX.Ctrl_EX_RegDst || rs1 == EX_MEM.Ctrl_EX_RegDst || rs1 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }
        if(rs2 == ID_EX.Ctrl_EX_RegDst || rs2 == EX_MEM.Ctrl_EX_RegDst || rs2 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }

        cout << "DEBUG: Store" << endl;
        // sx rs2, offset(rs1)
        // Mem(R[rs1] + offset) <- R[rs2][x:0]
        ValA = reg[rs1];
        ValB = ext_signed(s_imm, 12);
        ALUOp = ALU_ADD;
        if (funct3 == F3_SB) {
            MemWrite = MEM_BYTE;
        }
        else if (funct3 == F3_SH) {
            MemWrite = MEM_HALF;
        }
        else if (funct3 == F3_SW) {
            MemWrite = MEM_WORD;
        }
        else if (funct3 == F3_SD) {
            MemWrite = MEM_DOUBLE;
        }
    }
    
    else if (OP == OP_B) {

        if(rs1 == ID_EX.Ctrl_EX_RegDst || rs1 == EX_MEM.Ctrl_EX_RegDst || rs1 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }
        if(rs2 == ID_EX.Ctrl_EX_RegDst || rs2 == EX_MEM.Ctrl_EX_RegDst || rs2 == MEM_WB.Ctrl_EX_RegDst){
            ID_EX_bubble = 1;
            PC_stall = 1;
            IF_ID_stall = 1;
        }

        cout << "DEBUG: Branch" << endl;
        // bxx rs1, rs2, offset
        // if (R[rs1] ?? R[rs2])
        // PC <- PC + {offset, 1b'0}
        ValA = PC;
        ValB = ext_signed(sb_imm, 13);
        ALUOp = ALU_ADD;
        if (funct3 == F3_BEQ) {
            if (reg[rs1] == reg[rs2]) {
                Branch = 1;
            }
        }
        else if (funct3 == F3_BNE) {
            if (reg[rs1] != reg[rs2]) {
                Branch = 1;
            }
        }
        else if (funct3 == F3_BLT) {
            if (reg[rs1] < reg[rs2]) {
                Branch = 1;
            }
        }
        else if (funct3 == F3_BGE) {
            if (reg[rs1] >= reg[rs2]) {
                Branch = 1;
            }
        }
    }
    
    else if (OP == OP_AUIPC) {
        cout << "DEBUG: AUIPC" << endl;
        // auipc rd, offset
        // R[rd] <- PC + {offset, 12b'0}
        ValA = PC;
        ValB = ext_signed(u_imm, 32);
        ALUOp = ALU_ADD;
        RegWrite = 1;
        RegDst = rd;
    }
    
    else if (OP == OP_LUI) {
        cout << "DEBUG: LUI" << endl;
        // lui rd, offset
        // R[rd] <- {offset, 12b'0}
        ValA = 0;
        ValB = ext_signed(u_imm, 32);
        ALUOp = ALU_ADD;
        RegWrite = 1;
        RegDst = rd;
    }
    
    else if (OP == OP_JAL) {
        cout << "DEBUG: Jump and link" << endl;
        // jal rd, imm
        // R[rd] <- PC + 4
        // PC <- PC + {imm, 1b'0}
        // TODO: Move to suitable place
        if (rd != 0) {
            reg[rd] = PC + 4;
        }
        ValA = PC;
        ValB = ext_signed(i_imm, 12);
        ALUOp = ALU_ADD;
        Branch = 1;
    }
    
    // Write ID_EX
    id.PC = PC;
    
    // Control signals for execute
    id.ValA = ValA;
    id.ValB = ValB;
    id.Ctrl_EX_ALUOp = ALUOp;
    
    // Control signals for memory
    id.Ctrl_M_Branch = Branch;
    id.Ctrl_M_MemRead = MemRead;
    id.Ctrl_M_MemWrite = MemWrite;
    id.rs2 = rs2;
    
    // Control signals for write back
    id.Ctrl_WB_RegWrite = RegWrite;
    id.Ctrl_WB_MemtoReg = MemtoReg;
    id.Ctrl_EX_RegDst = RegDst;

    if (step_mode == true) {
        printf("DEBUG: ValA = %lx\n", ValA);
        printf("DEBUG: ValB = %lx\n", ValB);
        printf("DEBUG: MemRead = %i\n", MemRead);
        printf("DEBUG: MemWrite = %i\n", MemWrite);
        printf("DEBUG: RegWrite = %i\n", RegWrite);
        printf("DEBUG: MemtoReg = %i\n", MemtoReg);
    }
}

void ID_EX_update(){
    // Implementation of SEQ
    // Write ID_EX
    ID_EX.PC = id.PC;
    
    // Control signals for execute
    ID_EX.ValA = id.ValA;
    ID_EX.ValB = id.ValB;
    ID_EX.Ctrl_EX_ALUOp = id.Ctrl_EX_ALUOp;
    
    // Control signals for memory
    ID_EX.Ctrl_M_Branch = id.Ctrl_M_Branch;
    ID_EX.Ctrl_M_MemRead = id.Ctrl_M_MemRead;
    ID_EX.Ctrl_M_MemWrite = id.Ctrl_M_MemWrite;
    ID_EX.rs2 = id.rs2;
    
    // Control signals for write back
    ID_EX.Ctrl_WB_RegWrite = id.Ctrl_WB_RegWrite;
    ID_EX.Ctrl_WB_MemtoReg = id.Ctrl_WB_MemtoReg;
    ID_EX.Ctrl_EX_RegDst = id.Ctrl_EX_RegDst;
}

void ID_EX_reset(){
    // Implementation of SEQ
    // Write ID_EX
    ID_EX.PC = 0;
    
    // Control signals for execute
    ID_EX.ValA = 0;
    ID_EX.ValB = 0;
    ID_EX.Ctrl_EX_ALUOp = 0;
    
    // Control signals for memory
    ID_EX.Ctrl_M_Branch = 0;
    ID_EX.Ctrl_M_MemRead = 0;
    ID_EX.Ctrl_M_MemWrite = 0;;
    ID_EX.rs2 = id.rs2;
    
    // Control signals for write back
    ID_EX.Ctrl_WB_RegWrite = 0;
    ID_EX.Ctrl_WB_MemtoReg = 0;
    ID_EX.Ctrl_EX_RegDst = 0;
}

void EX() {
    // Read ID_EX
    unsigned long PC = ID_EX.PC;
    
    // Control signals for execute
    long ALUsrc1 = ID_EX.ValA;
    long ALUsrc2 = ID_EX.ValB;
    char ALUOp = ID_EX.Ctrl_EX_ALUOp;
    
    // Control signals for memory
    char Branch = ID_EX.Ctrl_M_Branch;
    char MemRead = ID_EX.Ctrl_M_MemRead;
    char MemWrite = ID_EX.Ctrl_M_MemWrite;
    unsigned char AlignPC = ID_EX.AlignPC;
    unsigned int rs2 = ID_EX.rs2;
    
    // Control signals for write back
    char RegWrite = ID_EX.Ctrl_WB_RegWrite;
    char MemtoReg = ID_EX.Ctrl_WB_MemtoReg;
    char RegDst = ID_EX.Ctrl_EX_RegDst;
    
    // ALU calculate
    long long ALU_out = 0;
    switch (ALUOp) {
        case ALU_ADD:
            ALU_out = ALUsrc1 + ALUsrc2;
            break;
        case ALU_MUL:
            ALU_out = ALUsrc1 * ALUsrc2;
            break;
        case ALU_SUB:
            ALU_out = ALUsrc1 - ALUsrc2;
            break;
        case ALU_SLL:
            ALU_out = ALUsrc1 << ALUsrc2;
            break;
        case ALU_MULH: // TODO
            break;
        case ALU_SLT:
            ALU_out = (ALUsrc1 < ALUsrc2)? 1:0;
            break;
        case ALU_XOR:
            ALU_out = ALUsrc1 ^ ALUsrc2;
            break;
        case ALU_DIV:
            ALU_out = ALUsrc1 / ALUsrc2;
            break;
        case ALU_SRL:
            ALU_out = ALUsrc1 >> ALUsrc2;
            break;
        case ALU_SRA:
            ALU_out = ((signed long long)ALUsrc1) >> ALUsrc2;
            break;
        case ALU_OR:
            ALU_out = ALUsrc1 | ALUsrc2;
            break;
        case ALU_REM:
            ALU_out = ALUsrc1 % ALUsrc2;
            break;
        case ALU_AND:
            ALU_out = ALUsrc1 & ALUsrc2;
            break;
        default:;
    }

    // Implementation of SEQ
    // Write EX_MEM
    ex.PC = PC;
    ex.ALU_out = ALU_out;
    
    if (step_mode == true) {
        printf("DEBUG: ALU_out = %llx\n", ALU_out);
    }
    
    // Control signals for memory
    ex.Ctrl_M_Branch = Branch;
    ex.Ctrl_M_MemRead = MemRead;
    ex.Ctrl_M_MemWrite = MemWrite;
    ex.rs2 = rs2;
    
    // Control signals for write back
    ex.Ctrl_WB_RegWrite = RegWrite;
    ex.Ctrl_WB_MemtoReg = MemtoReg;
    ex.Ctrl_EX_RegDst = RegDst;

    if (step_mode == true) {
        printf("DEBUG: ALU_out = %llx\n", ALU_out);
    }

}

void EX_MEM_update(){
    // Implementation of SEQ
    // Write EX_MEM
    EX_MEM.PC = ex.PC;
    EX_MEM.ALU_out = ex.ALU_out;
    
    
    // Control signals for memory
    EX_MEM.Ctrl_M_Branch = ex.Ctrl_M_Branch;
    EX_MEM.Ctrl_M_MemRead = ex.Ctrl_M_MemRead;
    EX_MEM.Ctrl_M_MemWrite = ex.Ctrl_M_MemWrite;
    EX_MEM.rs2 = ex.rs2;
    
    // Control signals for write back
    EX_MEM.Ctrl_WB_RegWrite = ex.Ctrl_WB_RegWrite;
    EX_MEM.Ctrl_WB_MemtoReg = ex.Ctrl_WB_MemtoReg ;
    EX_MEM.Ctrl_EX_RegDst = ex.Ctrl_EX_RegDst;
}

void MEM() {
    // Read EX_MEM
    // unsigned long PC = EX_MEM.PC;
    REG ALU_out = EX_MEM.ALU_out;
    
    // Control signals for memory
    char Branch = EX_MEM.Ctrl_M_Branch;
    char MemRead = EX_MEM.Ctrl_M_MemRead;
    char MemWrite = EX_MEM.Ctrl_M_MemWrite;
    char AlignPC = EX_MEM.AlignPC;
    unsigned int rs2 = EX_MEM.rs2;
    
    // Control signals for write back
    char RegWrite = EX_MEM.Ctrl_WB_RegWrite;
    char MemtoReg = EX_MEM.Ctrl_WB_MemtoReg;
    char RegDst = EX_MEM.Ctrl_EX_RegDst;
    
    REG Mem_out = 0; // Value read from memory
    
    // Read / write memory
    if (MemRead == MEM_BYTE) {
        Mem_out = (REG) *((unsigned char *) &memory[ALU_out]);
    }
    else if (MemRead == MEM_HALF) {
        Mem_out = (REG) *((unsigned short *) &memory[ALU_out]);
    }
    else if (MemRead == MEM_WORD) {
        Mem_out = (REG) *((unsigned int *) &memory[ALU_out]);
    }
    else if (MemRead == MEM_DOUBLE) {
        Mem_out = *((unsigned long long *) &memory[ALU_out]);
    }
    
    if (MemWrite == MEM_BYTE) {
        *((unsigned char *) &memory[ALU_out]) = (reg[rs2] & 0xFF);
    }
    else if (MemWrite == MEM_HALF) {
        *((unsigned short *) &memory[ALU_out]) = (reg[rs2] & 0xFFFF);
    }
    else if (MemWrite == MEM_WORD) {
        *((unsigned int *) &memory[ALU_out]) = (reg[rs2] & 0xFFFFFFFF);
    }
    else if (MemWrite == MEM_DOUBLE) {
        *((unsigned long long *) &memory[ALU_out]) = reg[rs2];
    }
    
    // Write MEM_WB
    mem.ALU_out = ALU_out;
    mem.Mem_out = Mem_out;
    
    // Control signals fro write back
    mem.Ctrl_WB_RegWrite = RegWrite;
    mem.Ctrl_WB_MemtoReg = MemtoReg;
    mem.Ctrl_EX_RegDst = RegDst;
    
    // Update PC
    if (!PC_stall){
        if (Branch) { 
            PC = ALU_out;
            if (AlignPC) {
                PC &= ~0x1;
            }

            //control hazard
            ID_EX_reset();
            IF_ID_reset();
        }
        else {
        PC = PC + 4;
        }
    }
    PC_stall = 0;

    if (step_mode == true) {
        printf("DEBUG: Mem_out = %llx\n", Mem_out);
    }

}

void MEM_WB_update(){
    // Write MEM_WB
    MEM_WB.ALU_out = mem.ALU_out;
    MEM_WB.Mem_out = mem.Mem_out;
    
    // Control signals fro write back
    MEM_WB.Ctrl_WB_RegWrite = mem.Ctrl_WB_RegWrite;
    MEM_WB.Ctrl_WB_MemtoReg = mem.Ctrl_WB_MemtoReg;
    MEM_WB.Ctrl_EX_RegDst = mem.Ctrl_EX_RegDst;
    
}

void WB() {
    // Read MEM_WB
    REG ALU_out = MEM_WB.ALU_out;
    REG Mem_out = MEM_WB.Mem_out;
    unsigned char RegDst = MEM_WB.Ctrl_EX_RegDst;
    
    // Control signals for write back
    unsigned char RegWrite = MEM_WB.Ctrl_WB_RegWrite;
    unsigned char MemtoReg = MEM_WB.Ctrl_WB_MemtoReg;
    
    // Write reg
    if (RegWrite == 1 && RegDst != 0) {

        cycle_cnt++;
        
        if (MemtoReg == 0) { // write from ALU
            reg[RegDst] = ALU_out;
        }
        else if (MemtoReg == 1) { // write from memory
            reg[RegDst] = Mem_out;
        }
    }
}
