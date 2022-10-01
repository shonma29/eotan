#ifndef _HMI_POINTER_H_
#define _HMI_POINTER_H_
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
#include <stdint.h>
#include <hmi/draw.h>

#define POINTER_WIDTH (24)
#define POINTER_HEIGHT (24)
#define POINTER_NUM_PER_LINE (((POINTER_WIDTH * 2) + (32 - 1)) / 32)

typedef struct {
	uint32_t point_x;
	uint32_t point_y;
	// 0: transparent, 1: xor, 2: bg, 3: fg
	uint32_t buf[POINTER_NUM_PER_LINE * POINTER_HEIGHT];
} pointer_pattern_t;

enum pointer_style {
	pointer_select,
	pointer_modify,
	pointer_movable,
	pointer_vmovable,
	pointer_hmovable,
	pointer_move,
	pointer_vmove,
	pointer_hmove,
	pointer_resizable,
	pointer_vresizable,
	pointer_hresizable,
	pointer_resize,
	pointer_vresize,
	pointer_hresize,
	pointer_busy,
	pointer_menu
};

#define NUM_OF_POINTER_STYLE (16)

extern void pointer_put(const Display *, const int, const int,
		const pointer_pattern_t *);
extern void pointer_restore(const Display *);

#endif
