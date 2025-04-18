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
#include <mpufunc.h>
#include <nerve/kcall.h>
#include "interrupt.h"
#include "../../lib/libserv/libserv.h"

static int _set(T_CISR *pk_cisr, const int);


static int _set(T_CISR *pk_cisr, const int n)
{
	pk_cisr->exinf = n;
	pk_cisr->intno = n;

	ER_ID error_no = create_isr(pk_cisr);
	if (error_no < 0) {
		kcall->printk("create_isr %d failed %d\n", n, error_no);
		return error_no;
	}

	return 0;
}


int define_mpu_handlers(void (*default_handler)(VP_INT exinf),
		void (*page_fault_handler)(VP_INT exinf))
{
	int result = 0;
	T_CISR pk_cisr;
	pk_cisr.isratr = TA_HLNG,
	pk_cisr.isr = default_handler;

	for (INTNO i = int_division_error; i <= int_protection; i++)
		if (_set(&pk_cisr, i))
			result = -1;

	for (INTNO i = int_reserved_15; i <= int_reserved_31; i++) {
		if (_set(&pk_cisr, i))
			result = -1;
	}
	pk_cisr.isr = page_fault_handler;
	if (_set(&pk_cisr, int_page_fault))
		result = -1;

	return result;
}
