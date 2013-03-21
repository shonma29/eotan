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
#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__ 1

enum ExternalInterrupts {
	int_division_error = 0,
	int_debugger = 1,
	int_nmi = 2,
	int_break_point = 3,
	int_overflow = 4,
	int_out_of_bound = 5,
	int_invalid_operation_code = 6,
	int_no_coprocessor = 7,
	int_double_fault = 8,
	int_coprocessor_segment_overrun = 9,
	int_invalid_tss = 10,
	int_no_segment = 11,
	int_stack_segment_fault = 12,
	int_protection = 13,
	int_page_fault = 14,
	int_reserved_15 = 15,
	int_math_fault = 16,
	int_alignment_check = 17,
	int_machine_check = 18,
	int_simd = 19,
	int_reserved_20 = 20,
	int_reserved_21 = 21,
	int_reserved_22 = 22,
	int_reserved_23 = 23,
	int_reserved_24 = 24,
	int_reserved_25 = 25,
	int_reserved_26 = 26,
	int_reserved_27 = 27,
	int_reserved_28 = 28,
	int_reserved_29 = 29,
	int_reserved_30 = 30,
	int_reserved_31 = 31
};

#define MIN_USER_INTERRUPT 0x21
#define MAX_USER_INTERRUPT 0xff
#define NUM_OF_INTERRUPTS (MAX_USER_INTERRUPT + 1)

#endif
