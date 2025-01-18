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
#include <archfunc.h>
#include <dev/device.h>
#include <nerve/global.h>
#include "ata.h"

static int detach(void);

static vdriver_unit_t primary_0 = {
	{ NULL, NULL },
	MYNAME,
	NULL
};
static vdriver_t driver_mine = {
	DEVICE_CLASS_STORAGE,
	{ NULL, NULL },
	detach,
	ata_read,
	ata_write
};


const vdriver_t *ata_attach(system_info_t *info)
{
	list_initialize(&(driver_mine.units));

	if (!ata_initialize()) {
		void *unit = ata_open(0);
		if (unit) {
			primary_0.unit = unit;
			list_append(&(driver_mine.units), &(primary_0.bros));
			return &driver_mine;
		}
	}

	return NULL;
}

static int detach(void)
{
	return 0;
}
