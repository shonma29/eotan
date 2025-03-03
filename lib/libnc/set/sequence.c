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
#include <mpu/bits.h>
#include <set/sequence.h>

#define MAP_ALL_FREE (-1)

#define BITS_MASK (INT_BIT - 1)

static int _find(sequence_t *s, const int);


int sequence_initialize(sequence_t *s, size_t max, void *buf)
{
	if (!max
			|| (max > INT_MAX)
			|| (max & BITS_MASK))
		return (-1);

	s->rest = max;
	s->clock_block = 0;
	s->num_of_blocks = SEQUENCE_BLOCK_SIZE(max);
	s->map = buf;

	for (unsigned int i = 0; i < s->num_of_blocks; i++)
		s->map[i] = MAP_ALL_FREE;

	return 0;
}

static int _find(sequence_t *s, const int block)
{
	uint32_t bits = s->map[block];
	if (bits) {
		int offset = count_ntz(bits);
		s->rest--;
		s->clock_block = block;
		s->map[block] &= ~((uint32_t) 1 << offset);
		return ((block << MPU_LOG_INT) | offset);
	} else
		return (-1);
}

int sequence_get(sequence_t *s)
{
	if (s->rest > 0) {
		for (unsigned int i = (unsigned int) (s->clock_block);
				i < s->num_of_blocks; i++) {
			int n = _find(s, i);
			if (n >= 0)
				return n;
		}

		for (unsigned int i = 0;
				i < (unsigned int) (s->clock_block); i++) {
			int n = _find(s, i);
			if (n >= 0)
				return n;
		}
	}

	return (-1);
}

void sequence_release(sequence_t *s, const int n)
{
	unsigned int block = (unsigned int) n >> MPU_LOG_INT;
	if (block >= s->num_of_blocks)
		return;

	uint32_t bits = (uint32_t) 1 << (n & BITS_MASK);
	if (s->map[block] & bits)
		return;

	s->map[block] |= bits;

	if (!(s->rest)) {
		s->clock_block = block;
	}

	s->rest++;
}
