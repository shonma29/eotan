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
#include <fcntl.h>
#include <ipc.h>
#include <services.h>
#include <stdio.h>
#include <unistd.h>
#include <fs/protocol.h>

#define BUF_SIZE (341 * 3)

static unsigned char buf[BUF_SIZE];

static void swap(const size_t);
static int putline(const unsigned int, const size_t, unsigned char *);
static int process(const int);


static void swap(const size_t size)
{
	unsigned char *p = buf;
	for (size_t i = size / 3; i > 0; i--) {
		unsigned char c = p[0];

		p[0] = p[2];
		p[2] = c;
		p += 3;
	}
}

static int putline(const unsigned int start, const size_t size,
		unsigned char *buf)
{
	fsmsg_t message;
	message.header.ident = IDENT;
	message.header.type = Twrite;
	message.Twrite.fid = 4;
	message.Twrite.offset = start;
	message.Twrite.count = size;
	swap(size);
	message.Twrite.data = (char *) buf;

	int err = ipc_call(PORT_CONSOLE, &message, MESSAGE_SIZE(Twrite));
	if (err < 0)
		printf("call error %d\n", err);

	return err;
}

static int process(const int fd)
{
	size_t width = 0;
	size_t height = 0;
	unsigned int c = 0;

	if (read(fd, buf, 3) != 3) {
		printf("read err[desc]\n");
		return (-1);
	}

	if (buf[0] != 'P' || buf[1] != '6' || buf[2] != '\n') {
		printf("bad desc\n");
		return (-2);
	}

	for (;;) {
		if (read(fd, buf, 1) != 1) {
			printf("read err[width]\n");
			return (-1);
		}

		if (buf[0] == ' ')
			break;

		if (buf[0] < '0' || buf[0] > '9') {
			printf("not num[width]\n");
			return (-2);
		}

		width = width * 10 + buf[0] - '0';
	}

	for (;;) {
		if (read(fd, buf, 1) != 1) {
			printf("read err[height]\n");
			return (-1);
		}

		if (buf[0] == '\n')
			break;

		if (buf[0] < '0' || buf[0] > '9') {
			printf("not num[height]\n");
			return (-2);
		}

		height = height * 10 + buf[0] - '0';
	}

	if (width == 0 || width >= 640 || height == 0 || height >= 480) {
		printf("bad size %d, %d\n", width, height);
		return (-2);
	}

	printf("size %d, %d\n", width, height);

	for (;;) {
		if (read(fd, buf, 1) != 1) {
			printf("read err[color]\n");
			return (-1);
		}

		if (buf[0] == '\n')
			break;

		if (buf[0] < '0' || buf[0] > '9') {
			printf("not num[color]\n");
			return (-2);
		}

		c = c * 10 + buf[0] - '0';
	}

	if (c != 255) {
		printf("bad color %d\n", c);
		return (-2);
	}

	unsigned int pos = 0;
	for (size_t i = 0; i < height; i++) {
		size_t rest = width * 3;
		unsigned int j = 0;

		while (rest > sizeof(buf)) {
			if (read(fd, buf, sizeof(buf)) != sizeof(buf)) {
				printf("read err[block]\n");
				return (-1);
			}

			if (putline(pos + j, sizeof(buf), buf) < 0)
				return (-3);

			j += sizeof(buf);
			rest -= sizeof(buf);
		}

		if (rest) {
			if (read(fd, buf, rest) != rest) {
				printf("read err[rest]\n");
				return (-1);
			}

			if (putline(pos + j, rest, buf) < 0)
				return (-3);
		}

		pos += 3072;
	}

	printf("done\n");
	return 0;
}

int main(int argc, char **argv)
{
	if (argc == 2) {
		int fd = open(argv[1], O_RDONLY);
		if (fd > 0) {
			process(fd);
			close(fd);
		} else
			printf("open err %d\n", fd);
	} else
		printf("arg err\n");

	return 0;
}
