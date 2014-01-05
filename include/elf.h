#ifndef _ELF_H_
#define _ELF_H_

/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

#define EI_MAG0 (0)
#define EI_MAG1 (1)
#define EI_MAG2 (2)
#define EI_MAG3 (3)
#define EI_CLASS (4)
#define EI_DATA (5)
#define EI_VERSION (6)
#define EI_PAD (7)

#define EI_NIDENT (16)

#define ET_NONE (0)
#define ET_REL (1)
#define ET_EXEC (2)
#define ET_DYN (3)
#define ET_CORE (4)
#define ET_LOPROC 0xff00
#define ET_HIPROC 0xffff

#define EM_NONE (0)
#define EM_M32 (1)
#define EM_SPARC (2)
#define EM_386 (3)
#define EM_68K (4)
#define EM_88K (5)
#define EM_860 (7)
#define EM_MIPS (8)

#define EV_NONE (0)
#define EV_CURRENT (1)

#define ELFMAG0 0x7f
#define ELFMAG1 'E'
#define ELFMAG2 'L'
#define ELFMAG3 'F'

#define ELFCLASSNONE (0)
#define ELFCLASS32 (1)
#define ELFCLASS64 (2)

#define ELFDATANONE (0)
#define ELFDATA2LSB (1)
#define ELFDATA2MSB (2)

typedef unsigned int Elf32_Addr;
typedef unsigned int Elf32_Off;
typedef unsigned short Elf32_Section;
typedef unsigned short Elf32_Versym;
typedef unsigned char Elf_Byte;
typedef unsigned short Elf32_Half;
typedef int Elf32_Sword;
typedef unsigned int Elf32_Word;
typedef long long Elf32_Sxword;
typedef unsigned long long Elf32_Xword;

typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf32_Half e_type;
	Elf32_Half e_machine;
	Elf32_Word e_version;
	Elf32_Addr e_entry;
	Elf32_Off e_phoff;
	Elf32_Off e_shoff;
	Elf32_Word e_flags;
	Elf32_Half e_ehsize;
	Elf32_Half e_phentsize;
	Elf32_Half e_phnum;
	Elf32_Half e_shentsize;
	Elf32_Half e_shnum;
	Elf32_Half e_shstrndx;
} Elf32_Ehdr;

#define PT_NULL (0)
#define PT_LOAD (1)
#define PT_DYNAMIC (2)
#define PT_INTERP (3)
#define PT_NOTE (4)
#define PT_SHLIB (5)
#define PT_PHDR (6)
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#define PF_X (1)
#define PF_W (2)
#define PF_R (4)

typedef struct {
	Elf32_Word p_type;
	Elf32_Off p_offset;
	Elf32_Addr p_vaddr;
	Elf32_Addr p_paddr;
	Elf32_Word p_filesz;
	Elf32_Word p_memsz;
	Elf32_Word p_flags;
	Elf32_Word p_align;
} Elf32_Phdr;

#define SHN_UNDEF (0)
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_HIPROC 0xff1f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_HIRESERVE 0xffff

#define SHT_NULL (0)
#define SHT_PROGBITS (1)
#define SHT_SYMTAB (2)
#define SHT_STRTAB (3)
#define SHT_RELA (4)
#define SHT_HASH (5)
#define SHT_DYNAMIC (6)
#define SHT_NOTE (7)
#define SHT_NOBITS (8)
#define SHT_REL (9)
#define SHT_SHLIB (10)
#define SHT_DYNSYM (11)
#define SHT_LOPROC 0x70000000
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MASKPROC 0xf0000000

typedef struct {
	Elf32_Word sh_name;
	Elf32_Word sh_type;
	Elf32_Word sh_flags;
	Elf32_Addr sh_addr;
	Elf32_Off sh_offset;
	Elf32_Word sh_size;
	Elf32_Word sh_link;
	Elf32_Word sh_info;
	Elf32_Word sh_addralign;
	Elf32_Word sh_entsize;
} Elf32_Shdr;

extern int isValidModule(const Elf32_Ehdr *eHdr);

#endif
