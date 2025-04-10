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
#include <set/sequence.h>

#define MAP_ALL_FREE (-1)


int sequence_initialize(sequence_t *s, size_t max, void *buf)
{
	if (!max
			|| (max > INT_MAX)
			|| !buf)
		return (-1);

	s->rest = max;
	s->clock_block = 0;
	s->num_of_blocks = SEQUENCE_BLOCK_SIZE(max);
	s->map = buf;
	s->mask = 0xffffffff;

	for (unsigned int i = 0; i < s->num_of_blocks; i++)
		s->map[i] = MAP_ALL_FREE;

	int remainder = max & SEQUENCE_BITS_MASK;
	if (remainder)
		s->map[s->num_of_blocks - 1] = (1 << remainder) - 1;

	return 0;
}
