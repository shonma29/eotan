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
#include <unistd.h>

#define MAX_CWD (1024)
#define MSG_MEMORY "memory exhausted\n"


int main(int argc, char **argv) {
	size_t size = MAX_CWD;
	char *q = NULL;

	//TODO parse option -L (use environment variable PWD. BSD default)
	//TODO parse option -P (get absolute path. POSIX default)

	do {
		char *p = (char*)malloc(size);

		if (!p) {
			fputs(MSG_MEMORY, stderr);
			return EXIT_FAILURE;
		}

		q = getcwd(p, sizeof(size));

		if (q) {
			puts(q);

		} else
			size *= 2;

		free(p);

	} while (!q);

	return EXIT_SUCCESS;
}
