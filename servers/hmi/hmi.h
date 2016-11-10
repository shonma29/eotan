#ifndef __HMI_H__
#define __HMI_H__
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
#include <device.h>

#define MYNAME "cons"

#define INTERRUPT_QUEUE_SIZE (1024)
#define REQUEST_QUEUE_SIZE (256)

#define MAX_WINDOW (16)

typedef struct {
	UW left;
	UW top;
	UW width;
	UW height;
	UW cursor_x;
	UW cursor_y;
	UW color;
	UW enabled;
} window_t;

typedef struct _interrupt_message_t {
	int type;
	int data;
} interrupt_message_t;

typedef struct _request_message_t {
	RDVNO rdvno;
	devmsg_t *message;
} request_message_t;

extern void hmi_handle(const int, const int);

#endif
