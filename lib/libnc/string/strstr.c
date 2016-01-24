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
#include <limits.h>
#include <stddef.h>
#include <string.h>


char *strstr(const char *haystack, const char *needle)
{
	size_t skip[UCHAR_MAX + 1];
	size_t keyLen = strlen(needle);
	int end = strlen(haystack) - keyLen;
	int i;

	if (keyLen == 0)
		return (char*)haystack;

	for (i = 0; i < sizeof(skip) / sizeof(skip[0]); i++)
		skip[i] = keyLen + 1;

	for (i = 0; i < keyLen; i++)
		skip[((unsigned char*)needle)[i]] = keyLen - i;

	for (i = 0; i <= end;
			i += skip[((unsigned char*)haystack)[i + keyLen]]) {
		int offset;

		for (offset = 0; offset < keyLen; offset++)
			if (haystack[i + offset] != needle[offset])
				break;

		if (offset == keyLen)
			return (char*)&(haystack[i]);
	}

	return NULL;
}
