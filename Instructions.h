//
//  Instructions.h
//  RISCV-Simulator
//
//  Created by CMouse on 2017/10/27.
//  Copyright © 2017年 Peking University. All rights reserved.
//

/* add.c
 addi    sp,sp,-32
 sd    s0,24(sp)
 sw    zero,-20(s0)
 j    10208 <main+0x5c>
 lui    a5,0x11
 lw    a4,-20(s0)
 slli    a4,a4,0x2
 add    a5,a4,a5
 addiw    a5,a5,10
 sext.w    a4,a5
 li    a5,4
 ble    a4,a5,101c4 <main+0x18>
 mv    a0,a5
 ld    s0,24(sp)
 */

#ifndef Instructions_h
#define Instructions_h

#define OP_R 51

#define F3_ADD 0
#define F3_MUL 0
#define F3_SUB 0
#define F3_SLL 1
#define F3_MULH 1
#define F3_SLT 2
#define F3_XOR 4
#define F3_DIV 4
#define F3_SRL 5
#define F3_SRA 5
#define F3_OR 6
#define F3_REM 6
#define F3_AND 7

#define F7_ADD 0
#define F7_MUL 1
#define F7_SUB 0x20
#define F7_SLL 0
#define F7_MULH 1
#define F7_SLT 0
#define F7_XOR 0
#define F7_DIV 1
#define F7_SRL 0
#define F7_SRA 0x20
#define F7_OR 0
#define F7_REM 1
#define F7_AND 0

#define OP_L 3

#define F3_LB 0
#define F3_LH 1
#define F3_LW 2
#define F3_LD 3

#define OP_I 19

#define F3_ADDI 0
#define F3_SLLI 1
#define F3_SLTI 2
#define F3_XORI 4
#define F3_SRLI 5
#define F3_SRAI 5
#define F3_ORI 6
#define F3_ANDI 7

#define F7_SLLI 0
#define F7_SRLI 0
#define F7_SRAI 0x10

#define OP_IW 27
#define F3_ADDIW 0

// Jump and load register
#define OP_JALR 0x67
#define F3_JALR 0

#define OP_ECALL 0x73
#define F3_ECALL 0
#define F7_ECALL 0

#define OP_S 0x23

#define F3_SB 0
#define F3_SH 1
#define F3_SW 2
#define F3_SD 3

// Branch
#define OP_B 0x63
#define F3_BEQ 0
#define F3_BNE 1
#define F3_BLT 4
#define F3_BGE 5

#define OP_RW 59
#define F3_ADDW 0
#define F7_ADDW 0

#define OP_AUIPC 0x17
#define OP_LUI 0x37

// Jump and load
#define OP_JAL 0x6f

#endif /* Instructions_h */
