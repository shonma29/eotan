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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <libc.h>
#include <sys/wait.h>
#include "_perror.h"

#define ERR (-1)

#define SLEEP_SECONDS (60)

enum {
	BIND = 1,
	STDIN = 2,
	STDOUT = 3,
	STDERR = 4,
	EXEC = 5
};

extern void __malloc_initialize(void);

static void collect(void);
static void execute(char **, char **);


static void collect(void)
{
	for (int status; waitpid(-1, &status, WNOHANG) > 0;);
}

static void execute(char **array, char **env)
{
	pid_t pid = rfork(RFNOTEG);
	if (pid == ERR) {
		_put_error("fork error ");
		_put_error(strerror(errno));
		_put_error("\n");
	} else if (!pid) {
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);

		int result = 0;
		do {
			if (bind("#i", "/dev", MREPL) < 0) {
				result = BIND;
				break;
			}

			if (open("/dev/cons", O_RDONLY) < 0) {
				result = STDIN;
				break;
			}

			if (open("/dev/cons", O_WRONLY) < 0) {
				result = STDOUT;
				break;
			}

			if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
				result = STDERR;
				break;
			}

			if (execve(array[0], array, env) < 0) {
				result = EXEC;
				break;
			}
		} while (false);

		_exit(result);
	}
}

void _main(int argc, char **argv, char **env)
{
	errno = 0;
	__malloc_initialize();
#if 1
	// if only opened by init
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
#endif
	int result = 0;
	do {
		if (bind("#c", "/dev", MREPL) < 0) {
			result = BIND;
			break;
		}

		if (open("/dev/cons", O_RDONLY) < 0) {
			result = STDIN;
			break;
		}

		if (open("/dev/cons", O_WRONLY) < 0) {
			result = STDOUT;
			break;
		}

		if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0) {
			result = STDERR;
			break;
		}

		char *array[] = { "/bin/shell", NULL };
		char *envp[] = { "COLUMNS=84", "LINES=29", NULL };
		execute(array, envp);
		execute(array, envp);

		for (;;) {
			collect();
			sleep(SLEEP_SECONDS);
		}
	} while (false);

	_exit(result);
}
