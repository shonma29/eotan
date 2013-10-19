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
#include <mpu/io.h>
#include "../../kernel/sync.h"
#include "../../kernel/mpu/mpufunc.h"
#include "archfunc.h"
#include "handler.h"
#include "8254.h"
#include "8259a.h"

static void pit_set(const UH n);


ER pit_initialize(const UW freq)
{
	UW half;
	UW n;
	T_DINH pk_dinh = {
		TA_HLNG,
		(FP)timer_service
	};

	if ((freq < PIT_MIN_FREQ)
			|| (freq > PIT_MAX_FREQ))
		return E_PAR;

	half = (freq << (8 - 1)) * 3;
	n = ((PIT_CLOCK_MUL3 << 8) + half) / (half << 1) + 1;

	enter_critical();
	idt_set(PIC_IR_VECTOR(ir_pit), handle32);
	interrupt_bind(PIC_IR_VECTOR(ir_pit), &pk_dinh);
	pit_set(n);
	pic_reset_mask(ir_pit);
	leave_critical();

	return E_OK;
}

static void pit_set(const UH n)
{
	outb(pit_std_control,
			PIT_SELECT_CNT0
			| PIT_ACCESS_WORD
			| PIT_MODE_RATE
			| PIT_COUNT_BINARY);
	outb(pit_std_counter0, n & 0xff);
	outb(pit_std_counter0, (n >> 8) & 0xff);
}
