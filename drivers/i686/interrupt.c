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
#include <mpu/config.h>
#include "func.h"
#include "sync.h"
#include "mpu/handler.h"
#include "mpu/interrupt.h"
#include "mpu/mpufunc.h"

static ER (*isr[MAX_IDT + 1])(void);

static ER dummy_handler(void);


static ER dummy_handler(void)
{
	return E_SYS;
}

ER interrupt_initialize(void)
{
	size_t i;

	printk("interrupt_initialize\n");

	// MPU exceptions
	for (i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
		isr[i] = context_mpu_handler;
		idt_set(i, handlers[i]);
	}

	isr[int_page_fault] = context_page_fault_handler;

	// hardware interruptions
	for (; i < sizeof(isr) / sizeof(isr[0]); i++)
		isr[i] = dummy_handler;

	return E_OK;
}

ER interrupt_bind(const INHNO inhno, const T_DINH *pk_dinh)
{
	if (inhno >= sizeof(isr) / sizeof(isr[0]))
		return E_PAR;

	printk("interrupt_bind[%d] 0x%x\n", inhno, pk_dinh->inthdr);
	isr[inhno] = (pk_dinh->inthdr)?
			((ER (*)(void))(pk_dinh->inthdr)):dummy_handler;

	return E_OK;
}

void interrupt(const UW edi, const UW esi, const UW ebp, const UW esp,
		const UW ebx, const UW edx, const UW ecx, const UW eax,
		const UW ds, const UW no, const UW err, const UW eip,
		const UW cs, const W eflags)
{
	sync_blocking++;

	if ((isr[no])())
		//TODO call fault when no error code
		fault_with_error(edi, esi, ebp, esp, ebx, edx,
				ecx, eax, ds, no, err, eip, cs, eflags);

	--sync_blocking;
	enter_critical();
}
