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
#include <fcntl.h>
#include <sys/wait.h>
#include <ipc.h>
#include <services.h>
#include <pseudo_signal.h>
#include "_perror.h"

#define ERR (-1)

typedef struct {
	int files[3];
	bool background;
} ExecOptions;

static char shutdown_message[] = {
	's', 'h', 'u', 't', 'd', 'o', 'w', 'n', '\n'
};

extern void __malloc_initialize(void);

static void execute(char **, char **, const ExecOptions *);


static void execute(char **array, char **env, const ExecOptions *opts)
{
	for (int status; waitpid(-1, &status, WNOHANG) > 0;);

	pid_t pid = fork();
	if (pid == 0) {
		// child
		if (opts->background
				&& (opts->files[0] == -1))
			close(STDIN_FILENO);

		if (execve(array[0], array, env) == ERR) {
			_put_error(array[0]);
			_put_error(" exec error ");
			_put_error(strerror(errno));
			_put_error("\n");
		}

		for (;;);
	} else {
		if (pid == ERR) {
			// error
			_put_error("fork error ");
			_put_error(strerror(errno));
			_put_error("\n");
			for (;;);
		} else if (!opts->background) {
			// parent
			int status;
			if (waitpid(pid, &status, 0) == ERR) {
				_put_error("wait error ");
				_put_error(strerror(errno));
				_put_error("\n");
				for (;;);
			}
		}
	}
}

void _main(int argc, char **argv, char **env)
{
	_set_local_errno(0);
	// 'execve' uses 'malloc'
	__malloc_initialize();

	// STDIN
	open("#i/cons", O_RDONLY);
	// STDOUT
	open("#i/cons", O_WRONLY);
	// STDERR
	dup2(STDOUT_FILENO, STDERR_FILENO);

	char *array[] = { NULL, NULL };
	char *envp[] = { "COLUMNS=84", "LINES=29", NULL };
	ExecOptions opts = { { -1, -1, -1 }, false };

	array[0] = "/clock";
	opts.background = true;
	execute(array, envp, &opts);

	array[0] = "/bin/shell";
	opts.background = false;
	execute(array, envp, &opts);

	write(STDOUT_FILENO, shutdown_message, sizeof(shutdown_message));

	int signal = SIGNAL_SYNC;
	ipc_call(PORT_FS, &signal, sizeof(signal));
	for (;;);
}
