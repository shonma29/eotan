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
#include "devfs.h"
#include "fs.h"


W write_device(ID device, B *buf, W start, W length, W *rlength)
{
	int result;
	device_info_t *info;

	if ((start < 0) || (length < 0))
		return EINVAL;

	info = device_find(device);
	if (!info)
		return ENODEV;

	if (!(info->driver))
		return ENODEV;

	result = info->driver->write((unsigned char*)buf, get_channel(device),
			(off_t)start, (size_t)length);
	*rlength = (result > 0)? result:0;

	return (result == length)? E_OK:E_SYS;
}

W read_device(ID device, B *buf, W start, W length, W *rlength)
{
	int result;
	device_info_t *info;

	if ((start < 0) || (length < 0))
		return EINVAL;

	info = device_find(device);
	if (!info)
		return ENODEV;

	if (!(info->driver))
		return ENODEV;

	result = info->driver->read((unsigned char*)buf, get_channel(device),
			(off_t)start, (size_t)length);
	*rlength = (result > 0)? result:0;

	return (result == length)? E_OK:E_SYS;
}
