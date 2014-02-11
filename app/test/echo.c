#/*
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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char **argv) {
	int i;
	int out_count = 0;
	bool print_newline = true;

	// parse options
	for (i = 1; i < argc; i++) {
		char *p = argv[i];

		// option
		if (p[0] == '-') {
			switch (p[1]) {
			case 'n':
				print_newline = false;
				break;

			default:
				fprintf(stderr, "unknown option %s\n", p);
				return EXIT_FAILURE;
			}
		}
	}

	// parse arguments
	for (i = 1; i < argc; i++) {
		char *p = argv[i];

		// option
		if (p[0] == '-')
			continue;

		if (out_count > 0)
			fputc(' ', stdout);

		fputs(p, stdout);
		out_count++;
	}

	if (print_newline)
		fputc('\n', stdout);

	return EXIT_SUCCESS;
}
