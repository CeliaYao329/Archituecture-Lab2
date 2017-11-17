//
//  PipeReg.h
//  RISCV-Simulator
//
//  Created by CMouse on 2017/10/27.
//  Copyright © 2017年 Peking University. All rights reserved.
//

#ifndef PipeReg_h
#define PipeReg_h

unsigned char PC_stall = 0;
unsigned char IF_ID_stall = 0;
unsigned char ID_EX_bubble = 0;
unsigned char IF_ID_bubble = 0;


struct IFID {
    unsigned int inst = 0;
    unsigned long PC = 0;
} iif, IF_ID;

#define EXTOP_ZERO 0
#define EXTOP_SIGN 1

// ALU operations
enum ALUOPS {
    ALU_NONE,
    ALU_ADD, ALU_MUL, ALU_SUB, ALU_SLL, ALU_MULH,
    ALU_SLT, ALU_XOR, ALU_DIV, ALU_SRL, ALU_SRA,
    ALU_OR, ALU_REM, ALU_AND};

struct IDEX {
    unsigned long PC = 0;
    
    // Control signals for EX
    unsigned long ValA = 0;
    unsigned long ValB = 0;
    unsigned char Ctrl_EX_ALUOp= ALU_NONE;
    
    // Control signals for M
    unsigned char Ctrl_M_Branch = 0;
    unsigned char Ctrl_M_MemRead = 0;
    unsigned char Ctrl_M_MemWrite = 0;
    unsigned char AlignPC = 0;
    unsigned int rs2 = 0;
    
    // Control signals for WB
    unsigned char Ctrl_WB_RegWrite = 0;
    unsigned char Ctrl_WB_MemtoReg = 0;
    unsigned char Ctrl_EX_RegDst = 0;
} id, ID_EX;

#define ALUSRC_REG 0
#define ALUSRC_IMM 1


struct EXMEM {
    unsigned long PC = 0;
    REG ALU_out = 0;
    
    unsigned char Ctrl_M_Branch = 0;
    unsigned char Ctrl_M_MemWrite = 0;
    unsigned char Ctrl_M_MemRead = 0;
    unsigned char AlignPC = 0;
    unsigned int rs2 = 0;
    
    unsigned char Ctrl_WB_RegWrite = 0;
    unsigned char Ctrl_WB_MemtoReg = 0;
    unsigned char Ctrl_EX_RegDst = 0;
} ex, EX_MEM;

#define MEM_NONE 0
#define MEM_BYTE 1
#define MEM_HALF 2
#define MEM_WORD 4
#define MEM_DOUBLE 8

struct MEMWB {
    REG ALU_out = 0;
    REG Mem_out = 0;
    
    unsigned char Ctrl_WB_RegWrite = 0;
    unsigned char Ctrl_WB_MemtoReg = 0;
    unsigned char Ctrl_EX_RegDst = 0;
} mem, MEM_WB;

#endif /* PipeReg_h */
