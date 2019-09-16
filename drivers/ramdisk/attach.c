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
#include <dev/device.h>
#include "../../lib/libserv/libserv.h"
#include "ramdisk.h"

static vdriver_unit_t initrd = {
	MYNAME,
	(const void *) &(ranges[0])
};
static vdriver_unit_t *units[] = {
	&initrd,
	NULL
};
static vdriver_t driver_mine = {
	DEVICE_CLASS_STORAGE,
	(const vdriver_unit_t **) units,
	attach,
	detach,
	open,
	close,
	read,
	write
};


const vdriver_t *attach(system_info_t *info)
{
	if (info->initrd.size) {
		log_info(MYNAME ": addr=%p size=%x\n",
				info->initrd.address, info->initrd.size);
		ranges[0] = info->initrd;
		return &driver_mine;
	}

	return NULL;
}
