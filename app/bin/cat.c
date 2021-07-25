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
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include "_perror.h"

#define BUFSIZ (8192)

static int print(const int, const int, const char *);
static int process(const int, const char *);


static int print(const int out, const int in, const char *name)
{
	for (;;) {
		char buf[BUFSIZ];
		int len = read(in, buf, sizeof(buf));
		if (len == -1) {
			_perror(name);
			return EXIT_FAILURE;
		}

		if (len == 0)
			break;

		if (write(out, buf, len) == -1) {
			_perror(name);
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}

static int process(const int out, const char *name)
{
	do {
		int in = open(name, O_RDONLY);
		if (in == -1)
			break;

		int result = print(out, in, name);

		if (close(in))
			break;

		return result;
	} while (false);

	_perror(name);
	return EXIT_FAILURE;
}

void _main(int argc, char *argv[], char *envp[])
{
	int result = EXIT_SUCCESS;

	if (argc == 1)
		result = print(STDOUT_FILENO, STDIN_FILENO, "stdin");
	else
		for (int i = 1; i < argc; i++)
			result |= process(STDOUT_FILENO, argv[i]);

	_exit(result);
}
