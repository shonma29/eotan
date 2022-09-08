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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#define ERR (-1)

typedef struct {
	int files[3];
	bool background;
} ExecOptions;

static void execute(char **, char **, const ExecOptions *);


static void execute(char **array, char **env, const ExecOptions *opts)
{
	for (int status; waitpid(-1, &status, WNOHANG) > 0;);

	pid_t pid = fork();
	if (pid == 0) {
		// child
		if (opts->background
				&& (opts->files[0] == -1))
			close(0);

		if (execve(array[0], array, env) == ERR) {
			if (errno == ENOENT)
				fprintf(stderr, "%s not found\n", array[0]);
			else
				fprintf(stderr, "exec error %d\n", errno);

			exit(errno);
		} else
			exit(ENOENT);
	} else {
		if (pid == ERR)
			// error
			fprintf(stderr, "fork error %d\n", pid);
		else if (!opts->background) {
			// parent
			int status;
			if (waitpid(pid, &status, 0) == -1)
				fprintf(stderr, "wait error %d\n", errno);
			else
				//TODO omit print, and set $status
				fprintf(stderr, "wait success %d\n", status);
		}
	}
}

int main(int argc, char **argv, char **env)
{
	char *array[] = { NULL, NULL };
	char *envp[] = { NULL };
	ExecOptions opts = { { -1, -1, -1 }, false };
#ifdef USE_VESA
	array[0] = "/clock";
	opts.background = true;
	execute(array, envp, &opts);
#endif
	array[0] = "/bin/shell";
	opts.background = false;
	for (;;)
		execute(array, envp, &opts);

	return EXIT_SUCCESS;
}
