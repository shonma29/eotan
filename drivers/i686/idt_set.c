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
#include <mpu/desc.h>
#include "gate.h"
#include "mpufunc.h"

static void gate_set(GateDescriptor *p,
		UH selector, void (*handler)(void), UB attr);


void idt_set(UB no, void (*handler)(void))
{
	GateDescriptor *p = (GateDescriptor*)IDT_ADDR;

	//TODO error check
	gate_set(&(p[no]), kern_code, handler,
			ATTR_PRESENT | (dpl_kern << 5) | interruptGate32);
}

static void gate_set(GateDescriptor *p,
		UH selector, void (*handler)(void), UB attr)
{
	//TODO error check
	p->offsetLow = ((UW)handler) & 0xffff;
	p->selector = selector;
	p->copyCount = 0;
	p->attr = attr;
	p->offsetHigh = (((UW)handler) >> 16) & 0xffff;
}
