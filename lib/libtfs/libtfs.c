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
#include <stddef.h>
#include <unistd.h>
#include <fs/vfs.h>

static int block_read(block_device_t *dev, void *buf, int blockno);
static int block_write(block_device_t *dev, void *buf, int blockno);

void block_initialize(block_device_t *dev)
{
	dev->channel = 0;
	dev->block_size = 0;
	dev->read = block_read;
	dev->write = block_write;
}

static int block_read(block_device_t *dev, void *buf, int blockno)
{
	off_t offset = blockno * dev->block_size;
	size_t size;

	if (lseek(dev->channel, offset, SEEK_SET) != offset)
		return 0;

	size = dev->block_size;
	return (read(dev->channel, buf, size) == size)? 0:(-1);
}

static int block_write(block_device_t *dev, void *buf, int blockno)
{
	off_t offset = blockno * dev->block_size;
	size_t size;

	if (lseek(dev->channel, offset, SEEK_SET) != offset)
		return 0;

	size = dev->block_size;
	return (write(dev->channel, buf, size) == size)? 0:(-1);
}
