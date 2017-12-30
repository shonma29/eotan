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
#include <stdio.h>

#define EAX (0)
#define EBX (1)
#define ECX (2)
#define EDX (3)

static void cpuid(unsigned int param, int *buf)
{
	__asm__ __volatile__ (
		"cpuid\n\t" \
		"movl %%eax, (%1)\n\t" \
		"movl %%ebx, 4(%1)\n\t" \
		"movl %%ecx, 8(%1)\n\t" \
		"movl %%edx, 12(%1)\n\t" \
		: \
		:"a"(param),"S"(buf) \
		:"%ebx","%ecx","%edx");
}

static void putline(const unsigned int param, const int *buf)
{
	printf("%x: eax=%x, ebx=%x, ecx=%x, edx=%x\n",
		param, buf[0], buf[1], buf[2], buf[3]);
}

static void putchars(const char *buf, const size_t len)
{
	int i;

	for (i = 0; i < len; i++)
		printf("%c", buf[i] & 0xff);

	printf("\n");
}

int main(int argc, char **argv)
{
	unsigned int i;
	int max;
	int buf[4];

	cpuid(0, buf);
	putline(0, buf);

	max = buf[0];
	if (0 <= max) {
		putchars((char*)(&(buf[EBX])), sizeof(buf[EBX]));
		putchars((char*)(&(buf[EDX])), sizeof(buf[EDX]));
		putchars((char*)(&(buf[ECX])), sizeof(buf[ECX]));
	}

	for (i = 1; i <= max; i++) {
		cpuid(i, buf);
		putline(i, buf);
	}

	cpuid(0x80000000, buf);
	putline(0x80000000, buf);

	max = buf[0];
	for (i = 0x80000001; i <= max; i++) {
		cpuid(i, buf);
		putline(i, buf);

		if (0x80000002 <= i && i <= 0x80000004)
			putchars((char*)buf, sizeof(buf));
	}

	return 0;
}
