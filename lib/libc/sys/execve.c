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
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mm/config.h>
#include <mpu/memory.h>
#include <sys/stat.h>
#include "sys.h"

#define STACK_TAIL USER_STACK_END_ADDR
#define LINE_SIZE (128)

static int count_args(char *const [], size_t *);
static void copy_args(void **, const unsigned int, char **,
		int, char *const []);
static char *copy_string(char *, char *);
static bool get_interpreter(char *, const char *name);
static bool can_access(const struct stat *, const int);


int execve(const char *name, char *const argv[], char *const envp[])
{
	char interpreter[LINE_SIZE];
	bool is_script = get_interpreter(interpreter, name);
	size_t size = 0;
	int argc = count_args(argv, &size);
	int org_argc = argc;
	if (is_script) {
		argc++;
		size += strlen(name) + 1;
	}

	int envc = count_args(envp, &size);
	int num_of_array = (3 + argc + 1 + envc + 1);

	size = roundUp(size, sizeof(int)) + num_of_array * sizeof(int)
			+ sizeof(thread_local_t);
	void **buf = (void **) malloc(size);
	if (!buf) {
		_set_local_errno(ENOMEM);
		return (-1);
	}

	buf[0] = (void *) argc;

	char *str = (char *) &(buf[num_of_array]);
	void **stack = (void *) (STACK_TAIL - size);

	// args
	buf[1] = (void *) &(stack[3]);

	void **index = &(buf[3]);
	unsigned int offset = (unsigned int) stack - (unsigned int) buf;
	if (is_script) {
		*index++ = str + offset;
		str = copy_string(str, (char *) name);
	}

	copy_args(index, offset, &str, org_argc, argv);

	// envp
	buf[2] = (void *) &(stack[3 + argc + 1]);
	copy_args(&(buf[3 + argc + 1]),
			(unsigned int) stack - (unsigned int) buf,
			&str, envc, envp);

	sys_args_t args = {
		syscall_exec,
		(int) (is_script ? interpreter : name),
		(int) buf,
		size
	};
	int result = _syscall(&args, sizeof(args));

	free(buf);
	return result;
}

static int count_args(char *const argv[], size_t *size)
{
	size_t len = 0;
	int argc = 0;

	if (argv)
		for (; argv[argc]; argc++)
			len += strlen(argv[argc]) + 1;

	*size += len;
	return argc;
}

static void copy_args(void **index, const unsigned int offset, char **ptr,
		int argc, char *const argv[])
{
	char *p = *ptr;
	for (int i = 0; i < argc; i++) {
		*index++ = p + offset;
		p = copy_string(p, argv[i]);
	}

	*ptr = p;
	*index++ = NULL;
}

static char *copy_string(char *dest, char *src)
{
	char c;
	do {
		c = *src++;
		*dest++ = c;
	} while (c);

	return dest;
}

static bool get_interpreter(char *interpreter, const char *name)
{
	int fd = open(name, O_RDONLY);
	if (fd == -1)
		return false;

	ssize_t len = read(fd, interpreter, LINE_SIZE);
	if (len == -1) {
		close(fd);
		return false;
	}

	if ((len < 4)
			|| (interpreter[0] != '#')
			|| (interpreter[1] != '!')) {
		close(fd);
		return false;
	}

	struct stat st;
	if (fstat(fd, &st) == -1) {
		close(fd);
		return false;
	}

	if (!can_access(&st, 1)) {
		close(fd);
		return false;
	}

	close(fd);

	char *w = interpreter;
	char *r = w + 2;
	for (len -= 2; len > 0; len--) {
		//TODO split arguments of interpreter
//		if (*r == '\n') {
		if (*r <= ' ') {
			*w = '\0';
			return true;
		}

		*w++ = *r++;
	}

	//TODO support recursive conversion
	return false;
}

//TODO unify to 'access'
static bool can_access(const struct stat *stat, const int mode)
{
	int bits = stat->st_mode & mode;

	if (stat->st_uid == getuid())
		bits |= (stat->st_mode >> 6) & mode;

	if (stat->st_gid == getgid())
		bits |= (stat->st_mode >> 3) & mode;

	return (bits == mode);
}
