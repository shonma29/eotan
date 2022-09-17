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
#include <string.h>
#include <major.h>
#include <dev/device.h>
#include "../../lib/libserv/libserv.h"
#include "ramdisk.h"

static memory_range_t ranges[1];

static int detach(void);
static int read(char *, const int, const off_t, const size_t);
static int write(char *, const int, const off_t, const size_t);
//TODO return adequate error code
static bool check_channel(const int);
static bool check_param(const int, const off_t, const size_t);

static vdriver_unit_t initrd = {
	{ NULL, NULL },
	MYNAME,
	&(ranges[0])
};
static vdriver_t driver_mine = {
	DEVICE_CLASS_STORAGE,
	{ NULL, NULL },
	detach,
	read,
	write
};


const vdriver_t *ramdisk_attach(system_info_t *info)
{
	list_initialize(&(driver_mine.units));

	if (info->initrd.size) {
		log_info(MYNAME ": addr=%p size=%x\n",
				info->initrd.address, info->initrd.size);
		ranges[0] = info->initrd;
		list_append(&(driver_mine.units), &(initrd.bros));
		return &driver_mine;
	}

	return NULL;
}

static int detach(void)
{
	return 0;
}

static int read(char *outbuf, const int channel, const off_t offset,
		const size_t size)
{
	if (!check_param(channel, offset, size))
		return (-1);

	memory_range_t *r = (memory_range_t *) channel;
	memcpy(outbuf, &(((char *) (r->address))[offset]), size);
	return size;
}

static int write(char *inbuf, const int channel, const off_t offset,
		const size_t size)
{
	if (!check_param(channel, offset, size))
		return (-1);

	memory_range_t *r = (memory_range_t *) channel;
	memcpy(&(((char *) (r->address))[offset]), inbuf, size);
	return size;
}

static bool check_channel(const int channel)
{
	if (((memory_range_t *) channel) != &(ranges[0]))
		return false;

	return true;
}

static bool check_param(const int channel, const off_t offset,
		const size_t size)
{
	if (!check_channel(channel))
		return false;

	if (offset < 0)
		return false;

	memory_range_t *r = (memory_range_t *) channel;
	if (offset >= r->size)
		return false;

	size_t rest = r->size - offset;
	if (size > rest)
		return false;

	return true;
}
