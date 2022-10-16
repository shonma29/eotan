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

static inline int min(const int x, const int y)
{
	return ((x < y) ? x : y);
}

static inline int max(const int x, const int y)
{
	return ((x > y) ? x : y);
}


bool rect_intersect(Rectangle *dest, Rectangle *src1, Rectangle *src2)
{
	do {
		dest->min.x = max(src1->min.x, src2->min.x);
		dest->max.x = min(src1->max.x, src2->max.x);
		if (dest->min.x >= dest->max.x)
			break;

		dest->min.y = max(src1->min.y, src2->min.y);
		dest->max.y = min(src1->max.y, src2->max.y);
		if (dest->min.y >= dest->max.y)
			break;

		return true;
	} while (false);

	dest->min.x = 0;
	dest->min.y = 0;
	dest->max.x = 0;
	dest->max.y = 0;
	return false;
}
