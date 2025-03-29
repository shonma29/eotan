#ifndef _HMI_HMI_H_
#define _HMI_HMI_H_
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
#include <core.h>
#include <services.h>
#include <set/lf_queue.h>
#include <hmi/window.h>

#define MYNAME "hmi"
#define MYPORT PORT_WINDOW

#define WINDOW_MAX (2)

#define INTERRUPT_QUEUE_SIZE (1024)
#define REQUEST_QUEUE_SIZE (64)

typedef enum {
	CONS = 1,
	CONSCTL = 2,
	DRAW = 3,
	EVENT = 4
} channel_e;

typedef struct _hmi_interrupt_t {
	int type;
	int data;
} hmi_interrupt_t;

typedef struct _driver_t {
	channel_e channel;
} driver_t;

extern slab_t request_slab;
extern Display *display;

// window.c
extern int window_initialize(void);
extern int window_create(window_t **, const int, const int,
		const int, const int, const int);
extern void window_set_title(window_t *, const char *);
extern window_t *window_find(const int);
extern int window_destroy(window_t *);
extern int window_focus(const int);

// draw.c
extern ER_UINT draw_write(const window_t *, const UW, const char *, const int);

// event.c
extern volatile lfq_t interrupt_queue;

extern void event_write(const int);
extern ER_UINT consctl_write(const UW, const char *);
extern int event_initialize(void);

// device.c
extern driver_t *device_lookup(const char *);

#endif
