#ifndef _HMI_WINDOW_H_
#define _HMI_WINDOW_H_
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
#include <hmi/draw.h>

#define WINDOW_ATTR_HAS_BORDER (0x0001)
#define WINDOW_ATTR_HAS_TITLE (0x0002)
#define WINDOW_ATTR_SCROLLABLE_X (0x0004)
#define WINDOW_ATTR_SCROLLABLE_Y (0x0008)
#define WINDOW_ATTR_MOVABLE (0x0010)
#define WINDOW_ATTR_RESIZABLE (0x0020)
#define WINDOW_ATTR_ICONIFIABLE (0x0040)
#define WINDOW_ATTR_MAXIMIZABLE (0x0080)
#define WINDOW_ATTR_IS_DIALOG (0x0100)

typedef struct {
	uint32_t attr;
	Frame outer;
	Frame inner;
} window_t;

#endif
