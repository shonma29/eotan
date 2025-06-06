#ifndef _HMI_TERMINAL_H_
#define _HMI_TERMINAL_H_
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
#include <sys/types.h>
#include <console.h>

#define STR_CONS_INIT "\x1b[2J\x1b[1;1H"
#define LEN_CONS_INIT (10)

#define ESC_MAX_PARAMS (2)
#define ESC_MAX_COLUMNS (5)
#define ESC_BUF_SIZE (16)

typedef struct _esc_state {
	void (*func)(struct _esc_state *, const char);
	Screen *screen;
	int len;
	int num_of_params;
	int columns;
	int params[ESC_MAX_PARAMS];
	char buf[ESC_BUF_SIZE];
} esc_state_t;

extern void terminal_initialize(esc_state_t *, const Display *);
extern void terminal_write(esc_state_t *, char const *, const size_t);

#endif
