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
#include <string.h>
#include <nerve/kcall.h>
#include "srv.h"

list_t servers;

static srv_unit_t *find(const char *);
//TODO return adequate error code
static bool check_param(const int, const off_t, const size_t);


int detach(void)
{
	return 0;
}

int create(const void *unit)
{
	srv_unit_t *u = (srv_unit_t *) unit;
	list_append(&servers, &(u->bros));
	return 0;
}

int remove(const char *name)
{
	srv_unit_t *u = find(name);
	if (u) {
		list_remove(&(u->bros));
		return 0;
	} else
		return (-1);
}

int open(const char *name)
{
	return ((int) find(name));
}

int close(const int channel)
{
	//TODO check channel
	return (channel ? 0 : (-1));
}

int read(char *outbuf, const int channel, const off_t offset, const size_t size)
{
	if (!check_param(channel, offset, size))
		return (-1);

	srv_unit_t *u = (srv_unit_t *) channel;
	for (;;) {
		if (lfq_dequeue(u->queue, outbuf) == QUEUE_OK)
			return size;

		kcall->thread_sleep();
	}
}

int write(char *inbuf, const int channel, const off_t offset, const size_t size)
{
	if (!check_param(channel, offset, size))
		return (-1);

	srv_unit_t *u = (srv_unit_t *) channel;
	if (lfq_enqueue(u->queue, inbuf) == QUEUE_OK) {
		kcall->thread_wakeup(u->thread_id);
		return size;
	}

	return (-1);
}

static srv_unit_t *find(const char *name)
{
	for (list_t *unit = list_head(&servers);
			!list_is_edge(&servers, unit); (unit = unit->next)) {
		srv_unit_t *u = (srv_unit_t *) unit;
		if (!strcmp(u->name, name))
			return u;
	}

	return NULL;
}

static bool check_param(const int channel, const off_t offset,
		const size_t size)
{
	//TODO check channel
	if (!channel)
		return false;

	if (offset)
		return false;

	if (size != 1)
		return false;

	return true;
}
