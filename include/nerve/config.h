#ifndef _NERVE_CONFIG_H_
#define _NERVE_CONFIG_H_
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
#include <mpu/memory.h>

#define BOOT_INFO_ADDR 0x80001000
#define SYSTEM_INFO_ADDR 0x80001e00
#define PERIPHERAL_INFO_ADDR 0x80001ff0
#define MEMORY_MAP_ADDR 0x80180000

#define CORE_STACK_ADDR 0x80008000
#define CORE_STACK_SIZE (4 * 1024)

#define KTHREAD_DIR_ADDR 0x00002000
#define KTHREAD_STACK_SIZE (4 * 1024)

#define KERNEL_LOG_ADDR 0x80170000
#define KERNEL_LOG_SIZE (16 * 1024)

#define KCALL_ADDR 0x80001f00
#define ICALL_ADDR 0x80001fc0

#define MACHINE_LOCAL_ADDR 0x7e000000
#define PROCESS_LOCAL_ADDR 0x7e001000

#define MAX_PAGES (1024 * 1024 / 2)

// MIN_MEMORY_MB should be a multiple of 4 MB.
#define MIN_MEMORY_MB (8)
#define NUM_OF_INITIAL_DIR \
		((MIN_MEMORY_MB * 1024 * 1024) / PAGE_SIZE / PTE_PER_PAGE)

#define MIN_MANUAL_ID (1)
#define MAX_MANUAL_ID (1024)
#define MIN_AUTO_ID (1025)
#define MAX_AUTO_ID (32767)

typedef enum {
	pri_dispatcher = 0,
	pri_mutex = 1,
	pri_server_high = 2,
	pri_server_middle = 3,
	pri_server_low = 4,
	pri_user_foreground = 5,
	pri_user_background = 6,
	pri_idle = 7
} system_priority_e;

#define MIN_PRIORITY (pri_dispatcher)
#define MAX_PRIORITY (pri_idle)

/* ticks per second */
#define TIME_TICKS (1000)

/* available ticks for user threads */
#define TIME_QUANTUM (10)

#define MAX_DEVICE_NAME (31)

#define MAX_INTERRUPT_SERVICE (64)
#define MAX_PERIPHERAL (32)

#if USE_VESA
#define USE_MONITOR 0
#endif

#endif
