#ifndef __MPU_GATE_H__
#define __MPU_GATE_H__ 1
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

#include "core.h"

#define ATTR_SEGMENT 0x10
#define ATTR_EXISTS 0x80

enum {
	kern_code = 0x08,
	kern_data = 0x10,
	user_code = 0x18,
	user_data = 0x20,
	user_stack = 0x28,
	call_service = 0x30
} Selector;

enum {
	dpl_kern = 0,
/*	dpl_driver = 1,*/
	dpl_server = 2,
	dpl_user = 3
} Dpl;

enum GateType {
/*	reserved0 = 0,*/
	tss16 = 1,
	ldt = 2,
	busyTss16 = 3,
	callGate16 = 4,
	taskGate = 5,
	interruptGate16 = 6,
	trapGate16 = 7,
/*	reserved8 = 8,*/
	tss32 = 9,
/*	reserved10 = 10,*/
	busyTss32 = 11,
	callGate32 = 12,
	reserved13 = 13,
	interruptGate32 = 14,
	trapGate32 = 15
};

typedef struct {
	UH offsetLow;
	UH selector;
	UB copyCount;
	UB attr;
	UH offsetHigh;
} GateDescriptor;

#endif
