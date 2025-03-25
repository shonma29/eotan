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
#include <console.h>
#include <services.h>
#include <nerve/kcall.h>
#include <set/lf_queue.h>
#include <libserv.h>
#include "monitor.h"

#define KERNLOG_QUEUE ((volatile lfq_t *) KERNEL_LOG_ADDR)

static Screen screen1;
static Console *cns;

static void initialize(void);
static unsigned int sleep(unsigned int);
static size_t lfcopy(char *, volatile lfq_t*, const size_t);


static void initialize(void)
{
	cns = getConsole(&screen1, &default_font);
	screen1.width /= 2;
	screen1.height = screen1.height - 20;
	screen1.chr_width = screen1.width / screen1.font.width;
	screen1.chr_height = screen1.height / screen1.font.height;
	screen1.base += 20 * screen1.bpl + screen1.width * 3;
	screen1.p = (uint8_t *) (screen1.base);
	screen1.fgcolor.rgb.b = 31;
	screen1.fgcolor.rgb.g = 223;
	screen1.fgcolor.rgb.r = 0;
	screen1.bgcolor.rgb.b = 0;
	screen1.bgcolor.rgb.g = 31;
	screen1.bgcolor.rgb.r = 0;
	cns->erase(&screen1, EraseScreenEntire);
	cns->locate(&screen1, 0, 0);
}

//TODO extract to libserv
static unsigned int sleep(unsigned int second)
{
	struct timespec t = { second, 0 };
	ER_UINT reply_size = kcall->ipc_call(PORT_TIMER, &t, sizeof(t));
	if (reply_size == sizeof(ER)) {
		ER *result = (ER *) &t;
		switch (*result) {
		case E_TMOUT:
			return 0;
		case E_PAR:
		case E_NOMEM:
			return second;
		default:
			break;
		}
	}

	return second;
}

static size_t lfcopy(char *outbuf, volatile lfq_t *q, const size_t size)
{
	size_t left;
	for (left = size; left > 0; left--) {
		wchar_t w;

		if (lfq_dequeue(q, &w) != QUEUE_OK)
			break;

		*outbuf = w & 0xff;
		outbuf++;
	}

	return (size - left);
}

void start(VP_INT exinf)
{
	initialize();
	kcall->printk("monitor: start\n");

	do {
		char outbuf[1024];
		for (size_t len;
				(len = lfcopy(outbuf, KERNLOG_QUEUE,
						sizeof(outbuf)));)
			for (int i = 0; i < len; i++)
				cns->putc(&screen1, outbuf[i]);
	} while (!sleep(1));

	kcall->thread_end_and_destroy();
}
