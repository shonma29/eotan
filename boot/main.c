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
#include <cga.h>
#include <stdarg.h>
#include <string.h>
#include "boot.h"

#define INT_LEN (4)

#define BIOS_CURSOR_COL 0x0450
#define BIOS_CURSOR_ROW 0x0451
#define CGA_VRAM_ADDR 0x000b8000

static CGA_Console *cns;

static void console_initialize();
static int printk(const char *fmt, ...);

void _main()
{
	ModuleHeader *h = (ModuleHeader*)MODULES_ADDR;

	console_initialize();

	for (; h->type != end;) {
		unsigned int k;

		printk("%d %d %d %d\n",
				h->type, h->length, h->bytes, h->zBytes);
		k = (unsigned int)h + sizeof(*h) + h->length;
		h = (ModuleHeader*)k;
	}

	for(;;);
}

static void console_initialize()
{
	unsigned char *x = (unsigned char*)BIOS_CURSOR_COL;
	unsigned char *y = (unsigned char*)BIOS_CURSOR_ROW;

	cns = getConsole((const unsigned short*)CGA_VRAM_ADDR);
	cns->locate(*x, *y);
	printk("Starter has woken up.\n");
}

static int printk(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	return vnprintf((void*)(cns->putc), (char*)fmt, ap);
};

