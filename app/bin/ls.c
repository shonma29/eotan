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
#include <dirent.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/dirent.h>
#include <sys/stat.h>

#define MYNAME "ls"

#define MSG_OPEN "No such file or directory"
#define MSG_READ "failed to read"
#define MSG_CLOSE "failed to close"
#define MSG_STAT "failed to get status"
#define DELIMITER ": "
#define NEWLINE "\n"

#define SIZE_BUF 16348

#define OK (0)
#define NG (1)

static char buf[SIZE_BUF];

static void pute(char *str);
static void puterror(char *name, char *message);
static int exec(int out, char *name, int argc, int *out_count);


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

static int exec(int out, char *name, int argc, int *out_count) {
	struct stat stat;
	int result = OK;
	int in = open(name, O_RDONLY);

	if (in == -1) {
		puterror(name, MSG_OPEN);
		return NG;
	}

	if (fstat(in, &stat)) {
		close(in);
		puterror(name, MSG_STAT);
		return NG;
	}

	if (*out_count)
		write(out, NEWLINE, 1);

	if (stat.st_mode & S_IFDIR) {
		if (argc > 2) {
			write(out, name, strlen(name));
			write(out, DELIMITER, strlen(DELIMITER));
			write(out, NEWLINE, 1);
		}

		for (;;) {
			int len = getdents(in, buf,
					sizeof(buf) / sizeof(struct dirent));
			struct dirent *p;

			if (len < 0) {
				puterror(name, MSG_READ);
				result = NG;
				break;
			}

			if (len == 0)	break;

			p = (struct dirent*)buf; 
			while (len > 0) {
				if (p->d_name[0] != '.') {
					write(out, p->d_name, strlen(p->d_name));
					write(out, NEWLINE, 1);
				}

				len -= p->d_reclen;
				p = (struct dirent*)((size_t)p + p->d_reclen);
			}
		}
	} else {
		write(out, name, strlen(name));
		write(out, NEWLINE, 1);
	}

	if (close(in)) {
		puterror(name, MSG_CLOSE);
		result = NG;
	}

	*out_count += 1;

	return result;
}

int main(int argc, char **argv) {
	int i;
	int result = OK;
	int out_count = 0;

	if (argc == 1)
		result |= exec(STDOUT_FILENO, ".", argc, &out_count);
	else
		for (i = 1; i < argc; i++) {
			argv++;
			result |= exec(STDOUT_FILENO, *argv, argc, &out_count);
		}

	return result? EXIT_FAILURE:EXIT_SUCCESS;
}
