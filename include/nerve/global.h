#ifndef _NERVE_GLOBAL_H_
#define _NERVE_GLOBAL_H_
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
#include <time.h>
#include <stddef.h>
#include <nerve/config.h>
#include <nerve/memory_map.h>
#include <set/lf_queue.h>
#include <hmi/draw.h>

typedef struct {
	char device[MAX_DEVICE_NAME + 1];
	int fstype;
	size_t block_size;
} mount_node_t;

typedef struct {
	void *address;
	size_t size;
} memory_range_t;

typedef struct {
	volatile struct timespec system_time;
	union {
		volatile uint32_t dispatch_skippable;
		struct {
			volatile uint16_t interrupt_nest;
			volatile uint8_t no_request;
			volatile uint8_t serializing;
		} state;
	} sync;
	volatile bool delay_thread_start;
	volatile lfq_t kqueue;
	MemoryMap memory_map;
	mount_node_t root;
	memory_range_t initrd;
	Display display;
} system_info_t;

#define sysinfo ((system_info_t *) SYSTEM_INFO_ADDR)

#endif
