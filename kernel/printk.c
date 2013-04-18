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
#include <stdarg.h>
#include <string.h>
#include <set/ring.h>
#include "setting.h"
#include "sync.h"

#define DEBUG 0

#if DEBUG
#include <cga.h>

static CGA_Console *cns;
static int initialized;
#endif

static UB buf[RING_MAX_LEN + 1];
static size_t len;

static void _putc(char ch);


int printk(const char *format, ...) {
	va_list ap;

	va_start(ap, format);
#if DEBUG
	if (!initialized){
		initialized = 1;
		cns = getConsole((const UH*)kern_p2v((void*)CGA_VRAM_ADDR));
		cns->cls();
		cns->locate(0, 0);
	}
#endif
	enter_critical();
	len = 0;
	vnprintf(_putc, (char*)format, ap);
	ring_put((ring_t*)KERNEL_LOG_ADDR, buf, len);
	leave_critical();

	return len;
}

static void _putc(char ch)
{
#if DEBUG
	cns->putc(ch);
#endif
	if (len >= RING_MAX_LEN)
		return;

	buf[len++] = ch;
}
