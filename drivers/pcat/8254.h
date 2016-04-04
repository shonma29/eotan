#ifndef __ARCH_8254_H__
#define __ARCH_8254_H__ 1
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

enum PitPort {
	pit_std_counter0 = 0x0040,
	pit_std_counter1 = 0x0041,
	pit_std_counter2 = 0x0042,
	pit_std_control = 0x0043,
	pit_ex_counter0 =  0x0044,
	pit_ex_counter1 = 0x0045,
	pit_ex_counter2 = 0x0046,
	pit_ex_control = 0x0047
};

#define PIT_CLOCK_MUL3 (3579545)
#define PIT_MIN_FREQ (20)
#define PIT_MAX_FREQ (1000000)

#define PIT_OUTPUT 0x80
#define PIT_NULL_COUNT 0x40
#define PIT_LAST_MODE 0x3f

#define PIT_READ_BACK 0xc0
#define PIT_SELECT_CNT2 0x80
#define PIT_SELECT_CNT1 0x40
#define PIT_SELECT_CNT0 0x00

#define PIT_READBACK_COUNT 0x20
#define PIT_READBACK_STATUS 0x10
#define PIT_READBACK_CNT2 0x08
#define PIT_READBACK_CNT1 0x04
#define PIT_READBACK_CNT0 0x02

#define PIT_ACCESS_WORD 0x30
#define PIT_ACCESS_HIGH 0x20
#define PIT_ACCESS_LOW 0x10
#define PIT_ACCESS_LATCH 0x00

#define PIT_MODE_TERMINAL 0x00
#define PIT_MODE_ONESHOT 0x02
#define PIT_MODE_RATE 0x04
#define PIT_MODE_SQUARE 0x06
#define PIT_MODE_SOFT_TRIGGER 0x08
#define PIT_MODE_HARD_TRIGGER 0x0a

#define PIT_COUNT_BCD 0x01
#define PIT_COUNT_BINARY 0x00

#endif
