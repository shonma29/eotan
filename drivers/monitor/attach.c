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
#include <vesa.h>
#include <dev/device.h>
#include "../../lib/libserv/libserv.h"
#include "font.h"
#include "monitor.h"

static vdriver_unit_t vesa = {
	{ NULL, NULL },
	MYNAME,
	window
};

static vdriver_t driver_mine = {
	DEVICE_CLASS_CONSOLE,
	{ NULL, NULL },
	detach,
	NULL,
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
#ifdef USE_VESA
	console_initialize();
	list_append(&(driver_mine.units), &(vesa.bros));
	return &driver_mine;
#else
	return NULL;
#endif
}

static void console_initialize(void)
{
	cns = getVesaConsole(&(window[0]), &default_font);
	Screen *s = &(window[0]);
	s->width /= 2;
	s->height /= 2;
	s->chr_width = s->width / s->font.width;
	s->chr_height = s->height / s->font.height;
	cns->cls(s);
	cns->locate(s, 0, 0);

	window[1] = window[0];
	s = &(window[1]);
	s->base += s->width * 3;
	s->p = (uint8_t *) (s->base);
	s->fgcolor.rgb.b = 31;
	s->fgcolor.rgb.g = 223;
	s->fgcolor.rgb.r = 0;
	s->bgcolor.rgb.b = 0;
	s->bgcolor.rgb.g = 31;
	s->bgcolor.rgb.r = 0;
	cns->cls(s);
	cns->locate(s, 0, 0);

	window[2] = window[0];
	s = &(window[2]);
	s->base += s->height * s->bpl;
	s->p = (uint8_t *) (s->base);
	s->fgcolor.rgb.b = 0;
	s->fgcolor.rgb.g = 127;
	s->fgcolor.rgb.r = 255;
	s->bgcolor.rgb.b = 0;
	s->bgcolor.rgb.g = 0;
	s->bgcolor.rgb.r = 31;
	cns->cls(s);
	cns->locate(s, 0, 0);

	window[3] = window[0];
	s = &(window[3]);
	s->base += s->height * s->bpl + s->width * 3;
	s->p = (uint8_t *) (s->base);
	s->fgcolor.rgb.b = 0x30;
	s->fgcolor.rgb.g = 0x30;
	s->fgcolor.rgb.r = 0x30;
	s->bgcolor.rgb.b = 0xfc;
	s->bgcolor.rgb.g = 0xfc;
	s->bgcolor.rgb.r = 0xfc;
	cns->cls(s);
	cns->locate(s, 0, 0);
}
