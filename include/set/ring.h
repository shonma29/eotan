#ifndef _SET_RING_H_
#define _SET_RING_H_
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

#define RING_MAX_LEN 1024

#define RING_OK (0)
#define RING_EMPTY (-1)
#define RING_FULL (-2)
#define RING_TOO_LONG (-3)

typedef unsigned int ring_pos_t;
typedef unsigned int counter_t;
typedef char ring_chr_t;

typedef struct
{
	ring_pos_t read;
	ring_pos_t write;
	counter_t cycle;
	size_t left;
	size_t max;
	ring_chr_t buf[0];
} ring_t;

extern ring_t *ring_create(void *buf, size_t size);
extern int ring_peak_len(ring_t *r);
extern int ring_get(ring_t *r, ring_chr_t *buf);
extern int ring_put(ring_t *r, const ring_chr_t *buf, const size_t len);

#endif
