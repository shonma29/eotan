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
#include <stdlib.h>
#include <string.h>


int _wctomb(unsigned char *dest, const wchar_t ch)
{
	if (ch <= 0x7f) {
		*dest = (unsigned char)(ch & 0x7f);
		return 1;
		
	} else if (ch <= 0x7ff) {
		*dest++ = (unsigned char)(0xc0 | (ch >> 6));
		*dest = (unsigned char)(0x80 | (ch & 0x3f));
		return 2;

	} else if (ch <= 0xffff) {
		*dest++ = (unsigned char)(0xe0 | (ch >> 12));
		*dest++ = (unsigned char)(0x80 | ((ch >> 6) & 0x3f));
		*dest = (unsigned char)(0x80 | (ch & 0x3f));
		return 3;

	} else if (ch <= 0x10ffff) {
		*dest++ = (unsigned char)(0xf0 | (ch >> 18));
		*dest++ = (unsigned char)(0x80 | ((ch >> 12) & 0x3f));
		*dest++ = (unsigned char)(0x80 | ((ch >> 6) & 0x3f));
		*dest = (unsigned char)(0x80 | (ch & 0x3f));
		return 4;

	} else
		return -1;
}

/**
 * convert UTF-32 character to UTF-8 character.
 */
int wctomb(char *s, wchar_t wchar)
{
	return s? _wctomb((unsigned char*)s, wchar):0;
}
