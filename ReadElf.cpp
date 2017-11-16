//
//  ReadElf.cpp
//  RISCV-Simulator
//
//  Created by CMouse on 2017/10/27.
//  Copyright © 2017年 Peking University. All rights reserved.
//

#include "ReadElf.hpp"

FILE *elf = NULL;
Elf64_Ehdr elf64_ehdr; // ELF header
Elf64_Phdr elf64_phdr; // Program header
Elf64_Shdr elf64_shdr; // Section header
Elf64_Sym elf64_sym; // Symbol table

// .text segment
unsigned long tadr = 0; // offset in file
unsigned long tsize = 0; // size in file
unsigned long tvadr = 0; // address in memory
unsigned long tvsize = 0; // size in memory

// .data segment
unsigned long dadr = 0; // offset in file
unsigned long dsize = 0; // size in file
unsigned long dvadr = 0; // address in memory
unsigned long dvsize = 0; // size in memory

// NOTE: For this lab, we are only simulating user programs
//       So entry point is set to address of <main>
unsigned long entry = 0; // entry point
unsigned long entry_main = 0; // entry point for main
unsigned long size_main = 0; // size of main
unsigned long endpc = 0;
unsigned long endpc_main = 0;

unsigned long global_pointer = 0;
unsigned long result = 0;

// Program headers
unsigned long padr = 0; // offset of program header table
unsigned short psize = 0; // size of each entry
unsigned short pnum = 0; // number of entries

// Section headers
unsigned long sadr = 0; // offset of section header table
unsigned short ssize = 0; // size of each entry
unsigned short snum = 0; // number of entries

// Symbol table
unsigned long symadr = 0; // offset of symbol table
unsigned long symsize = 0; // size of each entry
unsigned long symnum = 0; // number of entries

unsigned short shstrndx = 0; // Section name string table index
unsigned long shstraddr = 0; // Section name string table offset
unsigned long shstrsize = 0; // Section name string table size

unsigned long straddr = 0; // String table offset
unsigned long strtotalsize = 0; // String table total size

bool open_file(const char * filename) {
    elf = fopen(filename, "r");
    if (elf != NULL) {
        return true;
    }
    return false;
}

void read_elf(const char * filename) {
    if (!open_file(filename)) {
        return;
    }
    
    read_elf_header();
    read_program_headers();
    read_section_headers();
    read_symbol_table();
    
    fclose(elf);
}

void read_elf_header() {
    // Load ELF Header
    fread(&elf64_ehdr, 1, sizeof(elf64_ehdr), elf);
    printf("ELF Header:\n");
    printf("Magic:\t\t\t\t\t");
    for (int i = 0; i < EI_NIDENT; i++) {
        printf("%02x ", elf64_ehdr.e_ident[i]);
    }
    printf("\n");
    
    printf("Class:\t\t\t\t\tELF64\n");
    printf("Data:\t\t\t\t\t2's complement, little endian\n");
    printf("Version:\t\t\t\t%u\n", *((unsigned int *) &elf64_ehdr.e_version));
    
    entry = *((unsigned long *) &elf64_ehdr.e_entry);
    printf("Entry point address:\t\t\t0x%lx\n", entry);
    
    padr = *((unsigned long *) &elf64_ehdr.e_phoff);
    printf("Start of program headers:\t\t%lu (bytes into file)\n", padr);
    sadr = *((unsigned long *) &elf64_ehdr.e_shoff);
    printf("Start of section headers:\t\t%lu (bytes into file)\n", sadr);
    
    printf("Flags:\n"); // TODO: 60x5, RVC, double-float ABI
    printf("Size of this header:\t\t\t%hu (bytes)\n", *((unsigned short *) &elf64_ehdr.e_ehsize));
    
    psize = *((unsigned short *) &elf64_ehdr.e_phentsize);
    printf("Size of program headers:\t\t%hu (bytes)\n", psize);
    pnum = *((unsigned short *) &elf64_ehdr.e_phnum);
    printf("Number of program headers:\t\t%hu\n", pnum);
    
    ssize = *((unsigned short *) &elf64_ehdr.e_shentsize);
    printf("Size of section headers:\t\t%hu (bytes)\n", ssize);
    snum = *((unsigned short *) &elf64_ehdr.e_shnum);
    printf("Number of section headers:\t\t%hu\n", snum);
    
    shstrndx = *((unsigned short *) &elf64_ehdr.e_shstrndx);
    printf("Section header string table index:\t%hu\n", shstrndx);
}

void read_program_headers() {
    printf("\n");
    printf("Entry point 0x%lx\n", entry);
    printf("There are %hu program headers, starting at offset %lu\n\n", pnum, padr);
    
    printf("Program Headers:\n");
    
    printf("Type\t");
    printf("Offset\t\t\t");
    printf("VirtAddr\t\t");
    printf("PhysAddr");
    printf("\n");
    
    printf("\t");
    printf("FileSiz\t\t\t");
    printf("MemSiz\t\t\t");
    printf("Flags\t");
    printf("Align");
    printf("\n");
    
    for (int i = 0; i < pnum; i++) {
        // Relocate file
        fseek(elf, padr + i * psize, SEEK_SET);
        // Load corresponding program header
        fread(&elf64_phdr, 1, sizeof(elf64_phdr), elf);
        
        printf("LOAD\t"); // TODO
        unsigned long offset = *((unsigned long *) &elf64_phdr.p_offset);
        printf("0x%016lx\t", offset);
        unsigned long vaddr = *((unsigned long *) &elf64_phdr.p_vaddr);
        printf("0x%016lx\t", vaddr);
        printf("0x%016lx", *((unsigned long *) &elf64_phdr.p_paddr));
        printf("\n");
        
        printf("\t");
        unsigned long filesz = *((unsigned long *) &elf64_phdr.p_filesz);
        printf("0x%016lx\t", filesz);
        unsigned long memsz = *((unsigned long *) &elf64_phdr.p_memsz);
        printf("0x%016lx\t", memsz);
        unsigned int flag = *((unsigned int *) &elf64_phdr.p_flags);
        printf("FLAGS\t"); // TODO
        printf("ALIGN"); // TODO
        printf("\n");
        
        // TODO
        if (flag & PF_W) { // .data segment
            dadr = offset;
            dsize = filesz;
            dvadr = vaddr;
            dvsize = memsz;
        }
        else {
            tadr = offset;
            tsize = filesz;
            tvadr = vaddr;
            tvsize = memsz;
        }
    }
}

void read_section_headers() {
    // First find Section Name String Table
    fseek(elf, sadr + shstrndx * ssize, SEEK_SET);
    fread(&elf64_shdr, 1, sizeof(elf64_shdr), elf);
    shstraddr = *((unsigned long *) &elf64_shdr.sh_offset);
    shstrsize = *((unsigned long *) &elf64_shdr.sh_size);
    // Malloc sufficient space for Section Name String Table
    char * shstr = (char *) malloc(shstrsize + 1);
    fseek(elf, shstraddr, SEEK_SET);
    fread(shstr, 1, shstrsize, elf);
    
    printf("\n");
    printf("There are %hu section headers, starting at offset %lu:\n\n", snum, sadr);
    printf("Section Headers:\n");
    
    printf("[Nr]\t");
    printf("Name\t\t\t");
    printf("Type\t\t\t");
    printf("Address\t\t\t");
    printf("Offset\t");
    printf("\n");
    
    printf("\t");
    printf("Size\t\t\t");
    printf("EntSize\t\t\t");
    printf("Flags\t");
    printf("Link\t");
    printf("Info\t");
    printf("Align\t");
    printf("\n");
    
    for (int i = 0; i < snum; i++) {
        // Relocate file
        fseek(elf, sadr + i * ssize, SEEK_SET);
        // Load corresponding section header
        fread(&elf64_shdr, 1, sizeof(elf64_shdr), elf);
        
        printf("[%2i]\t", i);
        
        unsigned int offset_to_name = *((unsigned int *) &elf64_shdr.sh_name);
        char * name = shstr + offset_to_name;
        printf("%16s\t", name);
        
        unsigned int type = *((unsigned int *) &elf64_shdr.sh_type);
        printf("%16u\t", type);
        
        printf("%016lx\t", *((unsigned long *) &elf64_shdr.sh_addr));
        
        unsigned long offset = *((unsigned long *) &elf64_shdr.sh_offset);
        printf("%08lx\t", offset);
        printf("\n");
        
        printf("\t");
        unsigned long size = *((unsigned long *) &elf64_shdr.sh_size);
        printf("%016lx\t", size);
        unsigned long entsize = *((unsigned long *) &elf64_shdr.sh_entsize);
        printf("%016lx\t", entsize);
        printf("Flags\t"); // TODO
        printf("Link\t"); // TODO
        printf("Info\t"); // TODO
        printf("Align\t"); // TODO
        printf("\n");
        
        if (strcmp(name, ".symtab") == 0) { // is symbol table
            symadr = offset;
            symsize = entsize;
            symnum = size / entsize;
        }
        else if (strcmp(name, ".strtab") == 0) { // is string table
            straddr = offset;
            strtotalsize = size;
        }
    }
    
    free(shstr);
}

void read_symbol_table() {
    if (symadr == 0) { // has not found symbol table
        return;
    }
    // First find String Table
    if (straddr == 0) { // has not found string table
        return;
    }
    // Malloc sufficient space for String Table
    char * strtable = (char *) malloc(strtotalsize + 1);
    fseek(elf, straddr, SEEK_SET);
    fread(strtable, 1, strtotalsize, elf);
    
    printf("\n");
    printf("Symbol table '.symtab' contains %lu entries:\n", symnum);
    printf("Num:\t");
    printf("Value\t\t\t");
    printf("Size\t");
    printf("Type\t");
    printf("Bind\t");
    printf("Vis\t");
    printf("Ndx\t");
    printf("Name");
    printf("\n");
    for (int i = 0; i < symnum; i++) {
        // Relocate file
        fseek(elf, symadr + i * symsize, SEEK_SET);
        // Load corresponding symbol
        fread(&elf64_sym, 1, sizeof(elf64_sym), elf);
        printf("%3i:\t", i);
        
        unsigned long value = *((unsigned long *) &elf64_sym.st_value);
        printf("%016lx\t", value);
        
        unsigned long size = *((unsigned long *) &elf64_sym.st_size);
        printf("%lu\t", size);
        
        printf("TYPE\t"); // TODO
        printf("BIND\t"); // TODO
        printf("VIS\t"); // TODO
        printf("%hu\t", *((unsigned short *) &elf64_sym.st_shndx)); // Ndx
        
        unsigned int offset_to_name = *((unsigned int *) &elf64_sym.st_name);
        char * name = strtable + offset_to_name;
        printf("%s", name);
        
        if (strcmp(name, "main") == 0) { // store address of main
            entry_main = value;
            size_main = size;
        }
        
        else if (strcmp(name, "__global_pointer$") == 0) {
            global_pointer = value;
        }
        
        else if (strcmp(name, "result") == 0) {
            result = value;
        }
        
        printf("\n");
    }
    
    free(strtable);
}

// Testing use only
// int main(int argc, char * argv[]) {
//     if (argc != 2) {
//         printf("Usage: ReadElf <filename>\n");
//         return 1;
//     }
//     read_elf(argv[1]);
//     return 0;
// }
