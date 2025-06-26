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
#include <string.h>
#include <unistd.h>
#include <libc.h>
#include <sys/wait.h>
#include "_perror.h"

#define ERR (-1)

#define SLEEP_SECONDS (60)

extern void __malloc_initialize(void);

static void collect(void);
static void execute(char **, char **);


static void collect(void)
{
	for (int status; waitpid(-1, &status, WNOHANG) > 0;);
}

static void execute(char **array, char **env)
{
	pid_t pid = rfork(RFPROC | RFNOTEG);
	if (pid == ERR) {
		int error_no = errno;
		_put_error("failed to fork ");
		_put_error(strerror(error_no));
		_put_error("\n");
	} else if (!pid) {
		//TODO why is it needed?
		_put_error("open window\n");

		if (execve(array[0], array, env) < 0) {
			int error_no = errno;
			_put_error("failed to execve ");
			_put_error(strerror(error_no));
			_put_error("\n");
		}

		_exit(EXIT_FAILURE);
	}
}

void _main(int argc, char **argv, char **env)
{
	errno = 0;
	__malloc_initialize();

	char *array[] = { "/bin/nterm", NULL };
	char *envp[] = { NULL };
	execute(array, envp);

	for (;;) {
		collect();
		sleep(SLEEP_SECONDS);
	}
}
