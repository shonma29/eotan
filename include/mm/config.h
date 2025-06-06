#ifndef _MM_CONFIG_H_
#define _MM_CONFIG_H_
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
#include <sys/syslimits.h>

#define PROCESS_MAX (1024)
#define THREAD_MAX (1024)
#define THREADS_PER_PROCESS (32)
#define FILE_MAX (1024)
#define FILES_PER_PROCESS (1024)
#define SESSION_MAX (1024)
#define FILES_PER_SESSION (1024)
#define SEGMENT_MAX (8192)
#define NAMESPACE_MAX (1024)
#define SEMAPHORE_MAX (1024)

#define REQUEST_MAX (16)

#define USER_STACK_INITIAL_SIZE (ARG_MAX)
#define USER_STACK_MAX_SIZE (1 * 1024 * 1024)
#define USER_STACK_END_ADDR 0x80000000
#define USER_STACK_ADDR_MASK 0x7ff00000

#define USER_HEAP_MAX_ADDR (1 * 1024 * 1024 * 1024)

#define FIBER_POOL_MAX (2)

#endif
