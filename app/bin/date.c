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
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


void _main(int argc, char *argv[], char *envp[])
{
	//TODO support options
	//TODO support parameter

	do {
		time_t t = time(NULL);
		if (t == -1)
			break;

		struct tm tm;
		if (!gmtime_r(&t, &tm))
			//TODO show error, while it does not ocuur
			break;

		char buf[256];
		size_t len = strftime(buf, sizeof(buf),
				"%a %b %d %H:%M:%S %Z %Y\n", &tm);
		if (!len)
			break;

		write(STDOUT_FILENO, buf, len);
		_exit(EXIT_SUCCESS);
	} while (false);

	_exit(EXIT_FAILURE);
}
