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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <ctype.h>

#define ERR (-1)
#define ERR_OK (0)
#define ERR_MEMORY (-2)

#define DEFAULT_BUF_SIZE 1024
#define SHIFT_BUF_SIZE 1
#define DEFAULT_ARRAY_SIZE 16
#define SHIFT_ARRAY_SIZE 1

static size_t bufsize = DEFAULT_BUF_SIZE >> 1;
static size_t arraysize = DEFAULT_ARRAY_SIZE >> 1;
static char *buf = NULL;
static char **array = NULL;

static void execute(char **env)
{
	pid_t pid = fork();

	/* child */
	if (pid == 0) {
		if (execve(array[0], array, env) == ERR) {
			printf("exec error %d\n", errno);
			exit(errno);
		}
	}

	/* error */
	else if (pid == ERR)
		printf("fork error %d\n", pid);

	/* parent */
	else {
		int status;

		if (waitpid(pid, &status, 0) != pid)
			printf("waitpid error %d\n", errno);
		else
			printf("waitpid success %d\n", status);
	}
}

static void get_buf()
{
	char *buf2 = (char*)malloc(sizeof(char) * (bufsize << SHIFT_BUF_SIZE));

	if (buf2) {
		if (buf) {
			memcpy(buf2, buf, bufsize);
			free(buf);
		}

		buf = buf2;
		bufsize <<= SHIFT_BUF_SIZE;
	}
	else {
		printf("memory exhausted\n");
		exit(ERR_MEMORY);
	}
}

static void get_array()
{
	char **array2 = (char**)malloc(sizeof(char*)
			* (arraysize << SHIFT_ARRAY_SIZE));

	if (array2) {
		if (array) {
			memcpy(array2, array, sizeof(char*) * arraysize);
			free(array);
		}

		array = array2;
		arraysize <<= SHIFT_ARRAY_SIZE;
	}
	else {
		printf("memory exhausted\n");
		exit(ERR_MEMORY);
	}
}

static void parse()
{
	size_t i = 0;
	size_t pos = 0;

	for (;;) {
		for (; (buf[i] > 0) && (buf[i] <= ' '); i++);
		if (!buf[i])
			break;

		array[pos++] = &buf[i];
		if (pos >= arraysize)
			get_array();

		for (; buf[i] > ' '; i++);
		if (!buf[i])
			break;

		buf[i++] = '\0';
	}

	array[pos] = NULL;
}

int main(int argc, char **argv, char **env)
{
	get_buf();
	get_array();

	for (;;) {
		size_t pos;

		printf("$ ");

		for (pos = 0;;) {
			char c = fgetc(stdin);

			putchar(c);

			if (c == '\n') {
				buf[pos] = '\0';
				break;

			} else if (c == 0x08) {
				if (pos)
					pos--;

			} else {
				buf[pos++] = c;

				if (pos >= bufsize)
					get_buf();
			}
		}

		parse();
		if (array[0]) {
			if (strcmp(array[0], "exit"))
				execute(env);
			else
				break;
		}
	}

	if (buf)
		free(buf);

	if (array)
		free(array);

	return ERR_OK;
}
