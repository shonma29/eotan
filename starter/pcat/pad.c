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

#define ERR_OK (0)
#define ERR_ARG (1)
#define ERR_FILE (2)

#define HELP "usage:\n\tpad filename size\n"

static int pad(const char *, const unsigned int);


int main(int argc, char **argv)
{
	if (argc == 3) {
		int block_size = atoi(argv[2]);
		if (block_size > 0)
			return pad(argv[1], block_size);
	}

	fprintf(stderr, HELP);
	return ERR_ARG;
}

static int pad(const char *file_name, const unsigned int block_size)
{
	int result = ERR_OK;
	FILE *fp = NULL;

	do {
		struct stat buf;
		if (stat(file_name, &buf)) {
			fprintf(stderr, "%s cannot stat(%d)\n",
					file_name, errno);
			result = ERR_FILE;
			break;
		}

		size_t gap = buf.st_size % block_size;
		if (!gap)
			break;

		fp = fopen(file_name, "a");
		if (!fp) {
			fprintf(stderr, "%s cannot open(%d)\n",
					file_name, errno);
			result = ERR_FILE;
			break;
		}

		for (gap = block_size - gap; gap > 0; gap--)
			if (fputc(0, fp) == EOF) {
				fprintf(stderr, "%s cannot write(%d)\n",
						file_name, errno);
				result = ERR_FILE;
				break;
			}
	} while (0);

	if (fp)
		if (fclose(fp))
			fprintf(stderr, "%s cannot close(%d)\n",
					file_name, errno);

	return result;
}
