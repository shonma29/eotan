#ifndef _SET_SEQUENCE_H_
#define _SET_SEQUENCE_H_
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
#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <mpu/memory.h>

#define INT_BIT (CHAR_BIT * sizeof(int))

#define SEQUENCE_BITS_MASK (INT_BIT - 1)

#define SEQUENCE_BLOCK_SIZE(n) (((n) + (INT_BIT - 1)) >> MPU_LOG_INT)
#define SEQUENCE_MAP_SIZE(n) (SEQUENCE_BLOCK_SIZE(n) * sizeof(int))

typedef struct {
	size_t rest;
	int clock_block;
	size_t num_of_blocks;
	uint32_t *map;
	uint32_t mask;
} sequence_t;

extern int sequence_initialize(sequence_t *, size_t, void *);
extern int sequence_get(sequence_t *);
extern void sequence_release(sequence_t *, const int);

#endif
