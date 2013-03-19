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
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include "../include/boot/modules.h"

#define ERR_OK (0)
#define ERR_ARG (1)
#define ERR_FILE (2)

#define HELP "usage:\n\tappend moduleType filename blockSize\n"

static int append(FILE *out, const int moduleType, const char *fileName,
		const int blkSize);
static int dup(FILE *out, size_t size, FILE *in);
static int pad(FILE *out, size_t size);
static int appendEnd(FILE *out);


int main(int argc, char **argv)
{
	int type;

	switch (argc) {
	case 2:
		type = atoi(argv[1]);

		if (type == end)	return appendEnd(stdout);

		break;

	case 4:
		type = atoi(argv[1]);

		switch (type) {
		case kernel:
		case server:
		case user:
		case initrd:
			return append(stdout, type, argv[2], atoi(argv[3]));

		default:
			break;
		}

		break;

	default:
		break;
	}

	fprintf(stderr, HELP);
	return ERR_ARG;
}

static int append(FILE *out, const int moduleType, const char *fileName,
		const int blkSize)
{
	int ret = ERR_OK;
	FILE *fp = NULL;
	ModuleHeader h;

	do {
		struct stat buf;
		size_t size;

		/* get file size */
		if (stat(fileName, &buf)) {
			fprintf(stderr, "stat error(%d)\n", errno);
			ret = ERR_FILE;
			break;
		}

		h.type = moduleType;
		h.length = (buf.st_size + (blkSize - 1)) & ~(blkSize - 1);
		h.bytes = buf.st_size;
		h.zBytes = 0;

		if (fwrite(&h, 1, sizeof(h), out) != sizeof(h)) {
			fprintf(stderr, "write error(%d)\n", errno);
			ret = ERR_FILE;
			break;
		}

		fp = fopen(fileName, "rb");
		if (!fp) {
			fprintf(stderr, "open error(%d)\n", errno);
			ret = ERR_FILE;
			break;
		}

		ret = dup(out, h.bytes, fp);
		if (ret)	break;

		ret = pad(out, h.length - h.bytes);

	} while (0);

	if (fp) {
		if (fclose(fp)) {
			fprintf(stderr, "close error(%d)\n", errno);
		}
	}

	return ret;
}

static int dup(FILE *out, size_t size, FILE *in)
{
	for (; size > 0; size--) {
		int c = fgetc(in);

		if (c == EOF) {
			fprintf(stderr, "read error(%d)\n", errno);
			return ERR_FILE;
		}

		if (fputc(c, out) == EOF) {
			fprintf(stderr, "write error(%d)\n", errno);
			return ERR_FILE;
		}
	}

	return ERR_OK;
}

static int pad(FILE *out, size_t size)
{
	for (; size > 0; size--) {
		if (fputc(0, out) == EOF) {
			fprintf(stderr, "write error(%d)\n", errno);
			return ERR_FILE;
		}
	}

	return ERR_OK;
}

static int appendEnd(FILE *out)
{
	ModuleHeader h = { end, 0, 0, 0 };

	if (fwrite(&h, 1, sizeof(h), out) != sizeof(h)) {
		fprintf(stderr, "write error(%d)\n", errno);
		return ERR_FILE;
	}

	return ERR_OK;
}
