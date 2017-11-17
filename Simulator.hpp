//
//  Simulator.hpp
//  RISCV-Simulator
//
//  Created by CMouse on 2017/10/27.
//  Copyright © 2017年 Peking University. All rights reserved.
//

#ifndef Simulator_hpp
#define Simulator_hpp

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
using namespace std;

#define MAX 400000000

typedef unsigned long long REG;

// Memory
unsigned char memory[MAX] = {0};

// Register File
REG reg[32] = {0};

// PC
unsigned long PC = 0;

//CPI calculation
unsigned long long instruction_cnt = 0;
unsigned long long cycle_cnt = 0;

void load_memory(const char * filename);
void simulate();
void print_reg();
void print_mem();
void print_CPI();

void IF();
void ID();
void EX();
void MEM();
void WB();

void IF_ID_update();
void ID_EX_update();
void EX_MEM_update();
void MEM_WB_update();
void ID_EX_reset();
void IF_ID_reset();

// Get cnt bits starting from position s
unsigned int getbit(unsigned inst, int s, int cnt) {
    unsigned int mask = (1 << cnt) - 1;
    mask <<= s;
    return (inst & mask) >> s;
}

// Perform Sign Extension
unsigned long ext_signed(unsigned int src, unsigned char len) {
    unsigned int sign_bit = (1 << (len - 1)) & src;
    if (sign_bit == 0) {
        return src;
    }
    else {
        unsigned long mask = ~((1 << len) - 1);
        return mask | src;
    }
}

#endif /* Simulator_hpp */
