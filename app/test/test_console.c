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
#include <stdio.h>
#include <unistd.h>
#include <libc.h>
#include "cunit.h"

#define BIND_PATH "/d"

static char *test_null(void);
static char *test_zero(void);


static char *test_null(void)
{
	char buf[16];
	int fd = open(BIND_PATH "/null", O_WRONLY);
	assert_eq("WRONLY open", 3, fd);
	errno = 0;
	assert_eq("WRONLY read", -1, read(fd, buf, 8));
	assert_eq("WRONLY read errno", EBADF, errno);
	assert_eq("WRONLY write", 8, write(fd, buf, 8));
	close(fd);

	fd = open(BIND_PATH "/null", O_RDONLY);
	assert_eq("RDONLY open", 3, fd);
	assert_eq("RDONLY read", 0, read(fd, buf, 8));
	errno = 0;
	assert_eq("RDONLY write", -1, write(fd, buf, 8));
	assert_eq("RDONLY write errno", EBADF, errno);
	close(fd);

	fd = open(BIND_PATH "/null", O_RDWR);
	assert_eq("RDWR open", 3, fd);
	assert_eq("RDWR read", 0, read(fd, buf, 8));
	assert_eq("RDWR write", 8, write(fd, buf, 8));
	close(fd);

	return NULL;
}

static char *test_zero(void)
{
	char buf[1025];
	for (int i = 0; i < sizeof(buf); i++)
		buf[i] = 'a';

	int fd = open(BIND_PATH "/zero", O_RDWR);
	assert_eq("open", 3, fd);
	assert_eq("read", sizeof(buf), read(fd, buf, sizeof(buf)));
	assert_eq("write", 8, write(fd, buf, 8));
	close(fd);

	for (int i = 0; i < sizeof(buf); i++)
		assert_eq("buf", 0, buf[i]);

	return NULL;
}

int main(int argc, char **argv)
{
	bind("#c", BIND_PATH, MREPL);
	test(test_null);
	test(test_zero);
	return 0;
}
