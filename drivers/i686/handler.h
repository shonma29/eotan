#ifndef _MPU_HANDLER_H__
#define _MPU_HANDLER_H__
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

/* handler.s */
extern void handle0(void);
extern void handle1(void);
extern void handle2(void);
extern void handle3(void);
extern void handle4(void);
extern void handle5(void);
extern void handle6(void);
extern void handle7(void);
extern void handle8(void);
extern void handle9(void);
extern void handle10(void);
extern void handle11(void);
extern void handle12(void);
extern void handle13(void);
extern void handle14(void);
extern void handle15(void);
extern void handle16(void);
extern void handle17(void);
extern void handle18(void);
extern void handle19(void);
extern void handle20(void);
extern void handle21(void);
extern void handle22(void);
extern void handle23(void);
extern void handle24(void);
extern void handle25(void);
extern void handle26(void);
extern void handle27(void);
extern void handle28(void);
extern void handle29(void);
extern void handle30(void);
extern void handle31(void);

static void (*handlers[])(void) = {
	handle0,
	handle1,
	handle2,
	handle3,
	handle4,
	handle5,
	handle6,
	handle7,
	handle8,
	handle9,
	handle10,
	handle11,
	handle12,
	handle13,
	handle14,
	handle15,
	handle16,
	handle17,
	handle18,
	handle19,
	handle20,
	handle21,
	handle22,
	handle23,
	handle24,
	handle25,
	handle26,
	handle27,
	handle28,
	handle29,
	handle30,
	handle31
};

#endif
