#ifndef _MPU_IO_H_
#define _MPU_IO_H_
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

static inline void outb(unsigned short port, unsigned char v) {
	__asm__ __volatile__ ( \
			"outb %b0, %w1\n\t" \
			: \
			:"a"(v), "d"(port));
}

static inline void outw(unsigned short port, unsigned short v) {
	__asm__ __volatile__ ( \
			"outw %w0, %w1\n\t" \
			: \
			:"a"(v), "d"(port));
}

static inline void outl(unsigned short port, unsigned int v) {
	__asm__ __volatile__ ( \
			"outl %0, %w1\n\t" \
			: \
			:"a"(v), "d"(port));
}

static inline unsigned char inb(unsigned short port) {
	unsigned char v;

	__asm__ __volatile__ ( \
			"inb %w1, %b0\n\t" \
			:"=a"(v) \
			:"d"(port));
	return v;
}

static inline unsigned short inw(unsigned short port) {
	unsigned short v;

	__asm__ __volatile__ ( \
			"inw %w1, %w0\n\t" \
			:"=a"(v) \
			:"d"(port));
	return v;
}

static inline unsigned int inl(unsigned short port) {
	unsigned int v;

	__asm__ __volatile__ ( \
			"inl %w1, %0\n\t" \
			:"=a"(v) \
			:"d"(port));
	return v;
}

static inline void di(void) {
	__asm__ __volatile__ ( \
			"cli\n\t");
}

static inline void ei(void) {
	__asm__ __volatile__ ( \
			"sti\n\t");
}

static inline void halt(void) {
	__asm__ __volatile__ ( \
			"hlt\n\t");
}

#endif
