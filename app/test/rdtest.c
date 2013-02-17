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
#include <unistd.h>
#include "cunit.h"

#define BUF_SIZE (512)
#define MAX_SEEK (BUF_SIZE * 2 * 256)

static char buf1[BUF_SIZE];
static char buf2[BUF_SIZE];

char *testrd() {
	int fd = open("/dev/rd", O_RDWR);
	int i;

	assert_ne("open", -1, fd);

	for (i = 0; i < sizeof(buf1); i++) {
		buf1[i] = (i + 1) % 256;
		buf2[i] = 0;
	}
/*
	assert_eq("lseek", MAX_SEEK - 1, lseek(fd, MAX_SEEK - 1, SEEK_SET));
	assert_eq("write", -1, write(fd, buf1, 2));
	assert_eq("write", 1, write(fd, buf1, 1));
	assert_eq("write", -1, write(fd, buf1, 1));
	assert_eq("lseek", MAX_SEEK - 1, lseek(fd, MAX_SEEK - 1, SEEK_SET));
	assert_eq("read", -1, read(fd, buf2, 2));
	assert_eq("read", 1, read(fd, buf2, 1));
	assert_eq("read", -1, read(fd, buf2, 1));
*/
	assert_eq("lseek", 0, lseek(fd, 0, SEEK_SET));
	assert_eq("write", sizeof(buf1), write(fd, buf1, sizeof(buf1)));
	assert_eq("lseek", 0, lseek(fd, 0, SEEK_SET));
	assert_eq("read", sizeof(buf2), read(fd, buf2, sizeof(buf2)));
	for (i = 0; i < sizeof(buf1); i++) {
		assert_eq("cmp", buf1[i], buf2[i]);
	}

	assert_eq("close", 0, close(fd));

	return NULL;
}

int main(int argc, char **argv)
{
	test(testrd);

	return 0;
}
