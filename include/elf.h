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
#include <stdint.h>

#define EI_MAG0 (0)
#define EI_MAG1 (1)
#define EI_MAG2 (2)
#define EI_MAG3 (3)
#define EI_CLASS (4)
#define EI_DATA (5)
#define EI_VERSION (6)
#define EI_OSABI (7)
#define EI_ABIVERSION (8)
#define EI_PAD (9)

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
#define EM_SPARC32PLUS (18)
#define EM_SPARCV9 (43)
#define EM_AMD64 (62)

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

typedef uint32_t Elf32_Addr;
typedef uint32_t Elf32_Off;
typedef uint16_t Elf32_Section;
typedef uint32_t Elf32_Versym;
typedef uint8_t Elf_Byte;
typedef uint16_t Elf32_Half;
typedef int32_t Elf32_Sword;
typedef uint32_t Elf32_Word;
typedef uint64_t Elf32_Xword;
typedef int64_t Elf32_Sxword;

typedef uint64_t Elf64_Addr;
typedef uint64_t Elf64_Off;
typedef uint16_t Elf64_Half;
typedef uint16_t Elf64_Section;
typedef uint64_t Elf64_Versym;
typedef int32_t Elf64_Sword;
typedef uint32_t Elf64_Word;
typedef uint64_t Elf64_Xword;
typedef int64_t Elf64_Sxword;

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

typedef struct {
	unsigned char e_ident[EI_NIDENT];
	Elf64_Half e_type;
	Elf64_Half e_machine;
	Elf64_Word e_version;
	Elf64_Addr e_entry;
	Elf64_Off e_phoff;
	Elf64_Off e_shoff;
	Elf64_Word e_flags;
	Elf64_Half e_ehsize;
	Elf64_Half e_phentsize;
	Elf64_Half e_phnum;
	Elf64_Half e_shentsize;
	Elf64_Half e_shnum;
	Elf64_Half e_shstrndx;
} Elf64_Ehdr;

#define PT_NULL (0)
#define PT_LOAD (1)
#define PT_DYNAMIC (2)
#define PT_INTERP (3)
#define PT_NOTE (4)
#define PT_SHLIB (5)
#define PT_PHDR (6)
#define PT_TLS (7)
#define PT_LOOS 0x60000000
#define PT_SUNW_UNWIND 0x6464e550
#define PT_SUNW_EH_FRAME 0x6474e550
#define PT_LOSUNW 0x6ffffffa
#define PT_SUNWBSS 0x6ffffffa
#define PT_SUNWSTACK 0x6ffffffb
#define PT_SUNWDTRACE 0x6ffffffc
#define PT_SUNWCAP 0x6ffffffd
#define PT_HISUNW 0x6fffffff
#define PT_HIOS 0x6fffffff
#define PT_LOPROC 0x70000000
#define PT_HIPROC 0x7fffffff

#define PF_X (1)
#define PF_W (2)
#define PF_R (4)
#define PF_MASKPROC 0xf0000000

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

typedef struct {
	Elf64_Word p_type;
	Elf64_Word p_flags;
	Elf64_Off p_offset;
	Elf64_Addr p_vaddr;
	Elf64_Addr p_paddr;
	Elf64_Xword p_filesz;
	Elf64_Xword p_memsz;
	Elf64_Xword p_align;
} Elf64_Phdr;

#define SHN_UNDEF (0)
#define SHN_LORESERVE 0xff00
#define SHN_LOPROC 0xff00
#define SHN_BEFORE 0xff00
#define SHN_AFTER 0xff01
#define SHN_AMD64_LCOMMON 0xff02
#define SHN_HIPROC 0xff1f
#define SHN_LOOS 0xff20
#define SHN_LOSUNW 0xff3f
#define SHN_SUNW_IGNORE 0xff3f
#define SHN_HISUNW 0xff3f
#define SHN_HIOS 0xff3f
#define SHN_ABS 0xfff1
#define SHN_COMMON 0xfff2
#define SHN_XINDEX 0xffff
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
#define SHT_INIT_ARRAY (14)
#define SHT_FINI_ARRAY (15)
#define SHT_PREINIT_ARRAY (16)
#define SHT_GROUP (17)
#define SHT_SYMTAB_SHNDX (18)
#define SHT_LOOS 0x60000000
#define SHT_LOSUNW 0x6fffffef
#define SHT_SUNW_capchain 0x6fffffef
#define SHT_SUNW_capinfo 0x6ffffff0
#define SHT_SUNW_symsort 0x6ffffff1
#define SHT_SUNW_tlssort 0x6ffffff2
#define SHT_SUNW_LDYNSYM 0x6ffffff3
#define SHT_SUNW_dof 0x6ffffff4
#define SHT_SUNW_cap 0x6ffffff5
#define SHT_SUNW_SIGNATURE 0x6ffffff6
#define SHT_SUNW_ANNOTATE 0x6ffffff7
#define SHT_SUNW_DEBUGSTR 0x6ffffff8
#define SHT_SUNW_DEBUG 0x6ffffff9
#define SHT_SUNW_move 0x6ffffffa
#define SHT_SUNW_COMDAT 0x6ffffffb
#define SHT_SUNW_syminfo 0x6ffffffc
#define SHT_SUNW_verdef 0x6ffffffd
#define SHT_SUNW_verneed 0x6ffffffe
#define SHT_SUNW_versym 0x6fffffff
#define SHT_HISUNW 0x6fffffff
#define SHT_HIOS 0x6fffffff
#define SHT_LOPROC 0x70000000
#define SHT_SPARC_GOTDATA 0x70000000
#define SHT_AMD64_UNWIND 0x70000001
#define SHT_HIPROC 0x7fffffff
#define SHT_LOUSER 0x80000000
#define SHT_HIUSER 0xffffffff

#define SHF_WRITE 0x1
#define SHF_ALLOC 0x2
#define SHF_EXECINSTR 0x4
#define SHF_MERGE 0x10
#define SHF_STRINGS 0x20
#define SHF_INFO_LINK 0x40
#define SHF_LINK_ORDER 0x80
#define SHF_OS_NONCONFORMING 0x100
#define SHF_GROUP 0x200
#define SHF_TLS 0x400
#define SHF_MASKOS 0x0ff00000
#define SHF_AMD64_LARGE 0x10000000
#define SHF_ORDERED 0x40000000
#define SHF_EXCLUDE 0x80000000
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

typedef struct {
	Elf64_Word sh_name;
	Elf64_Word sh_type;
	Elf64_Xword sh_flags;
	Elf64_Addr sh_addr;
	Elf64_Off sh_offset;
	Elf64_Xword sh_size;
	Elf64_Word sh_link;
	Elf64_Word sh_info;
	Elf64_Xword sh_addralign;
	Elf64_Xword sh_entsize;
} Elf64_Shdr;

extern int isValidModule(const Elf32_Ehdr *eHdr);

#endif
