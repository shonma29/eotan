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
#include <stdio.h>
#include <stdlib.h>
#include "../../include/mpu/memory.h"
#include "../../include/starter/initrd.h"
#include "../../include/starter/modules.h"
//TODO move to include
#include "../../lib/librc/rangecoder.h"
#include "../../lib/librc/bit.h"
#include "m_out.h"

#define HELP "usage:\n\tappend outfile type infile [id]\nServers need their id.\n"

static FILE *in = stdin;
static FILE *out = NULL;

static int append(FILE *, const int, FILE *, const int);
static int pad(FILE *, const size_t);
static int appendEnd(FILE *);

static int bgetc(RangeCoder *);
static int bputc(unsigned char, RangeCoder *);
static int bputc_1st(unsigned char, RangeCoder *);
static int encode(FILE *, FILE *);


int main(int argc, char **argv)
{
	if (argc >= 3) {
		out = fopen(argv[1], "rb+");
		if (!out) {
			fprintf(stderr, "open error(%d)\n", errno);
			return ERR_FILE;
		}
	}

	int type;
	int result = ERR_ARG;
	switch (argc) {
	case 3:
		type = atoi(argv[2]);
		switch (type) {
		case mod_end:
			result = appendEnd(out);
			break;
		case mod_kernel:
		case mod_user:
		case mod_initrd:
			result = append(out, type, in, 0);
			break;
		default:
			break;
		}
		break;
	case 4:
		type = atoi(argv[2]);
		switch (type) {
		case mod_kthread:
		case mod_server:
			result = append(out, type, in, atoi(argv[3]));
			break;
		default:
			break;
		}

		break;
	default:
		break;
	}

	if (out)
		if (fclose(out))
			fprintf(stderr, "close error(%d)\n", errno);

	if (result == ERR_ARG)
		fprintf(stderr, HELP);

	return result;
}

static int append(FILE *out, const int type, FILE *in, const int arg)
{
	int result = ERR_OK;

	do {
		if (fseek(out, 0, SEEK_END)) {
			fprintf(stderr, "seek error(%d)\n", errno);
			result = ERR_FILE;
			break;
		}

		long start_pos = ftell(out);
		if (start_pos < 0) {
			fprintf(stderr, "ftell error(%d)\n", errno);
			result = ERR_FILE;
			break;
		}

		ModuleHeader h = { type, 0, 0, NULL, 0, NULL, arg };
		if (fwrite(&h, 1, sizeof(h), out) != sizeof(h)) {
			fprintf(stderr, "write error(%d)\n", errno);
			result = ERR_FILE;
			break;
		}

		if (type == mod_initrd) {
			h.address = (void *) INITRD_ADDR;
			h.pages = pages(INITRD_SIZE);
			fprintf(stderr, "size %x\n", INITRD_SIZE);
		} else {
			m_out_t m;
			if (fread(&m, 1, sizeof(m), in) != sizeof(m)) {
				fprintf(stderr, "read error(%d)\n", errno);
				result = ERR_FILE;
				break;
			} else {
				h.address = m.address;
				h.pages = pages(m.length);
				h.entry = m.entry;
			}
		}

		result = encode(out, in);
		if (result)
			break;

		long end_pos = ftell(out);
		if (end_pos < 0) {
			fprintf(stderr, "ftell error(%d)\n", errno);
			result = ERR_FILE;
			break;
		}

		size_t len = end_pos - start_pos - sizeof(h);
		h.length = (len + (MODULE_ALIGNMENT - 1))
				& ~(MODULE_ALIGNMENT - 1);
		h.bytes = len;
		result = pad(out, h.length - h.bytes);
		if (result)
			break;

		if (fseek(out, start_pos, SEEK_SET)) {
			fprintf(stderr, "seek error(%d)\n", errno);
			result = ERR_FILE;
			break;
		}

		if (fwrite(&h, 1, sizeof(h), out) != sizeof(h)) {
			fprintf(stderr, "write error(%d)\n", errno);
			result = ERR_FILE;
			break;
		}
	} while (false);

	return result;
}

static int pad(FILE *out, const size_t gap)
{
	for (size_t rest = gap; rest > 0; rest--)
		if (fputc(0, out) == EOF) {
			fprintf(stderr, "write error(%d)\n", errno);
			return ERR_FILE;
		}

	return ERR_OK;
}

static int appendEnd(FILE *out)
{
	if (fseek(out, 0, SEEK_END)) {
		fprintf(stderr, "seek error(%d)\n", errno);
		return ERR_FILE;
	}

	ModuleHeader h = { mod_end, 0, 0, NULL, 0, NULL, 0 };
	if (fwrite(&h, 1, sizeof(h), out) != sizeof(h)) {
		fprintf(stderr, "write error(%d)\n", errno);
		return ERR_FILE;
	}

	return ERR_OK;
}

static int bgetc(RangeCoder *rc)
{
	return fgetc(in);
}

static int bputc(unsigned char ch, RangeCoder *rc)
{
	return fputc(ch, out);
}

static int bputc_1st(unsigned char ch, RangeCoder *rc)
{
	rc->put = bputc;
	return 0;
}

static int encode(FILE *out, FILE *in)
{
	int result = ERR_OK;
	Frequency *freq = NULL;

	do {
		freq = (Frequency*) malloc(sizeof(Frequency));
		if (!freq) {
			fputs("cannot allocate memory\n", stderr);
			result = ERR_MEMORY;
			break;
		}

		RangeCoder rc;
		rc_initialize(&rc, bgetc, bputc_1st);
		frequency_initialize(freq);
		result = rc_encode(freq, &rc);
	} while (false);

	if (freq)
		free(freq);

	return result;
}
