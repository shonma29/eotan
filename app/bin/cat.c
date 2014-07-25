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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MYNAME "cat"

#define MSG_OPEN "No such file or directory"
#define MSG_READ "failed to read"
#define MSG_CLOSE "failed to close"
#define DELIMITER ": "
#define NEWLINE "\n"

#define SIZE_BUF 2048

#define OK (0)
#define NG (1)

static void pute(char *str);
static void puterror(char *name, char *message);
static int exec(int out, char *name);

static void pute(char *str) {
	write(STDERR_FILENO, str, strlen(str));
}

static void puterror(char *name, char *message) {
	pute(MYNAME);
	pute(DELIMITER);
	pute(name);
	pute(DELIMITER);
	pute(message);
	pute(NEWLINE);
}

static int exec(int out, char *name) {
	int in = open(name, O_RDONLY);

	if (in == -1) {
		puterror(name, MSG_OPEN);
		return NG;
	}

	for (;;) {
		char buf[SIZE_BUF];
		int len = read(in, buf, sizeof(buf) / sizeof(char));

		if (len < 0) {
			puterror(name, MSG_READ);
			break;
		}

		if (len == 0)	break;

		write(out, buf, len);
	}

	if (close(in)) {
		puterror(name, MSG_CLOSE);
		return NG;
	}

	return OK;
}

int main(int argc, char **argv) {
	int i;
	int result = OK;

	for (i = 1; i < argc; i++) {
		argv++;
		result |= exec(STDOUT_FILENO, *argv);
	}

	return result;
}
