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
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include "../../include/elf.h"
#include "../../include/mpu/memory.h"
#include "m_out.h"

#define ERR_OK (0)
#define ERR_FILE (1)
#define ERR_DATA (2)

#define BAD_ELF "bad ELF\n"

#define DEBUG (0)

typedef struct {
	FILE *in;
	FILE *out;
	Elf32_Ehdr eHdr;
	//TODO save Phdr
	m_out_t header;
} buf_t;

static int read_header(buf_t *b);
static int extract(buf_t *);


static int read_header(buf_t *b)
{
	int result = ERR_OK;

	do {
		if (fread(&(b->eHdr), 1, sizeof(b->eHdr), b->in)
				!= sizeof(b->eHdr)) {
			fprintf(stderr, "read error(%d)\n", errno);
			result = ERR_FILE;
			break;
		}

		if (!isValidModule(&(b->eHdr))) {
			fprintf(stderr, BAD_ELF);
			result = ERR_DATA;
			break;
		}

		int offset = b->eHdr.e_phoff;
		uintptr_t file_head;
		uintptr_t section_tail = 0;

		for (int i = 0; i < b->eHdr.e_phnum;
				offset += sizeof(Elf32_Phdr), i++) {
			if (fseek(b->in, offset, SEEK_SET)) {
				fprintf(stderr, BAD_ELF);
				result = ERR_DATA;
				break;
			}

			Elf32_Phdr pHdr;
			if (fread(&pHdr, 1, sizeof(pHdr), b->in)
					!= sizeof(pHdr)) {
				fprintf(stderr, BAD_ELF);
				result = ERR_DATA;
				break;
			}

			if (pHdr.p_type != PT_LOAD)
				continue;

			if (!section_tail) {
				if ((uintptr_t) (pHdr.p_vaddr) & (PAGE_SIZE - 1)) {
					fprintf(stderr, BAD_ELF);
					result = ERR_DATA;
					break;
				}

				file_head = pHdr.p_vaddr;
				section_tail = pHdr.p_vaddr;
#if DEBUG
				fprintf(stderr, "file_head: %x\n", file_head);
#endif
			}
#if DEBUG
			if (section_tail != (uintptr_t) (pHdr.p_vaddr)) {
				size_t gap = (uintptr_t) (pHdr.p_vaddr)
						- section_tail;
				fprintf(stderr, "gap: %x\n", gap);
			}
#endif
			uintptr_t tail = pHdr.p_vaddr + pHdr.p_memsz;
			size_t segment_gap = pHdr.p_memsz - pHdr.p_filesz
					+ ((tail + (PAGE_SIZE - 1))
							& ~(PAGE_SIZE - 1))
					- tail;
#if DEBUG
			fprintf(stderr, "t=%d o=%x v=%x p=%x f=%x m=%x, g=%x\n",
					pHdr.p_type, pHdr.p_offset,
					pHdr.p_vaddr, pHdr.p_paddr,
					pHdr.p_filesz, pHdr.p_memsz,
					segment_gap);
#endif
			section_tail = pHdr.p_vaddr + pHdr.p_filesz
					+ segment_gap;
#if DEBUG
			fprintf(stderr, "section_tail: %x\n", section_tail);
#endif
		}

		b->header.address = (void *) file_head;
		b->header.length = section_tail - file_head;
		b->header.entry = (void *) (b->eHdr.e_entry);

		fprintf(stderr, "module: %p %x %p\n", b->header.address,
				b->header.length, b->header.entry);
	} while (false);

	return result;
}

//TODO optimize
static int extract(buf_t *b)
{
	int result = ERR_OK;
	int offset = b->eHdr.e_phoff;
	uintptr_t file_head;
	uintptr_t section_tail = 0;

	if (fwrite(&(b->header), sizeof(b->header), 1, b->out) != 1) {
		fprintf(stderr, "write error(%d)\n", errno);
		return ERR_FILE;
	}

	for (int i = 0; i < b->eHdr.e_phnum; offset += sizeof(Elf32_Phdr), i++) {
		if (fseek(b->in, offset, SEEK_SET)) {
			fprintf(stderr, BAD_ELF);
			result = ERR_DATA;
			break;
		}

		Elf32_Phdr pHdr;
		if (fread(&pHdr, 1, sizeof(pHdr), b->in) != sizeof(pHdr)) {
			fprintf(stderr, BAD_ELF);
			result = ERR_DATA;
			break;
		}

		if (pHdr.p_type != PT_LOAD)
			continue;

		if (fseek(b->in, pHdr.p_offset, SEEK_SET)) {
			fprintf(stderr, BAD_ELF);
			result = ERR_DATA;
			break;
		}

		if (!section_tail) {
			if ((uintptr_t) (pHdr.p_vaddr) & (PAGE_SIZE - 1)) {
				fprintf(stderr, BAD_ELF);
				result = ERR_DATA;
				break;
			}

			file_head = pHdr.p_vaddr;
			section_tail = pHdr.p_vaddr;
		}

		if (section_tail != (uintptr_t) (pHdr.p_vaddr)) {
			size_t gap = (uintptr_t) (pHdr.p_vaddr) - section_tail;
			for (int i = 0; i < gap; i++)
				if (fputc(0, b->out) == EOF) {
					fprintf(stderr, "write error(%d)\n",
							errno);
					return ERR_FILE;
				}
		}

		for (int i = 0; i < pHdr.p_filesz; i++) {
			int ch = fgetc(b->in);
			if (ch == EOF) {
				fprintf(stderr, BAD_ELF);
				return ERR_DATA;
			}

			if (fputc(ch, b->out) == EOF) {
				fprintf(stderr, "write error(%d)\n", errno);
				return ERR_FILE;
			}
		}

		uintptr_t tail = pHdr.p_vaddr + pHdr.p_memsz;
		size_t segment_gap = pHdr.p_memsz - pHdr.p_filesz
				+ ((tail + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1))
				- tail;

		for (int i = 0; i < segment_gap; i++)
			if (fputc(0, b->out) == EOF) {
				fprintf(stderr, "write error(%d)\n", errno);
				return ERR_FILE;
			}

		section_tail = pHdr.p_vaddr + pHdr.p_filesz + segment_gap;
	}

	return result;
}

int main(int argc, char **argv)
{
	int result;
	buf_t b;
	b.in = NULL;
	b.out = stdout;

	do {
		//TODO check argc
		b.in = fopen(argv[1], "rb");
		if (!(b.in)) {
			fprintf(stderr, "open error(%d)\n", errno);
			result = ERR_FILE;
			break;
		}

		result = read_header(&b);
		if (result)
			break;

		result = extract(&b);
	} while (false);

	if (b.in)
		if (fclose(b.in))
			fprintf(stderr, "close error(%d)\n", errno);

	return result;
}
