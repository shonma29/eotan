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
#ifndef __LIBRC_RANGECODER_H__
#define __LIBRC_RANGECODER_H__

#include <stdbool.h>
#include <stdio.h>

#define MAX_RANGE 0xffffffff
#define MIN_RANGE 0x01000000
#define SHIFT (32 - 8)

#define CHR_RANGE (256 + 1)
#define CHR_EOS (256)

#define ERR_OK (0)
#define ERR_FILE (1)
#define ERR_ARG (2)
#define ERR_MEMORY (3)
#define ERR_DATA (4)

typedef struct _RangeCoder {
	int (*get)(struct _RangeCoder *);
	int (*put)(unsigned char, struct _RangeCoder *);
	unsigned int range;
	unsigned int low;
	bool carry;
	unsigned int buf;
	unsigned int count;
} RangeCoder;

#include "bit.h"

extern void rc_initialize(RangeCoder *, int (*)(RangeCoder *),
		int (*)(unsigned char, RangeCoder *));
extern int rc_encode_normalize(RangeCoder *);
extern int rc_encode(Frequency *freq, RangeCoder *rc);
extern int rc_decode_normalize(RangeCoder *);
extern int rc_decode(Frequency *freq, RangeCoder *rc);

#endif
