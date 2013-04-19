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
#include <elf.h>
#include <stdarg.h>
#include <string.h>
#include <boot/modules.h>
#include <boot/vesa.h>
#include <memory_map.h>
#include <mpu/mpufunc.h>

#define USE_VESA

#ifdef USE_VESA
#include <set/ring.h>
#include <setting.h>
#else
#include <cga.h>
#endif

#ifdef USE_VESA
static UB buf[RING_MAX_LEN + 1];
static size_t len;

static void _putc(char ch);
#else

static CGA_Console *cns;
#endif

static void console_initialize();
W printk(const B *fmt, ...);
static void kick(const ModuleHeader *h);


void _main(void)
{
	VesaInfo *v = (VesaInfo*)VESA_INFO_ADDR;

	console_initialize();

	paging_initialize();
	gdt_initialize();
	idt_initialize();	
	memory_initialize();

	printk("VESA mode=%x fb=%p width=%d height=%d bpl=%d bpp=%d\n",
			v->mode_attr, v->buffer_addr, v->width, v->height,
			v->bytes_per_line, v->bits_per_pixel);
	printk("VESA mm=%d, red=%d,%d green=%d,%d blue=%d,%d dcm=%d\n",
		v->memory_model, v->red_position, v->red_size, 
		v->green_position, v->green_size, 
		v->blue_position, v->blue_size, v->direct_color_mode);

	kick((ModuleHeader*)MODULES_ADDR);
	for (;;);
}

#ifdef USE_VESA
static void _putc(char ch)
{
	if (len >= RING_MAX_LEN)
		return;

	buf[len++] = ch;
}

static void console_initialize()
{
	ring_create(kern_v2p((void*)KERNEL_LOG_ADDR), KERNEL_LOG_SIZE);
}

W printk(const B *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	len = 0;
	vnprintf(_putc, (char*)fmt, ap);
	ring_put((ring_t*)KERNEL_LOG_ADDR, buf, len);

	return len;
};
#else
static void console_initialize()
{
	UB *x = (UB*)BIOS_CURSOR_COL;
	UB *y = (UB*)BIOS_CURSOR_ROW;

	cns = getConsole((const UH*)CGA_VRAM_ADDR);
	cns->locate(*x, *y);
	printk("Starter has woken up.\n");
}

W printk(const B *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	return vnprintf((void*)(cns->putc), (char*)fmt, ap);
};
#endif

static void kick(const ModuleHeader *h)
{
	while (h->type != mod_end) {
		UW addr;

		if (h->type == mod_kernel) {
			Elf32_Ehdr *eHdr = (Elf32_Ehdr*)&(h[1]);
			void (*entry)(void) = (void*)(eHdr->e_entry);

			entry();
			break;
		}

		addr = (unsigned int)h + sizeof(*h) + h->length;
		h = (ModuleHeader*)addr;
	}
}
