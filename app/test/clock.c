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
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc.h>


int main(int argc, char **argv)
{
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	if (bind("#i", "/dev", MREPL) < 0)
		return EXIT_FAILURE;

	if (open("/dev/cons", O_RDONLY) < 0)
		return EXIT_FAILURE;

	if (open("/dev/cons", O_WRONLY) < 0)
		return EXIT_FAILURE;

	if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0)
		return EXIT_FAILURE;

	do {
		time_t t = time(NULL);
		if (t == -1)
			break;//TODO show error

		struct tm tm;
		if (!gmtime_r(&t, &tm))
			break;//TODO show error

		char buf[256];//TODO ugly
		size_t len = strftime(buf, sizeof(buf),
				"\x1b[1K\x1b[H%a %b %d %H:%M", &tm);
		if (!len)
			break;//TODO show error

		len++;
		if (write(STDOUT_FILENO, buf, len) != len)
			break;//TODO show error

		if (tm.tm_sec < 60)
			sleep(60 - tm.tm_sec);
	} while (true);

	_exit(EXIT_FAILURE);
}
