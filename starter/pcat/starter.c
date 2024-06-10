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
#include <features.h>
#include <stdarg.h>
#include <stdnoreturn.h>
#include <string.h>
#include <arch/archfunc.h>
#include <mpu/mpufunc.h>
#include <nerve/global.h>
#include <set/lf_queue.h>
#include <starter/modules.h>
#ifndef USE_UEFI
#include "vesa.h"
#endif
#include "starter.h"

#define KERNLOG_UNITS ((KERNEL_LOG_SIZE - sizeof(lfq_t)) \
		/ lfq_node_size(sizeof(int)))

static void _putc(const char);
#ifndef USE_UEFI
static void set_display(void);
#endif


noreturn void _main(void)
{
	pic_mask_all();
#ifndef USE_UEFI
	set_display();
#endif
	size_t num_of_pages = get_num_of_pages();
	num_of_pages = (num_of_pages > MAX_PAGES) ? MAX_PAGES : num_of_pages;

	paging_initialize(num_of_pages);
	lfq_initialize((volatile lfq_t *) KERNEL_LOG_ADDR,
			(void *) ((uintptr_t) KERNEL_LOG_ADDR + sizeof(lfq_t)),
			sizeof(int),
			KERNLOG_UNITS);
	printk("Starter has woken up.\n");

	mpu_initialize();
	memory_initialize(num_of_pages);

	Display *d = &(sysinfo->display);
	printk("display fb=%p width=%d height=%d bpl=%d bpp=%d\n",
			d->base, d->r.max.x, d->r.max.y, d->bpl, d->bpp);

	set_selector();
}

static void _putc(const char ch)
{
	int w = ch;
	while (lfq_enqueue((volatile lfq_t *) KERNEL_LOG_ADDR, &w) != QUEUE_OK) {
		int trash;
		lfq_dequeue((volatile lfq_t *) KERNEL_LOG_ADDR, &trash);
	}
}

void printk(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vnprintf(_putc, (char *) format, &ap);
}
#ifndef USE_UEFI
static void set_display(void)
{
	VesaInfo *v = (VesaInfo *) VESA_INFO_ADDR;
	Display *d = &(((system_info_t *) kern_v2p(sysinfo))->display);
	d->r.min.x = 0;
	d->r.min.y = 0;
	d->r.max.x = v->width;
	d->r.max.y = v->height;
	d->base = (void *) ((uintptr_t) (v->buffer_addr));
	d->bpl = v->bytes_per_line;
	d->bpp = sizeof(Color_Rgb);
}
#endif
