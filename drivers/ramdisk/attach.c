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
#include <device.h>
#include <major.h>
#include <string.h>
#include <boot/initrd.h>
#include <nerve/global.h>
#include "../../lib/libserv/libserv.h"
#include "ramdisk.h"

static vdriver_t driver_mine = {
	get_device_id(DEVICE_MAJOR_RAMDISK, 0),
	(unsigned char*)MYNAME,
	INITRD_SIZE,
	detach,
	open,
	close,
	read,
	write
};


vdriver_t *attach(int exinf)
{
	system_info_t *info = (system_info_t*)SYSTEM_INFO_ADDR;

	if (info->initrd.size > 0) {
		if (info->initrd.size <= driver_mine.size) {
			log_info(MYNAME ": initrd start=%p size=%x\n",
					info->initrd.start, info->initrd.size);
			ranges[0].start = (void*)INITRD_ADDR;
			ranges[0].size = (size_t)INITRD_SIZE;

			if (decode(info))
				log_err(MYNAME ": broken initrd\n");
			else
				return &driver_mine;
		} else
			log_err(MYNAME ": too large initrd (%x)\n",
				info->initrd.size);
	}

	return NULL;
}
