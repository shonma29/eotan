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
#include <func.h>
#include "mpufunc.h"


void fault(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW es, UW ds, UW no,
		UW eip, UW cs, UW eflags)
{
	printk("abort(%d). task=%d\n"
		" cs=%x eip=%x eflags=%x\n"
		" ds=%x es=%x\n"
		" eax=%x ebx=%x ecx=%x edx=%x\n"
		" edi=%x esi=%x ebp=%x esp=%x\n",
			no, thread_id(run_task), cs, eip, eflags, ds, es,
			eax, ebx, ecx, edx, edi, esi, ebp, esp);
	//TODO stop the thread
	panic("fault");
}

void fault_with_error(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW es, UW ds, UW no,
		UW err, UW eip, UW cs, UW eflags)
{
	printk("abort(%d). task=%d\n"
		" cs=%x eip=%x eflags=%x\n"
		" ds=%x es=%x error=%x\n"
		" eax=%x ebx=%x ecx=%x edx=%x\n"
		" edi=%x esi=%x ebp=%x esp=%x\n",
			no, thread_id(run_task), cs, eip, eflags, ds, es,
			err, eax, ebx, ecx, edx, edi, esi, ebp, esp);
	//TODO stop the thread
	panic("fault_with_error");
}
