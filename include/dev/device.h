#ifndef _DEV_DEVICE_H_
#define _DEV_DEVICE_H_
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
#include <nerve/global.h>
#include <set/list.h>
#include <sys/types.h>

#define MAX_DEVICE (32)
#define DEV_BUF_SIZE (1024)

typedef enum {
	devmsg_find = 0
} devmsg_e;

typedef struct {
	devmsg_e type;
	int arg1;
	int arg2;
	int arg3;
} devmsg_t;

typedef struct {
	list_t bros;
	const char *name;
	void *unit;
} vdriver_unit_t;

typedef struct _vdriver_t {
	const char *class;
	list_t units;
	int (*detach)(void);
	int (*create)(const void *);
	int (*remove)(const char *);
	int (*open)(const char *);
	int (*close)(const int);
	int (*read)(char *, const int, const off_t, const size_t);
	int (*write)(char *, const int, const off_t, const size_t);
} vdriver_t;

typedef struct {
	char name[MAX_DEVICE_NAME + 1];
	const void *unit;
	const vdriver_t *driver;
} device_info_t;

#endif
