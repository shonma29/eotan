#ifndef _CORE_SETTING_H_
#define _CORE_SETTING_H_
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
#include <config.h>

#define BOOT_INFO_ADDR 0x80003000
#define MEMORY_MAP_ADDR 0x80180000

#define KERN_STACK_ADDR 0x80008000
#define KERN_STACK_SIZE (16 * 1024)

#define KERNEL_LOG_ADDR 0x80170000
#define KERNEL_LOG_SIZE (16 * 1024)

#define MAX_PAGES (1024 * 1024 / 2)

// MIN_MEMORY_SIZE should be a multiple of 4 MB.
#define NUM_OF_INITIAL_DIR (MIN_MEMORY_SIZE / PAGE_SIZE / PTE_PER_PAGE)

#define MIN_MANUAL_ID (1)
#define MAX_MANUAL_ID (49151)
#define MIN_AUTO_ID (49152)
#define MAX_AUTO_ID (65535)

#define KERNEL_DOMAIN_ID (-1)

#define MIN_PRIORITY (0)
#define MAX_PRIORITY (7)

typedef enum {
	pri_super = 0,
	pri_dispatcher = 1,
	pri_mutex = 2,
	pri_server_high = 3,
	pri_server_middle = 4,
	pri_server_low = 5,
	pri_user_foreground = 6,
	pri_user_background = 7
} system_priority_e;

#endif
