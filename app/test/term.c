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
#include <unistd.h>
#include <sys/wait.h>

#define ERR (-1)
#define ERR_PIPE (1)
#define ERR_EXEC (2)
#define ERR_FORK (3)
#define ERR_READ (4)
#define ERR_WRITE (5)

static char *array[] = { "/bin/shell", NULL };

static void collect(void);
static int tunnel(const int, const int);
static int execute(char **, char **, int fds[2]);


static void collect(void)
{
	for (int status; waitpid(-1, &status, WNOHANG) > 0;);
}

static int tunnel(const int out, const int in)
{
	for (;;) {
		char buf[BUFSIZ];
		ssize_t len = read(in, buf, sizeof(buf));
		if (!len)
			break;
		else if (len < 0) {
			fprintf(stderr, "failed to read(%d) %d %d\n",
					in, len, errno);
			return ERR_READ;
		}

		ssize_t result = write(out, buf, len);
		if (result != len) {
			fprintf(stderr, "failed to write(%d) %d %d\n",
					out, result, errno);
			return ERR_WRITE;
		}
	}

	return 0;
}

static int execute(char **array, char **env, int fds[2])
{
	pid_t child = fork();
	if (child == ERR) {
		fprintf(stderr, "failed to fork child %d\n", errno);
		return ERR_FORK;
	} else if (child) {
		close(fds[0]);

		pid_t out = fork();
		if (out == ERR) {
			fprintf(stderr, "failed to fork out %d\n", errno);
			return ERR_FORK;
		} else if (out) {
			int result = tunnel(fds[1], STDIN_FILENO);
			collect();
			return result;
		} else
			return tunnel(STDOUT_FILENO, fds[1]);
	} else {
		for (int i = STDIN_FILENO; i <= STDERR_FILENO; i++) {
			close(i);
			dup2(fds[0], i);
		}

		close(fds[0]);
		close(fds[1]);
		execve(array[0], array, env);
		fprintf(stderr, "failed to exec %d\n", errno);
		return ERR_EXEC;
	}
}

int main(int argc, char **argv, char **env)
{
	int fds[2];
	if (pipe(fds)) {
		fprintf(stderr, "failed to pipe %d\n", errno);
		return ERR_PIPE;
	}

	return execute(array, env, fds);
}
