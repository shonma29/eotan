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
#include <stdarg.h>
#include <stdnoreturn.h>
#include <string.h>
#include <starter/vesa.h>
#include <arch/archfunc.h>
#include <mpu/mpufunc.h>
#include <set/lf_queue.h>
#include <starter/modules.h>

#define KERNLOG_UNITS ((KERNEL_LOG_SIZE - sizeof(lfq_t)) \
		/ lfq_node_size(sizeof(int)))

extern void memory_initialize(void);

static void _putc(const char);
void printk(const char *, ...);


noreturn void _main(void)
{
	pic_mask_all();
	paging_initialize();
	lfq_initialize((volatile lfq_t *) KERNEL_LOG_ADDR,
			(void *) ((uintptr_t) KERNEL_LOG_ADDR + sizeof(lfq_t)),
			sizeof(int),
			KERNLOG_UNITS);
	printk("Starter has woken up.\n");

	mpu_initialize();
	memory_initialize();

	VesaInfo *v = (VesaInfo *) VESA_INFO_ADDR;
	printk("VESA mode=%x fb=%p width=%d height=%d bpl=%d bpp=%d\n",
			v->mode_attr, v->buffer_addr, v->width, v->height,
			v->bytes_per_line, v->bits_per_pixel);
	printk("VESA mm=%d, red=%d,%d green=%d,%d blue=%d,%d dcm=%d\n",
			v->memory_model, v->red_position, v->red_size,
			v->green_position, v->green_size,
			v->blue_position, v->blue_size, v->direct_color_mode);

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
