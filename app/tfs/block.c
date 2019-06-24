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
#include <string.h>
#include <unistd.h>
#include "../../include/fs/vfs.h"
#include "libserv.h"

static void block_clear(block_device_t *, void *);
static int block_read(block_device_t *, void *, const int);
static int block_write(block_device_t *, void *, const int);
static int block_invalidate(block_device_t *, const int);

void block_initialize(block_device_t *dev)
{
	dev->channel = 0;
	dev->block_size = 0;
	dev->clear = block_clear;
	dev->read = block_read;
	dev->write = block_write;
	dev->invalidate = block_invalidate;
}

static void block_clear(block_device_t *dev, void *buf)
{
	memset(buf, 0, dev->block_size);
}

static int block_read(block_device_t *dev, void *buf, const int blockno)
{
	off_t offset = blockno * dev->block_size;
	size_t size;

	if (lseek(dev->channel, offset, SEEK_SET) != offset) {
		log_debug("block_read: seek(%d) error\n", blockno);
		return (-1);
	}

	size = dev->block_size;
	int result = read(dev->channel, buf, size);
	if (result < 0) {
		log_debug("block_read: read(%d) error %d\n", blockno, result);
		return -1;
	}

	//TODO until replace statfs
	if (result < size)
		memset(&(buf[result]), 0, size - result);

	return size;
}

static int block_write(block_device_t *dev, void *buf, const int blockno)
{
	off_t offset = blockno * dev->block_size;
	size_t size;

	if (lseek(dev->channel, offset, SEEK_SET) != offset) {
		log_debug("block_write: seek(%d) error\n", blockno);
		return (-1);
	}

	size = dev->block_size;
	int result = (write(dev->channel, buf, size) == size) ? 0 : (-1);
	if (result)
		log_debug("block_write: write(%d) error\n", blockno);

	return result;
}

static int block_invalidate(block_device_t *dev, const int blockno)
{
	return 0;
}
