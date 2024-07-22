#ifndef _CONSOLE_CONSOLE_H_
#define _CONSOLE_CONSOLE_H_
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
#include <copier.h>
#include <sys/types.h>
#include <set/tree.h>
#include "../../lib/libserv/libserv.h"

#define MYNAME "console"

struct file {
	node_t node;
	uint_fast32_t f_flag;
	size_t size;
	void *driver;
};

typedef struct _driver_t {
	bool endless;
	int (*read)(struct file *, copier_t *, const off_t, const size_t,
			size_t *);
	int (*write)(struct file *, copier_t *, const off_t, const size_t,
			size_t *);
} driver_t;

extern int device_initialize(void);
extern driver_t *device_lookup(const char *);

#endif
