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
#include <major.h>
#include <dev/device.h>
#include "../../lib/libserv/libserv.h"
#include "monitor.h"

#ifdef USE_VESA
#include <vesa.h>
#else
#include <cga.h>
#endif

static vdriver_unit_t monitor = {
	{ NULL, NULL },
	MYNAME,
	&root
};

static vdriver_t driver_mine = {
	DEVICE_CLASS_CONSOLE,
	{ NULL, NULL },
	detach,
	create,
	NULL,
	open,
	close,
	read,
	write
};

static void console_initialize(void);


const vdriver_t *attach(system_info_t *info)
{
	list_initialize(&(driver_mine.units));
	console_initialize();
	list_append(&(driver_mine.units), &(monitor.bros));
	return &driver_mine;
}

static void console_initialize(void)
{
#ifdef USE_VESA
	cns = getVesaConsole(&root, &default_font);
#else
	cns = getCgaConsole(&root,
			(const uint16_t *) kern_p2v((void *) CGA_VRAM_ADDR));
	cns->erase(&root, EraseScreenEntire);
	cns->locate(&root, 0, 0);
#endif
}
