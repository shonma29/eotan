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
#include <core.h>
#include <device.h>
#include <major.h>
#include <sys/errno.h>
#include "devfs.h"


int write_device(int channel, void *buf, off_t start, size_t length,
		size_t *rlength)
{
	if (start < 0)
		return EINVAL;

	device_info_t *info = device_find(channel);
	if (!info)
		return ENODEV;

	if (!(info->driver))
		return ENODEV;

	int result = info->driver->write(buf, get_channel(channel),
			start, length);
	*rlength = (result > 0)? result:0;

	return (result == length)? E_OK:E_SYS;
}

int read_device(int channel, void *buf, off_t start, size_t length,
		size_t *rlength)
{
	if (start < 0)
		return EINVAL;

	device_info_t *info = device_find(channel);
	if (!info)
		return ENODEV;

	if (!(info->driver))
		return ENODEV;

	int result = info->driver->read(buf, get_channel(channel),
			start, length);
	*rlength = (result > 0)? result:0;

	return (result == length)? length:E_SYS;
}
