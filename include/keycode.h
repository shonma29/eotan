#ifndef __KEYCODE_H__
#define __KEYCODE_H__ 1
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
#include <stdbool.h>

#define LSHIFT 0x01
#define RSHIFT 0x02
#define MASK_SHIFT (LSHIFT | RSHIFT)
#define LCTRL 0x04
#define RCTRL 0x08
#define MASK_CTRL (LCTRL | RCTRL)
#define LALT 0x10
#define RALT 0x20
#define MASK_ALT (LALT | RALT)
#define LWIN 0x40
#define RWIN 0x80
#define MASK_WIN (LWIN | RWIN)
#define CAPS 0x100
#define BREAK 0x200

static inline bool is_shift(const unsigned short m)
{
	bool shift = (m & MASK_SHIFT) ? true : false;
	bool caps = (m & CAPS) ? true : false;

	return (shift ^ caps);
}

#endif
