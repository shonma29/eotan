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
#include <major.h>
#include <string.h>
#include <fs/vfs.h>
#include <nerve/kcall.h>
#include "../../lib/libserv/libserv.h"

static vdriver_t *driver;
static int minor;

static void block_clear(block_device_t *, void *);
static int block_read(block_device_t *, void *, const int);
static int block_write(block_device_t *, void *, const int);
static int block_invalidate(block_device_t *, const int);


void *malloc(size_t size)
{
	if (size > PAGE_SIZE)
		return NULL;

	return kcall->palloc();
}

void free(void *p)
{
	kcall->pfree(p);
}

int block_initialize(block_device_t *dev)
{
	dev->clear = block_clear;
	dev->read = block_read;
	dev->write = block_write;
	dev->invalidate = block_invalidate;

	driver = device_find(dev->channel);
	if (driver) {
		minor = get_channel(dev->channel);
		return 0;
	} else
		return ENODEV;
}

static void block_clear(block_device_t *dev, void *buf)
{
	memset(buf, 0, dev->block_size);
}

static int block_read(block_device_t *dev, void *buf, const int blockno)
{
	if (blockno < 0)
		return EINVAL;

	size_t len = dev->block_size;
	int result = driver->read(buf, minor, blockno * dev->block_size, len);
	return ((result == len) ? len : (-1));
}

static int block_write(block_device_t *dev, void *buf, const int blockno)
{
	if (blockno < 0)
		return EINVAL;

	size_t len = dev->block_size;
	int result = driver->write(buf, minor, blockno * dev->block_size, len);
	return ((result == len) ? 0 : (-1));
}

static int block_invalidate(block_device_t *dev, const int blockno)
{
	return 0;
}
