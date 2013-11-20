#ifndef __MPU_EFLAGS_H__
#define __MPU_EFLAGS_H__ 1
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

#define EFLAGS_CARRY 0x00000001
#define EFLAGS_PARITY 0x00000004
#define EFLAGS_ADJUST 0x00000010
#define EFLAGS_ZERO 0x00000040
#define EFLAGS_SIGN 0x00000080
#define EFLAGS_TRAP 0x00000100
#define EFLAGS_INTERRUPT_ENABLE 0x00000200
#define EFLAGS_DIRECTION 0x00000400
#define EFLAGS_OVERFLOW 0x00000800
#define EFLAGS_IOPL_0 0x00000000
#define EFLAGS_IOPL_1 0x00001000
#define EFLAGS_IOPL_2 0x00002000
#define EFLAGS_IOPL_3 0x00003000
#define EFLAGS_NESTED_TASK 0x00004000

#define EFLAGS_RESUME 0x00010000
#define EFLAGS_VIRTUAL_8086 0x00020000
#define EFLAGS_ALIGNMENT_CHECK 0x00040000
#define EFLAGS_VIRTUAL_INTERRUPT 0x00080000
#define EFLAGS_VIRTUAL_INTERRUPT_PENDING 0x00100000
#define EFLAGS_CPUID_AVAILABLE 0x00200000

#endif
