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
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_QUOTIENT (UINT_MAX / 10)
#define MAX_REMAINDER (UINT_MAX % 10)

#define MSG_ARG "usage: sleep seconds\n"


int main(int argc, char **argv) {
	char *str = argv[1];

	//TODO check options

	if (argc != 2)
		;

	else if (*str) {
		unsigned int n = 0;

		for (;; str++) {
			unsigned int add;

			if (!*str) {
				sleep(n);
				return EXIT_SUCCESS;
			}

			if (!isdigit(*str))
				break;

			if (n > MAX_QUOTIENT)
				break;

			add = *str - '0';
			if ((n == MAX_QUOTIENT)
					&& (add > MAX_REMAINDER))
				break;

			n = n * 10 + add;
		}
	}

	fputs(MSG_ARG, stderr);
	return EXIT_FAILURE;
}
