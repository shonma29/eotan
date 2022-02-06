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
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include "_perror.h"

#define MSG_ARG "usage: chmod mode file ...\n"


void _main(int argc, char *argv[], char *envp[])
{
	int result = EXIT_SUCCESS;
	do {
		if (argc <= 2)
			break;

		//TODO suppert symbol
		errno = 0;

		char *end;
		mode_t mode = strtol(argv[1], &end, 8);

		if (*end)
			break;

		if (errno) {
			_perror("chmod");
			_exit(EXIT_FAILURE);
		}

		for (int i = 2; i < argc; i++)
			if (chmod(argv[i], mode)) {
				_perror(argv[i]);
				result = EXIT_FAILURE;
				break;
			}

		_exit(result);
	} while (false);

	_put_error(MSG_ARG);
	_exit(EXIT_FAILURE);
}