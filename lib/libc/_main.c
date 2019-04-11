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
#include <unistd.h>

FILE __libc_files[FOPEN_MAX];
char **environ;

extern int main(int argc, char *argv[], char *envp[]);
static void __libc_initialize(void);


void _main(int argc, char *argv[], char *envp[])
{
	environ = envp;
	__libc_initialize();
	exit(main(argc, argv, envp));
}

static void __libc_initialize(void)
{
	FILE *p;

	p = stdin;
	p->mode = __FILE_MODE_READABLE;
	p->pos = 0;
	p->len = 0;
	p->fd = STDIN_FILENO;
	p->buf_size = 1;
	p->seek_pos = 0;

	for (int i = STDOUT_FILENO; i <= STDERR_FILENO; i++) {
		p = &(__libc_files[i]);
		p->mode = __FILE_MODE_WRITABLE;
		p->pos = 0;
		p->len = 0;
		p->fd = i;
		p->buf_size = 1;
		p->seek_pos = 0;
	}
}
