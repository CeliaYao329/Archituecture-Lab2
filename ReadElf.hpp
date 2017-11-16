//
//  ReadElf.hpp
//  RISCV-Simulator
//
//  Created by CMouse on 2017/10/27.
//  Copyright © 2017年 Peking University. All rights reserved.
//

#ifndef ReadElf_hpp
#define ReadElf_hpp

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    unsigned char b[8];
} int64;

typedef struct {
    unsigned char b[4];
} int32;

typedef struct {
    unsigned char b[2];
} int16;

typedef struct {
    unsigned char b;
} int8;

typedef int64 Elf64_Addr; // Unsigned program address
typedef int16 Elf64_Half; // Unsigned medium integer
typedef int64 Elf64_Off; // Unsigned file offset
typedef int32 Elf64_Sword; // Signed large integer
typedef int32 Elf64_Word; // Unsigned large integer
typedef int64 Elf64_Xword;
typedef int64 Elf64_Sxword;

// ELF Header
#define EI_NIDENT 16 // Number of identification bytes

typedef struct
{
    unsigned char e_ident[EI_NIDENT]; // ELF identification
    Elf64_Half e_type; // Object file type
    Elf64_Half e_machine; // Machine type
    Elf64_Word e_version; // Object file version
    Elf64_Addr e_entry; // Entry point address
    Elf64_Off e_phoff; // Program header offset
    Elf64_Off e_shoff; // Section header offset
    Elf64_Word e_flags; // Processor-specific flags
    Elf64_Half e_ehsize; // ELF header size
    Elf64_Half e_phentsize; // Size of program header entry
    Elf64_Half e_phnum; // Number of program header entries
    Elf64_Half e_shentsize; // Size of section header entry
    Elf64_Half e_shnum; // Number of section header entries
    Elf64_Half e_shstrndx; // Section name string table index
} Elf64_Ehdr;

// Program Header
#define PF_W 0x2 // Writable segment

typedef struct
{
    Elf64_Word p_type; // Type of segment
    Elf64_Word p_flags; // Segment attributes
    Elf64_Off p_offset; // Offset in file
    Elf64_Addr p_vaddr; // Virtual address in memory
    Elf64_Addr p_paddr; // Reserved
    Elf64_Xword p_filesz; // Size of segment in file
    Elf64_Xword p_memsz; // Size of segment in memory
    Elf64_Xword p_align; // Alignment of segment
} Elf64_Phdr;

// Section Header
#define SHT_SYMTAB 2

typedef struct
{
    Elf64_Word sh_name; // Section name
    Elf64_Word sh_type; // Section type
    Elf64_Xword sh_flags; // Section attributes
    Elf64_Addr sh_addr; // Virtual address in memory
    Elf64_Off sh_offset; // Offset in file
    Elf64_Xword sh_size; // Size of section
    Elf64_Word sh_link; // Link to other section
    Elf64_Word sh_info; // Miscellaneous information
    Elf64_Xword sh_addralign; // Address alignment boundary
    Elf64_Xword sh_entsize; // Size of entries, if section has table
} Elf64_Shdr;

// Symbol Table Entry
typedef struct
{
    Elf64_Word st_name; // Symbol name
    unsigned char st_info; // Type and binding attributes
    unsigned char st_other; // Reserved
    Elf64_Half st_shndx; // Section table index
    Elf64_Addr st_value; // Symbol value
    Elf64_Xword st_size; // Size of object
} Elf64_Sym;

void read_elf_header();
void read_program_headers();
void read_section_headers();
void read_symbol_table();

#endif /* ReadElf_hpp */
