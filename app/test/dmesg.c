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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc.h>

#undef FORCE_NEWLINE

#define MYNAME "dmesg"

#define MSG_BIND "failed to bind"
#define MSG_OPEN "failed to open"
#define MSG_READ "failed to read"
#define MSG_WRITE "failed to write"
#define DELIMITER ": "
#define NEWLINE "\n"

#define CHR_NEWLINE '\n'

#define OK (0)
#define NG (1)

static char kprint_buf[1024];

#define KPRINT_MAX (sizeof(kprint_buf) - 1)

static void pute(const char *);
static void puterror(const char *);
static int relay(const int in, const int);


static void pute(const char *str)
{
	write(STDERR_FILENO, str, strlen(str));
}

static void puterror(const char *message)
{
	pute(MYNAME);
	pute(DELIMITER);
	pute(message);
	pute(NEWLINE);
}

static int relay(const int out, const int in)
{
	ssize_t size = read(in, kprint_buf, KPRINT_MAX);
	if (size < 0) {
		puterror(MSG_READ);
		return NG;
	} else if (!size)
		return NG;
#ifdef FORCE_NEWLINE
	if (kprint_buf[size - 1] != CHR_NEWLINE)
		kprint_buf[size++] = CHR_NEWLINE;
#endif
	if (write(out, kprint_buf, size) != size) {
		puterror(MSG_WRITE);
		return NG;
	}

	return OK;
}

int main(int argc, char **argv)
{
	int result = bind("#c", "/mnt", MREPL);
	if (result < 0) {
		puterror(MSG_BIND);
		return EXIT_FAILURE;
	}

	int fd = open("/mnt/kprint", O_RDONLY);
	if (fd < 0) {
		puterror(MSG_OPEN);
		return EXIT_FAILURE;
	}

	while (!relay(STDOUT_FILENO, fd));

	return EXIT_SUCCESS;
}
