#ifndef _MPU__TUNNEL_REGISTERS_H_
#define _MPU__TUNNEL_REGISTERS_H_
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

static inline int _tunnel_registers(int flags, void *func)
{
	int result;

	__asm__ __volatile__ ( \
		"subl $524, %%esp\n\t" \
		"xorl %%edx, %%edx\n\t" \
		"movl %%esp, %%ecx\n\t" \
		"movb %%cl, %%dl\n\t" \
		"notb %%dl\n\t" \
		"incb %%dl\n\t" \
		"andb $0xf, %%dl\n\t" \
		"addl %%edx, %%ecx\n\t" \
		"fxsave (%%ecx)\n\t" \
		"movl 528(%%esp), %%edx\n\t" \
		"pushl %%edx\n\t" \
		"pushl %%ecx\n\t" \
		"pushl %%edi\n\t" \
		"pushl %%esi\n\t" \
		"pushl %%ebp\n\t" \
		"pushl %%ebx\n\t" \
		"call *%%eax\n\t" \
		"addl $548, %%esp\n\t" \
		:"=a" (result) \
		:"a" (func) \
		:);

	return result;
}

static inline int _tunnel_out(void)
{
	int result;

	__asm__ __volatile__ ( \
		"popl %%ebx\n\t" \
		"popl %%ebp\n\t" \
		"popl %%esi\n\t" \
		"popl %%edi\n\t" \
		"popl %%ecx\n\t" \
		"fxrstor (%%ecx)\n\t" \
		"addl $528, %%esp\n\t" \
		:"=a" (result) \
		: \
		:);

	return result;
}

#endif
