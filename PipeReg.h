//
//  PipeReg.h
//  RISCV-Simulator
//
//  Created by CMouse on 2017/10/27.
//  Copyright © 2017年 Peking University. All rights reserved.
//

#ifndef PipeReg_h
#define PipeReg_h

struct IFID {
    unsigned int inst;
    unsigned long PC;
} IF_ID;

#define EXTOP_ZERO 0
#define EXTOP_SIGN 1

struct IDEX {
    unsigned long PC;
    
    // Control signals for EX
    unsigned long ValA;
    unsigned long ValB;
    unsigned char Ctrl_EX_ALUOp;
    
    // Control signals for M
    unsigned char Ctrl_M_Branch;
    unsigned char Ctrl_M_MemRead;
    unsigned char Ctrl_M_MemWrite;
    unsigned char AlignPC;
    unsigned int rs2;
    
    // Control signals for WB
    unsigned char Ctrl_WB_RegWrite;
    unsigned char Ctrl_WB_MemtoReg;
    unsigned char Ctrl_EX_RegDst;
} ID_EX;

#define ALUSRC_REG 0
#define ALUSRC_IMM 1

// ALU operations
enum ALUOPS {
    ALU_NONE,
    ALU_ADD, ALU_MUL, ALU_SUB, ALU_SLL, ALU_MULH,
    ALU_SLT, ALU_XOR, ALU_DIV, ALU_SRL, ALU_SRA,
    ALU_OR, ALU_REM, ALU_AND};

struct EXMEM {
    unsigned long PC;
    REG ALU_out;
    
    unsigned char Ctrl_M_Branch;
    unsigned char Ctrl_M_MemWrite;
    unsigned char Ctrl_M_MemRead;
    unsigned char AlignPC;
    unsigned int rs2;
    
    unsigned char Ctrl_WB_RegWrite;
    unsigned char Ctrl_WB_MemtoReg;
    unsigned char Ctrl_EX_RegDst;
} EX_MEM;

#define MEM_NONE 0
#define MEM_BYTE 1
#define MEM_HALF 2
#define MEM_WORD 4
#define MEM_DOUBLE 8

struct MEMWB {
    REG ALU_out;
    REG Mem_out;
    
    unsigned char Ctrl_WB_RegWrite;
    unsigned char Ctrl_WB_MemtoReg;
    unsigned char Ctrl_EX_RegDst;
} MEM_WB;

#endif /* PipeReg_h */
